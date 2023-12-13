// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "TalesDungeoneer/lib/datastructures/GlobalData.h"
#include "Logging/StructuredLog.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "TalesDungeoneer/Saves/SavedCharacters.h"

#include "Net/UnrealNetwork.h"
#include "TalesDungeoneer/Gamemode/BaseFiles/TalesGameStateBase.h"
#include "TalesDungeoneer/Weapons/WeaponSystem.h"

// Sets default values
ACharacterBase::ACharacterBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	USkeletalMeshComponent* SkeletalMesh = GetMesh();
	SkeletalMesh->SetIsReplicated(true);

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	// Every single character gets an inventory, vitality and weapon system
	// regardless of whether or not they are a player.
	InventoryComponent = CreateDefaultSubobject
			<UInventoryComponent>(TEXT("InventoryComponent"));
	
	WeaponComponent		= CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
	MeshMergeComponent	= CreateDefaultSubobject<UMeshMergeComponent>(TEXT("MeshMergeComponent"));

	// Allow weapon overlap collisions
	//GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Block);
}

/** Called to start an attack.
 *  Once started, the attack will continue until finished.
 * @param WeaponSlot The weapon slot to attack with (defaults to Primary)
 */
void ACharacterBase::DoAttack(EWeaponSlots WeaponSlot)
{
	WeaponComponent->PerformAttack(WeaponSlot);
}

void ACharacterBase::SetCharacterLevel(int NewLevel)
{
	const int OldLevel = _CharacterLevel;
	if (OldLevel != NewLevel)
	{
		if (NewLevel > 0)
		{
			_CharacterLevel = NewLevel;
			_ExperiencePoints = 0.f;
		}
	}
}

void ACharacterBase::SetCharacterClass(ECharacterClass NewClass)
{
	if (GetNetMode() < NM_Client)
	{
		if (_CharacterClass != NewClass)
		{
			_CharacterClass = NewClass;
			ReinitializeSubsystems();
		}
	}
}

void ACharacterBase::SetCharacterRace(ECharacterRace NewRace)
{
	if (GetNetMode() < NM_Client)
	{
		if (_CharacterRace != NewRace)
		{
			_CharacterRace = NewRace;
			ReinitializeSubsystems();
		}
	}
}

bool ACharacterBase::SaveCharacterData()
{
	return true;
}

bool ACharacterBase::LoadCharacterData(const FString SaveSlotName, const int32 UserIndex)
{
	return true;
}


// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>
					(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Server Init (always happens before the client)
	if (GetNetMode() < NM_Client)
	{		
		// Register the Weapon Component to listen for changes to Equipment
		if (IsValid(WeaponComponent) && IsValid(InventoryComponent))
		{
			// Setup listeners first, then initialize
			if (!InventoryComponent->OnEquipmentUpdated.IsAlreadyBound(this, &ACharacterBase::UpdateWeapon))
				InventoryComponent->OnEquipmentUpdated.AddDynamic(this, &ACharacterBase::UpdateWeapon);

			UpdateWeapon(EEquipmentSlotType::PRIMARY);
			UpdateWeapon(EEquipmentSlotType::SECONDARY);
		}
		ReinitializeSubsystems();
	}
	
}


void ACharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetCharacterName(CharacterName);

	InventoryComponent->NumberOfInvSlots = 18;
	InventoryComponent->EligibleEquipmentSlots = {
		EEquipmentSlotType::PRIMARY,		EEquipmentSlotType::SECONDARY,
		EEquipmentSlotType::HELMET,			EEquipmentSlotType::NECK,
		EEquipmentSlotType::EARRINGLEFT,	EEquipmentSlotType::EARRINGRIGHT,
		EEquipmentSlotType::FACE, 			EEquipmentSlotType::SHOULDERS,
		EEquipmentSlotType::BACK, 			EEquipmentSlotType::SLEEVES,
		EEquipmentSlotType::WRISTLEFT,		EEquipmentSlotType::WRISTRIGHT,
		EEquipmentSlotType::HANDS,			EEquipmentSlotType::RINGLEFT,
		EEquipmentSlotType::RINGRIGHT,		EEquipmentSlotType::TORSO,
		EEquipmentSlotType::WAIST,			EEquipmentSlotType::LEGS,
		EEquipmentSlotType::FEET,			EEquipmentSlotType::COSMETIC
	};
	
}

void ACharacterBase::CharacterRestoredFromSave(const FString SaveSlotName)
{
	bCharacterSaveRestored = true;
	OnCharacterRestored.Broadcast(SaveSlotName);
	if (HasAuthority())
		Client_CharacterRestored(SaveSlotName);
}

void ACharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	const bool doServerSave =	HasAuthority() &&   bSavesOnServer;
	const bool doClientSave = ! HasAuthority() && ! bSavesOnServer;
	if ( doServerSave || doClientSave )
	{
		if (IsValid(GetWorld()))
		{
			ATalesGameStateBase* TalesGameState = Cast<ATalesGameStateBase>(GetWorld()->GetGameState());
			if (IsValid(TalesGameState))
			{
				FString SaveResponse;
				TalesGameState->SaveCurrentCharacter(SaveResponse, false);
				UE_LOG(LogTemp, Display, TEXT("Character Save: %s"), *SaveResponse);
			}
		}
	}
	Super::EndPlay(EndPlayReason);
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


//////////////////////////////////////////////////////////////////////////
// Input

void ACharacterBase::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpInputAction,
			ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpInputAction,
			ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveInputAction,
		ETriggerEvent::Triggered, this, &ACharacterBase::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookInputAction,
		ETriggerEvent::Triggered, this, &ACharacterBase::Look);

		//Actions
		EnhancedInputComponent->BindAction(PrimaryInputAction,
			ETriggerEvent::Triggered, this, &ACharacterBase::PrimaryAction);
		EnhancedInputComponent->BindAction(SecondaryInputAction,
			ETriggerEvent::Triggered, this, &ACharacterBase::SecondaryAction);

	}

}

void ACharacterBase::HotkeyTriggered(UInputAction* HotkeyAction)
{

}

void ACharacterBase::SetCharacterName(FString ProposedName)
{
	// TODO - Add checks for symbols, special characters, etc
	CharacterName = ProposedName;
}

void ACharacterBase::Client_CharacterRestored_Implementation(const FString& SaveSlotName)
{
	bCharacterSaveRestored = true;
	OnCharacterRestored.Broadcast(SaveSlotName);
}

void ACharacterBase::OnRep_CharacterName_Implementation()
{
	OnCharacterNameChanged.Broadcast();
}

void ACharacterBase::OnRep_CharacterLevel_Implementation(int OldLevel)
{
	const int NewLevel = GetCharacterLevel();
	if (OldLevel < NewLevel)
	{
		OnCharacterLevelUp.Broadcast(NewLevel);
	}
	else
	{
		OnCharacterLevelChanged.Broadcast();
	}
}

void ACharacterBase::OnRep_ExperienceChanged_Implementation(float OldExperience)
{
	OnExperienceChanged.Broadcast();
}

void ACharacterBase::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ACharacterBase::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}


//////////////////////////////////////////////////////////////////////////
// Replication
void ACharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ACharacterBase, _CharacterRisk);
	DOREPLIFETIME(ACharacterBase, CharacterName);
	DOREPLIFETIME(ACharacterBase, _IsMale);
	
	DOREPLIFETIME(ACharacterBase, SkinColor);
	DOREPLIFETIME(ACharacterBase, PronounObjective);
	DOREPLIFETIME(ACharacterBase, PronounPossessive);
	DOREPLIFETIME(ACharacterBase, PronounSubject);
	
	DOREPLIFETIME(ACharacterBase, _CharacterLevel);
	DOREPLIFETIME_CONDITION(ACharacterBase, _ExperiencePoints, COND_OwnerOnly);
}
