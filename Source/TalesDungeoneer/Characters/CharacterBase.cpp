// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"


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

	GetMesh()->SetIsReplicated(true);

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
	
	VitalityComponent = CreateDefaultSubobject
			<UVitalityComponent>(TEXT("VitalityComponent"));
	
	WeaponComponent = CreateDefaultSubobject
			<UWeaponComponent>(TEXT("WeaponComponent"));
	
	AbilityComponent = CreateDefaultSubobject
		<UAbilityComponent>(TEXT("AbilityComponent"));
	
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->bOwnerNoSee = true;
	
	
}

void ACharacterBase::ToggleWeapon(EWeaponSlots WeaponSlot,
		bool ForceDraw, bool ForceStow)
{
	if (ForceDraw)
	{
		WeaponComponent->SetToggleWeapon(WeaponSlot, true);
	}
	else if (ForceStow)
	{
		WeaponComponent->SetToggleWeapon(WeaponSlot, false);
	}
	else
	{
		WeaponComponent->SetToggleWeapon(WeaponSlot,
			WeaponComponent->GetIsWeaponReady(WeaponSlot));
	}
}

void ACharacterBase::PerformAttack(EWeaponSlots WeaponSlot)
{
	WeaponComponent->PerformAttack(WeaponSlot);
}

void ACharacterBase::StartBlocking()
{
	WeaponComponent->SetIsBlocking(true);
}

void ACharacterBase::StopBlocking()
{
	WeaponComponent->SetIsBlocking(false);
}


void ACharacterBase::SetCharacterTeam(ECharacterTeam NewTeam)
{
	if (HasAuthority())
	{
		_CharacterTeam = NewTeam;
	}
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

		//Attacking
		EnhancedInputComponent->BindAction(PrimaryAttackInputAction,
			ETriggerEvent::Triggered, this, &ACharacterBase::PrimaryAttack);
		EnhancedInputComponent->BindAction(SecondaryAttackInputAction,
		ETriggerEvent::Triggered, this, &ACharacterBase::SecondaryAttack);

		//Actions
		EnhancedInputComponent->BindAction(PrimaryInputAction,
			ETriggerEvent::Triggered, this, &ACharacterBase::PrimaryAction);
		EnhancedInputComponent->BindAction(SecondaryInputAction,
			ETriggerEvent::Triggered, this, &ACharacterBase::SecondaryAction);

	}

}

void ACharacterBase::HotkeyTriggered(UInputAction* HotkeyAction)
{
	if (IsValid(AbilityComponent))
	{
		AbilityComponent->AbilityAction(HotkeyAction);
	}
}

void ACharacterBase::PrimaryAttack_Implementation()
{
	WeaponComponent->PerformAttack(EWeaponSlots::PRIMARY);
	OnPrimaryAttack.Broadcast();
}

void ACharacterBase::PrimaryAttackVirtual()
{
	PrimaryAttack();
}

void ACharacterBase::SecondaryAttack_Implementation()
{
	WeaponComponent->PerformAttack(EWeaponSlots::SECONDARY);
	OnSecondaryAttack.Broadcast();
}

void ACharacterBase::SecondaryAttackVirtual()
{
	SecondaryAttack();
}

void ACharacterBase::PrimaryActionVirtual()
{
	PrimaryAction();
}

void ACharacterBase::SecondaryActionVirtual()
{
	SecondaryAction();
}

void ACharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (HasAuthority())
	{
		if (IsValid(InventoryComponent))
		{
			// Don gear that is equipped at time of initialization
			UpdateWeapon(EWeaponSlots::PRIMARY);
			UpdateWeapon(EWeaponSlots::SECONDARY);
		}
	}
}

void ACharacterBase::UpdateWeapon(EWeaponSlots WeaponSlot)
{
	EEquipmentSlotType equipmentEnum = EEquipmentSlotType::PRIMARY;
	switch(WeaponSlot)
	{
	case EWeaponSlots::SECONDARY:
		equipmentEnum = EEquipmentSlotType::PRIMARY;
		break;
	default:
		break;
	}
	const int equipmentSlot   = InventoryComponent->getEquipmentSlotNumber(equipmentEnum);
	const FName equipmentItem = InventoryComponent->getItemNameInSlot(equipmentSlot, true);
	if (UItemSystem::getItemNameIsValid(equipmentItem))
	{
		WeaponComponent->SetWeapon(equipmentItem, WeaponSlot);
	}
	else
	{
		WeaponComponent->UnsetWeapon(WeaponSlot);	
	}
}

void ACharacterBase::PrimaryAction_Implementation()
{
	OnPrimaryAction.Broadcast();
}

void ACharacterBase::SecondaryAction_Implementation()
{
	OnSecondaryAction.Broadcast();
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
	DOREPLIFETIME(ACharacterBase, _CharacterTeam);
	DOREPLIFETIME(ACharacterBase, _CharacterLevel);
	DOREPLIFETIME(ACharacterBase, _CharacterClass);
	DOREPLIFETIME(ACharacterBase, _CharacterRisk);
}
