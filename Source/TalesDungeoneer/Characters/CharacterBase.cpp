// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterBase.h"

#include "VitalityMatters/Public/lib/VitalityData.h"
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
#include "TalesDungeoneer/TalesDungeoneer.h"
#include "TalesDungeoneer/Entities/SimpleActors/FloatingTextBase.h"
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
	
	VitalityStats   = CreateDefaultSubobject<UVitalityStatComponent>(TEXT("VitalityStats"));

	
	VitalityWelfare = CreateDefaultSubobject<UVitalityWelfareComponent>(TEXT("VitalityWelfare"));
	// TODO - Add racial benefits, such as dark vision and levitation
	
	VitalityEffects = CreateDefaultSubobject<UVitalityEffectsComponent>(TEXT("VitalityEffects"));
	
	WeaponComponent		= CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
	AbilityComponent	= CreateDefaultSubobject<UAbilityComponent>(TEXT("AbilityComponent"));
	MeshMergeComponent	= CreateDefaultSubobject<UMeshMergeComponent>(TEXT("MeshMergeComponent"));

	// Allow weapon overlap collisions
	//GetCapsuleComponent()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Block);
}

/**
 * @brief Returns the value of faction of this NPC to the given target actor.
 *		Consideration will return the lowest level. So, an NPC who has 3 factions
 *		aligned with LIKE but one with HATE, this NPC will hate the given actor.
 * @param TargetActor The actor to consider
 * @param FactionValue The value of the faction between characters
 * @return The faction state of consideration
 */
EFactionState ACharacterBase::GetFactionConsideration(ACharacterBase* TargetActor, float& FactionValue)
{
	bool IsMemberOfSameFaction = false;
	EFactionState CurrentRegard = EFactionState::ALLY;
	if (IsValid(TargetActor))
	{
		
		// Loop through this actor's faction memberships, checking if any of them
		// are conflicting with the target actor's memberships.
		for (const EFaction MyFaction : GetFactionMemberships())
		{
			// "What is your state towards this current faction membership?"
			const EFactionState TheirConsideration = TargetActor->GetFactionState(MyFaction);
			if (TheirConsideration < CurrentRegard)
				CurrentRegard = TheirConsideration;
			
			// Stop the loop immediately if we're hated
			if (CurrentRegard < EFactionState::NONE)
				return EFactionState::HATE;
		}

		// Loop through the target actor's faction memberships, checking if
		// any of them are hated factions to our memberships.
		for (const EFaction TargetFaction : TargetActor->GetFactionMemberships())
		{
			// "Does your Faction Membership affect how I consider you?"
			const EFactionState FactionState = GetFactionState(TargetFaction);
			if (FactionState < CurrentRegard)
				CurrentRegard = FactionState;
			
			// Stop the loop immediately if we hate them
			if (CurrentRegard < EFactionState::NONE)
				return EFactionState::HATE;
			
		}
		return CurrentRegard;
	}
	return EFactionState::NONE;
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
	if (_CharacterClass != NewClass)
	{
		_CharacterClass = NewClass;
		ReinitializeSubsystems();
	}
}

void ACharacterBase::SetCharacterRace(ECharacterRace NewRace)
{
	if (_CharacterRace != NewRace)
	{
		_CharacterRace = NewRace;
		ReinitializeSubsystems();
	}
}

void ACharacterBase::SetFactionState(EFaction FactionEnum, EFactionState FactionState)
{
	for (FStFactionDataMap& DataMap : _FactionData.DataMap)
	{
		if (DataMap.FactionEnum == FactionEnum)
		{
			switch(FactionState)
			{
			case EFactionState::LIKE:
				DataMap.FactionValue = 100.f;
				break;
			case EFactionState::ALLY:
				DataMap.FactionValue = 500.f;
				break;
			case EFactionState::HATE:
				DataMap.FactionValue = -100.f;
				break;
			default:
				DataMap.FactionValue = 0.f;
				break;
			}
			return;
		}
	}
}

void ACharacterBase::SetFactionValue(EFaction FactionEnum, float PointsToSet)
{
	// Set the existing value, if exists
	for (FStFactionDataMap& DataMap : _FactionData.DataMap)
	{
		if (DataMap.FactionEnum == FactionEnum)
		{
			DataMap.FactionValue = PointsToSet;
			return;
		}
	}
	_FactionData.DataMap.Add(FStFactionDataMap(FactionEnum, PointsToSet));
}

void ACharacterBase::IncreaseFaction(EFaction FactionEnum, float PointsToAdd)
{
	for (FStFactionDataMap& DataMap : _FactionData.DataMap)
	{
		if (DataMap.FactionEnum == FactionEnum)
		{
			DataMap.FactionValue += abs(PointsToAdd);
			return;
		}
	}
	_FactionData.DataMap.Add(
		FStFactionDataMap(FactionEnum, abs(PointsToAdd))
		);
}

void ACharacterBase::DecreaseFaction(EFaction FactionEnum, float PointsToLose)
{
	for (FStFactionDataMap& DataMap : _FactionData.DataMap)
	{
		if (DataMap.FactionEnum == FactionEnum)
		{
			DataMap.FactionValue -= abs(PointsToLose);
			return;
		}
	}
	_FactionData.DataMap.Add(
		FStFactionDataMap(FactionEnum, abs(PointsToLose))
		);
}

void ACharacterBase::SetFactionMembership(EFaction FactionEnum, bool IsMember)
{
	if (IsMember)
		_FactionMembership.AddUnique(FactionEnum);
	else
		_FactionMembership.Remove(FactionEnum);
}


EFactionState ACharacterBase::GetFactionState(EFaction FactionToCheck) const
{
	return _FactionData.GetFactionState(FactionToCheck);
}

void ACharacterBase::SetExperiencePoints(float NewValue)
{
	_ExperiencePoints = NewValue;
}


void ACharacterBase::AddExperiencePoints(float AddValue)
{
	// Only execute on the server, or when playing standalone
	if (GetNetMode() < NM_Client) return;
	
	const float NewValue = _ExperiencePoints + FMath::Abs(AddValue);
	const float NextLevel = GetExperienceNeeded();
		
	if (NewValue >= NextLevel)
	{
		if (_CharacterLevel < UGlobalData::GetGameMaxCharacterLevel())
		{
			_CharacterLevel += 1;
			_ExperiencePoints = NewValue/NextLevel;
			
			AbilityComponent->AddUnlockPoints( UnlockPointsOnLevelUp );
			const FTransform LevelUpSpawnTransform(GetActorLocation()
				+ FVector(
					FMath::RandRange(32.f, 196.0f),
					FMath::RandRange(32.f, 196.0f),
					FMath::RandRange(32.f, 196.0f))
			);
	
			AFloatingTextBase* LevelUpText = GetWorld()->SpawnActor<AFloatingTextBase>(
				DamageTextActor, LevelUpSpawnTransform);

			if (IsValid(LevelUpText))
			{
				LevelUpText->TextShown = "LEVEL UP";
				LevelUpText->TextColor = FLinearColor::Yellow;
				LevelUpText->SecondsToShow = 3.f;
				LevelUpText->UpdateFloatingText();
			}
			OnExperienceChanged.Broadcast();
		}
		return;
	}

	const FTransform SpawnTransform(GetActorLocation()
		+ FVector(
			FMath::RandRange(32.f, 196.0f),
			FMath::RandRange(32.f, 196.0f),
			FMath::RandRange(32.f, 196.0f))
	);
	
	AFloatingTextBase* FloatText = GetWorld()->SpawnActor<AFloatingTextBase>(
		DamageTextActor, SpawnTransform);
	
	if (IsValid(FloatText))
	{
		FloatText->TextShown = "+ " + FString::FromInt(FMath::RoundToInt(AddValue)) + " XP";
		FloatText->TextColor = FLinearColor::Blue;
		FloatText->SecondsToShow = 3.f;
		FloatText->UpdateFloatingText();
	}
	
	_ExperiencePoints = NewValue > NextLevel ? NextLevel : NewValue;
	OnExperienceChanged.Broadcast();
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
		if (!VitalityWelfare->OnDamageTaken.IsAlreadyBound(this, &ACharacterBase::SpawnDamageText))
			VitalityWelfare->OnDamageTaken.AddDynamic(this, &ACharacterBase::SpawnDamageText);
	}

	// When an ability is started, check if Combat State should change
	if (!AbilityComponent->OnAbilityCastStarted.IsAlreadyBound(this, &ACharacterBase::CheckAbilityStart))
		AbilityComponent->OnAbilityCastStarted.AddDynamic(this, &ACharacterBase::CheckAbilityStart);

	// When an ability is finished, check if Combat State should change
	if (!AbilityComponent->OnAbilityCastComplete.IsAlreadyBound(this, &ACharacterBase::CheckAbilitySuccess))
		AbilityComponent->OnAbilityCastComplete.AddDynamic(this, &ACharacterBase::CheckAbilitySuccess);

	// Server Init (always happens before the client)
	if (GetNetMode() < NM_Client)
	{
		
		// Initialize the Inventory System
		// Register the Weapon Component to listen for changes to Equipment
		if (IsValid(WeaponComponent) && IsValid(InventoryComponent))
		{
			// Setup listeners first, then initialize
			if (!InventoryComponent->OnEquipmentUpdated.IsAlreadyBound(this, &ACharacterBase::UpdateWeapon))
				InventoryComponent->OnEquipmentUpdated.AddDynamic(this, &ACharacterBase::UpdateWeapon);

			ATalesGameStateBase* gState = Cast<ATalesGameStateBase>( GetWorld()->GetGameState() );
			if (IsValid(gState))
			{
				InventoryComponent->StartingItems =
					gState->GetStartingInventoryData(
						GetCharacterRace(), GetCharacterClass());
			}
			InventoryComponent->InitializeInventory();
			
			UpdateWeapon(EEquipmentSlotType::PRIMARY);
			UpdateWeapon(EEquipmentSlotType::SECONDARY);
		}
	}

	// Grant Starting Abilities
	if (IsValid(AbilityComponent))
	{
		ATalesGameStateBase* gState = Cast<ATalesGameStateBase>( GetWorld()->GetGameState() );
		if (IsValid(gState))
		{
			TArray<FName> StartingAbilities = gState->GetStartingAbilityData(GetCharacterClass());
			for (const FName& AbilityName : StartingAbilities)
			{
				AbilityComponent->AddKnownAbility(AbilityName);
			}
		}
	}

	// Reload Saved Character Data
	// Characters are saved on the client
	ATalesGameStateBase* TalesGameState = Cast<ATalesGameStateBase>(GetWorld()->GetGameState());
	if (IsValid(TalesGameState))
	{
		// If character fails to load, grant starting/default inventory
		TalesGameState->LoadCharacter(
			  TalesGameState->GetSelectedCharacterSaveSlotName(), true);
	}
	
}


void ACharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	SetCharacterName(CharacterName);

	SetCharacterClass(_CharacterClass);
	SetCharacterRace(_CharacterRace);

	ReinitializeSubsystems();
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
	if (GetNetMode() == NM_DedicatedServer)
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

float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	VitalityWelfare->DamageHealth(DamageCauser, DamageAmount);
	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

void ACharacterBase::SetCharacterName(FString ProposedName)
{
	// TODO - Add checks for symbols, special characters, etc
	CharacterName = ProposedName;
}

void ACharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}

void ACharacterBase::Client_CharacterRestored_Implementation(const FString& SaveSlotName)
{
	bCharacterSaveRestored = true;
	OnCharacterRestored.Broadcast(SaveSlotName);
}

void ACharacterBase::UpdateWeapon(EEquipmentSlotType EquipmentEnum)
{
	EWeaponSlots WeaponEnum = EWeaponSlots::NONE;
	switch(EquipmentEnum)
	{
	case EEquipmentSlotType::PRIMARY:
		WeaponEnum = EWeaponSlots::PRIMARY;
		break;
	case EEquipmentSlotType::SECONDARY:
		WeaponEnum = EWeaponSlots::SECONDARY;
		break;
	// Atomic Update. Update all weapons.
	case EEquipmentSlotType::NONE:
		UpdateWeapon(EEquipmentSlotType::PRIMARY);
		UpdateWeapon(EEquipmentSlotType::SECONDARY);
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("Invalid Equip Slot Received ( UpdateWeapon() )"));
		return;
	}
	
	const int equipmentSlot   = InventoryComponent->getEquipmentSlotNumber(EquipmentEnum);
	const FName equipmentItem = InventoryComponent->getItemNameInSlot(equipmentSlot, true);

	if (UItemSystem::getItemNameIsValid(equipmentItem))
	{
		const FStItemData ItemData = UItemSystem::getItemDataFromItemName(equipmentItem);
		if (ItemData.itemCategory == EItemCategory::WEAPON)
		{
			WeaponComponent->SetWeapon(equipmentItem, WeaponEnum);
			return;
		}
		UE_LOG(LogTemp, Warning, TEXT("%s(%s): Equipped Item '%s' was not of type 'WEAPON'"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *equipmentItem.ToString());
	}
	WeaponComponent->UnsetWeapon(WeaponEnum);
	
}

void ACharacterBase::SpawnDamageText(AActor* DamageInstigator, float DamageTaken)
{

	// Do not run on a server with no player (AKA dedicated server)
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
		FloatText->TextShown = FString::SanitizeFloat(DamageTaken < 0 ? 0-DamageTaken : DamageTaken);
		FloatText->TextColor = DamageTaken > 0 ? FLinearColor::Red : FLinearColor::Green;
		FloatText->SecondsToShow = 3.f;
		FloatText->UpdateFloatingText();
	}
}

void ACharacterBase::CheckAbilityStart(FName AbilityName, float CastTime)
{
	const ECombatState CombatState = VitalityWelfare->GetCombatState();
	if (CombatState != ECombatState::ALERT && CombatState != ECombatState::ENGAGED)
	{
		// Check if ability requires focus
		const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
		if (AbilityData.bRequiresFocus)
		{
			VitalityWelfare->SetCombatAlert();
		}
	}
}

void ACharacterBase::CheckAbilitySuccess(FName AbilityName, bool WasSuccessful)
{
	if (!WasSuccessful)
		return;
	
	const ECombatState CombatState = VitalityWelfare->GetCombatState();
	if (CombatState != ECombatState::ENGAGED)
	{
		// Check if ability requires focus and is a detrimental ability
		const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(AbilityName);
		if (AbilityData.AbilityType == EAbilityType::DETRIMENT)
		{
			// If the character took hostile action towards another actor
			if (IsValid(AbilityComponent->GetTargetedActor()))
				VitalityWelfare->SetCombatEngaged();
		}
	}
}

void ACharacterBase::ReinitializeSubsystems()
{
	// Set default starting values - Run on server only
	if (IsValid(VitalityStats) && GetNetMode() < NM_Client)
	{
		// Set the initial value of the stats groups
		for (int i = 0; i < UVitalitySystem::GetNumberOfCoreStats(); i++)
			VitalityStats->StartingStats.CoreStats[i] = 100.f;
		
		for (int i = 0; i < UVitalitySystem::GetNumberOfDamageTypes(); i++)
		{
			VitalityStats->StartingStats.DamageBonuses[i] = 0.f;
			VitalityStats->StartingStats.DamageResists[i] = 0.f;
		}

		/* For each for the stat groups, it will convert the key to int and
		 * set the corresponding stat value by int index, for all modifiers that exist.
		 */
		if (IsValid(VitalityStats))
		{
			// Set initial values based on race
			ATalesGameStateBase* gState = Cast<ATalesGameStateBase>( GetWorld()->GetGameState() );
			if (IsValid(gState))
			{
				const FStCharacterRaces StartingRaceData = gState->GetStartingRaceData(GetCharacterRace());
				for (const TPair<EVitalityStat, float> CoreStat : StartingRaceData.CoreStats)
					VitalityStats->StartingStats.CoreStats[static_cast<int>(CoreStat.Key)] += CoreStat.Value;

				for (const TPair<EDamageType, float> DamageStat : StartingRaceData.DamageBonuses)
					VitalityStats->StartingStats.DamageBonuses[static_cast<int>(DamageStat.Key)] += DamageStat.Value;

				for (const TPair<EDamageType, float> DamageStat : StartingRaceData.DamageResists)
					VitalityStats->StartingStats.DamageResists[static_cast<int>(DamageStat.Key)] += DamageStat.Value;
	
				// Add/Subtract additional values based on class
				const FStCharacterClasses StartingClassData = gState->GetStartingClassData(GetCharacterClass());
				for (const TPair<EVitalityStat, float> CoreStat : StartingClassData.CoreStats)
					VitalityStats->StartingStats.CoreStats[static_cast<int>(CoreStat.Key)] += CoreStat.Value;

				for (const TPair<EDamageType, float> DamageStat : StartingClassData.DamageBonuses)
					VitalityStats->StartingStats.DamageBonuses[static_cast<int>(DamageStat.Key)] += DamageStat.Value;

				for (const TPair<EDamageType, float> DamageStat : StartingClassData.DamageResists)
					VitalityStats->StartingStats.DamageResists[static_cast<int>(DamageStat.Key)] += DamageStat.Value;
				
			}
		}
	}
}

void ACharacterBase::OnRep_CharacterName_Implementation()
{
	OnCharacterNameChanged.Broadcast();
}

void ACharacterBase::OnRep_CharacterLevel_Implementation(int OldLevel)
{
	const int NewLevel = GetCharacterLevel();
	if (OldLevel < NewLevel)
		OnCharacterLevelUp.Broadcast(NewLevel);
	OnCharacterLevelChanged.Broadcast();
}

void ACharacterBase::OnRep_ExperienceChanged_Implementation(float OldExperience)
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
	
	DOREPLIFETIME(ACharacterBase, _FactionMembership);
	DOREPLIFETIME(ACharacterBase, _FactionData);
	DOREPLIFETIME(ACharacterBase, _CharacterTeam);
	DOREPLIFETIME(ACharacterBase, _CharacterLevel);
	DOREPLIFETIME(ACharacterBase, _CharacterClass);
	DOREPLIFETIME(ACharacterBase, _CharacterRace);
	DOREPLIFETIME(ACharacterBase, _CharacterRisk);
	DOREPLIFETIME(ACharacterBase, CharacterName);
	DOREPLIFETIME(ACharacterBase, _IsMale);
	
	DOREPLIFETIME(ACharacterBase, SkinColor);
	DOREPLIFETIME(ACharacterBase, PronounObjective);
	DOREPLIFETIME(ACharacterBase, PronounPossessive);
	DOREPLIFETIME(ACharacterBase, PronounSubject);
	
	DOREPLIFETIME_CONDITION(ACharacterBase, _ExperiencePoints, COND_OwnerOnly);
}
