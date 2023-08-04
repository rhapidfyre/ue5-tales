// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "TalesDungeoneer/lib/datastructures/GlobalData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "Net/UnrealNetwork.h"
#include "TalesDungeoneer/Entities/SimpleActors/FloatingTextBase.h"


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
	
	MeshMergeComponent = CreateDefaultSubobject
		<UMeshMergeComponent>(TEXT("MeshMergeComponent"));
	
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

void ACharacterBase::SetCharacterLevel(int NewLevel)
{
	const int OldLevel = _CharacterLevel;
	if (OldLevel != _CharacterLevel)
	{
		_CharacterLevel = NewLevel;
		_ExperiencePoints = 0.f;
		
		if (_CharacterLevel > OldLevel)
		{
			OnCharacterLevelUp.Broadcast(_CharacterLevel);
		}
		
	}
}

void ACharacterBase::SetCharacterClass(ECharacterClass NewClass)
{
	_CharacterClass = NewClass;
}

void ACharacterBase::SetCharacterRace(ECharacterRace NewRace)
{
	_CharacterRace = NewRace;
}


void ACharacterBase::SetExperiencePoints(float NewValue)
{
	_ExperiencePoints = NewValue;
}


void ACharacterBase::AddExperiencePoints(float AddValue)
{
	const float NewValue = _ExperiencePoints + FMath::Abs(AddValue);
	const float NextLevel = GetExperienceNeeded();
	
	FTransform SpawnTransform(GetActorLocation()
		+ FVector(
			FMath::RandRange(32.f, 196.0f),
			FMath::RandRange(32.f, 196.0f),
			FMath::RandRange(32.f, 196.0f))
	);
	
	AFloatingTextBase* FloatText = GetWorld()->SpawnActor<AFloatingTextBase>(
		DamageTextActor, SpawnTransform);

	if (IsValid(FloatText))
	{
		FloatText->TextShown = "+ " + FString::SanitizeFloat(AddValue) + " XP";
		FloatText->TextColor = FLinearColor::Blue;
		FloatText->SecondsToShow = 3.f;
		FloatText->UpdateFloatingText();
	}
	
	if (NewValue >= NextLevel)
	{
		if (_CharacterLevel < UGlobalData::GetGameMaxCharacterLevel())
		{
			_CharacterLevel += 1;
			_ExperiencePoints = 0.f;
			AbilityComponent->AddUnlockPoints( UnlockPointsOnLevelUp );
			OnCharacterLevelUp.Broadcast(_CharacterLevel);
			FTransform LevelUpSpawnTransform(GetActorLocation()
				+ FVector(
					FMath::RandRange(32.f, 196.0f),
					FMath::RandRange(32.f, 196.0f),
					FMath::RandRange(32.f, 196.0f))
			);
	
			AFloatingTextBase* LevelUpText = GetWorld()->SpawnActor<AFloatingTextBase>(
				DamageTextActor, LevelUpSpawnTransform);

			if (IsValid(FloatText))
			{
				LevelUpText->TextShown = "LEVEL UP";
				LevelUpText->TextColor = FLinearColor::Yellow;
				LevelUpText->SecondsToShow = 3.f;
				LevelUpText->UpdateFloatingText();
			}
			return;
		}
	}
	_ExperiencePoints = NewValue > NextLevel ? NextLevel : NewValue;
}


void ACharacterBase::RemoveExperiencePoints(float AddValue)
{
	const float newValue = _ExperiencePoints -= FMath::Abs(AddValue);
	_ExperiencePoints = newValue > 0.f ? newValue : 0.f;
}


void ACharacterBase::AwardExperiencePoints(int AwardLevel, float BasePoints)
{
	const float ExperienceReward = BasePoints > 0.f ? BasePoints : 1.f;

	// EXP Penalties	https://www.desmos.com/calculator
	// Only players of equal level to the encounter should get the same experience reward.
	// Players who are level should get less, but at a lower rate of decay.
	// Players who are higher level should take a stronger penalty.
	// This discourages power-leveling by lessening the reward and punishing grinding low level NPCs
	const double UnderScalingFactor = 0.00225; // The exp penalty for being under level
	const double OverScalingFactor  = 0.001; // The exp penalty for being over level
	
	const int ConvergenceLevel = 30; // What level the experience dies
	
	int EffectiveLevel   = (AwardLevel > 0) ? AwardLevel : 1;
	int LevelDifference  = (_CharacterLevel - EffectiveLevel);

	// Award no experience
	if (LevelDifference >= ConvergenceLevel)
		return;
	
	const double ScalingFactor = (LevelDifference > 0) ? OverScalingFactor : UnderScalingFactor;
	if (LevelDifference == 0)
		AddExperiencePoints(BasePoints);
	else
	{
		const float XpAdjustment = 1/(1+pow(EULERS_NUMBER, 0.3 * (abs(LevelDifference) - 8)));
		AddExperiencePoints(BasePoints * XpAdjustment);
	}
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

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (IsValid(LocalPlayer))
	{
		if (!VitalityComponent->OnDamageTaken.IsAlreadyBound(this, &ACharacterBase::SpawnDamageText))
			VitalityComponent->OnDamageTaken.AddDynamic(this, &ACharacterBase::SpawnDamageText);
	}

	// When an ability is started, check if Combat State should change
	if (!AbilityComponent->OnAbilityCastStarted.IsAlreadyBound(this, &ACharacterBase::CheckAbilityStart))
		AbilityComponent->OnAbilityCastStarted.AddDynamic(this, &ACharacterBase::CheckAbilityStart);

	// When an ability is finished, check if Combat State should change
	if (!AbilityComponent->OnAbilityCastComplete.IsAlreadyBound(this, &ACharacterBase::CheckAbilitySuccess))
		 AbilityComponent->OnAbilityCastComplete.AddDynamic(this, &ACharacterBase::CheckAbilitySuccess);
}


void ACharacterBase::OnConstruction(const FTransform& Transform)
{
	
	// Setup character with default values
	
}

void ACharacterBase::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
	
	// If the mesh merge component isn't ready, run first time setup
	//if (!MeshMergeComponent->GetIsMeshMergeSystemReady())
	//{
		//MeshMergeComponent->InitializeMeshMerge();
	//};
	
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

void ACharacterBase::SpawnDamageText(AActor* DamageTaker, AActor* DamageInstigator, float DamageTaken)
{
	if (!IsValid(DamageTaker))
		return;

	// Do not run on dedicated server
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!IsValid(LocalPlayer))
		return;
	
	// Makes sure this delegate is running on the player who dealt the damage
	if (!IsValid(DamageInstigator))
		return;

	// Damage Instigator must be THIS player
	if (LocalPlayer->GetPlayerController(GetWorld()) != DamageInstigator->GetInstigatorController())
		return;
	
	if (!IsValid(DamageTextActor))
		return;
	
	FTransform SpawnTransform(DamageTaker->GetActorLocation()
		+ FVector(
			FMath::RandRange(32.f, 196.0f),
			FMath::RandRange(32.f, 196.0f),
			FMath::RandRange(32.f, 196.0f))
	);
	
	AFloatingTextBase* FloatText = GetWorld()->SpawnActor<AFloatingTextBase>(
		DamageTextActor, SpawnTransform);

	if (IsValid(FloatText))
	{
		FloatText->TextShown = FString::SanitizeFloat(DamageTaken < 0 ? 0-DamageTaken : DamageTaken);
		FloatText->TextColor = DamageTaken > 0 ? FLinearColor::Red : FLinearColor::Green;
		FloatText->SecondsToShow = 3.f;
		FloatText->UpdateFloatingText();
	}
}

void ACharacterBase::CheckAbilityStart(FName AbilityName, float CastTime)
{
	const ECombatState CombatState = VitalityComponent->GetCombatState();
	if (CombatState != ECombatState::ALERT && CombatState != ECombatState::ENGAGED)
	{
		// Check if ability requires focus
		const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
		if (AbilityData.bRequiresFocus)
		{
			VitalityComponent->SetCombatState(ECombatState::ALERT);
		}
	}
}

void ACharacterBase::CheckAbilitySuccess(FName AbilityName, bool WasSuccessful)
{
	if (!WasSuccessful)
		return;
	
	const ECombatState CombatState = VitalityComponent->GetCombatState();
	if (CombatState != ECombatState::ENGAGED)
	{
		// Check if ability requires focus and is a detrimental ability
		const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
		if (AbilityData.AbilityType == EAbilityType::DETRIMENT)
		{
			// If the character took hostile action towards another actor
			if (IsValid(AbilityComponent->GetTargetedActor()))
				VitalityComponent->SetCombatState(ECombatState::ENGAGED);
		}
	}
}

void ACharacterBase::OnRep_CharacterLevel_Implementation(int OldLevel)
{
	if (OldLevel < _CharacterLevel)
		OnCharacterLevelUp.Broadcast(_CharacterLevel);
}

void ACharacterBase::OnRep_ExperienceChanged_Implementation()
{
	OnExperienceChanged.Broadcast();
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
	DOREPLIFETIME(ACharacterBase, _CharacterRace);
	DOREPLIFETIME(ACharacterBase, _CharacterRisk);
	DOREPLIFETIME(ACharacterBase, _IsMale);
	
	DOREPLIFETIME(ACharacterBase, SkinColor);
	DOREPLIFETIME(ACharacterBase, BodyData);
	DOREPLIFETIME(ACharacterBase, PronounObjective);
	DOREPLIFETIME(ACharacterBase, PronounPossessive);
	DOREPLIFETIME(ACharacterBase, PronounSubject);
	
	DOREPLIFETIME_CONDITION(ACharacterBase, _ExperiencePoints, COND_OwnerOnly);
}
