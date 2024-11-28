// Starcache Studios, LLC (c) 2024


#include "Characters/CharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Characters/Components/EquipmentComponent.h"
#include "Components/CapsuleComponent.h"
#include "DataAssets/CharacterDefaults.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Gamemode/BaseFiles/TalesGameStateBase.h"
#include "lib/datastructures/TalesGlobalData.h"
#include "Saves/SavedCharacters.h"
#include "Kismet/GameplayStatics.h"
#include "lib/Tags/TalesGlobalTags.h"
#include "RsAbilityComponent.h"
#include "Attributes/RsCoreAttributeSet.h"
#include "Attributes/RsDamageAttributeSet.h"
#include "Attributes/RsAbilityAttributeSet.h"
#include "Attributes/RsVitalityAttributeSet.h"
#include "Characters/Controllers/PlayerControllerBase.h"
#include "Abilities/RsGameplayAbilityBase.h"
#include "Interfaces/RsAnimInstance.h"

#include "Logging/StructuredLog.h"

float GetAttributeValue(const TMap<FGameplayAttribute, int>& AttributeMap, const FGameplayAttribute& SearchAttribute)
{
	if (!AttributeMap.IsEmpty() && SearchAttribute.IsValid())
	{
		if (AttributeMap.Contains(SearchAttribute))
		{
			return *AttributeMap.Find(SearchAttribute);
		}
	}
	return 0.f;
}

float ModifiedStatValue(float ModifierValue)
{
	return FMath::Clamp(STAT_DEFAULT * (1 + (ModifierValue * 0.02)), 0.f, STAT_MAX);
}

// Sets default values - Constructor
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

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity				= 700.f;
	GetCharacterMovement()->AirControl					= 0.35f;
	GetCharacterMovement()->MaxWalkSpeed				= 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed			= 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking	= 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	MeleeStrikeDetector = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MeleeStrikeDetector"));
	MeleeStrikeDetector->SetupAttachment(GetMesh(), NAME_None);
	MeleeStrikeDetector->SetRelativeLocation(FVector(-20.f, 60.f, 100.f));
	MeleeStrikeDetector->SetCapsuleHalfHeight(48.f);
	MeleeStrikeDetector->SetCapsuleRadius(32.f);

	AbilitySystemComponent = CreateDefaultSubobject<URsAbilityComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);

	AttributeVitalitySet	= CreateDefaultSubobject<URsVitalityAttributeSet>("AttributeVitalitySet");
	AttributeCoreStatsSet	= CreateDefaultSubobject<URsCoreAttributeSet>("AttributeCoreStatsSet");
	AttributeDamageSet		= CreateDefaultSubobject<URsDamageAttributeSet>("AttributeDamageSet");
	AttributeEffectSet		= CreateDefaultSubobject<URsAbilityAttributeSet>("AttributeEffectSet");


	// Setup listeners for when the inventory changes
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SaveFolder = UTalesGlobalData::InventorySaveFolder();
	if (!InventoryComponent->OnInventoryUpdated.IsAlreadyBound(this, &ACharacterBase::InventoryUpdateDelegate))
	{
		InventoryComponent->OnInventoryUpdated.AddDynamic(this, &ACharacterBase::InventoryUpdateDelegate);
	}

	if (!InventoryComponent->OnInventoryRestored.IsAlreadyBound(this, &ACharacterBase::InventoryRestoredDelegate))
	{
		InventoryComponent->OnInventoryRestored.AddDynamic(this, &ACharacterBase::InventoryRestoredDelegate);
	}

	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>(TEXT("EquipmentComponent"));
    if (!EquipmentComponent->OnEquipmentSlotToggled.IsAlreadyBound(this, &ACharacterBase::EquipmentUpdateDelegate))
	{
		EquipmentComponent->OnEquipmentSlotToggled.AddDynamic(this, &ACharacterBase::EquipmentUpdateDelegate);
	}

	MeshMergeComponent = CreateDefaultSubobject<UMeshMergeComponent>(TEXT("MeshMergeComponent"));

	// Allow weapon overlap collisions
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Block);

}

bool ACharacterBase::GetIsBlockingAttacks() const
{
	if (IsValid(AbilitySystemComponent))
	{
		if (AbilitySystemComponent->HasMatchingGameplayTag(TAG_Effects_Blocking_Physical.GetTag()))
		{
			return true;
		}
	}

	if (IsValid(EquipmentComponent))
	{
		//return EquipmentComponent->GetIsBlocking();
	}

	return false;
}

/**
 *  Calculates difficulty of this character if fought by the player characters.
 *  Takes into account other characters part of this characters group.
 * \return A percentage - The likelihood of success for this encounter
 */
float ACharacterBase::GetRiskLevel() const
{
	return 1.f;
}

void ACharacterBase::SetCharacterRace(const FGameplayTag& NewRaceTag)
{
	if (!HasAuthority())
		return;

	if (NewRaceTag.GetGameplayTagParents().HasTag(TAG_Character_Race.GetTag()))
	{
		const FGameplayTag OldRaceTag = GetCharacterRace();
		CharacterRace_ = NewRaceTag;
		const UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponent();
		if (IsValid(AbilitySystem))
		{
			UpdateCoreStats();
		}
		OnCharacterRaceChanged.Broadcast(OldRaceTag, NewRaceTag);
	}
}

void ACharacterBase::SetCharacterClass(const FGameplayTag& NewClassTag)
{
	if (!HasAuthority())
		return;

	if (NewClassTag.GetGameplayTagParents().HasTag(TAG_Character_Class))
	{
		const FGameplayTag OldClassTag = GetCharacterClass();
		CharacterClass_ = NewClassTag;
		const UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponent();
		if (IsValid(AbilitySystem))
		{
			UpdateCoreStats();
		}
		OnCharacterClassChanged.Broadcast(OldClassTag, NewClassTag);
	}
}

UCharacterRaceData* ACharacterBase::GetCharacterRaceData() const
{
	return !IsValid(CharacterData) ? nullptr : CharacterData->CharacterRaceData;
}

UCharacterClassData* ACharacterBase::GetCharacterClassData() const
{
	return !IsValid(CharacterData) ? nullptr : CharacterData->CharacterClassData;
}

FString ACharacterBase::GetCharacterSafeName() const
{
	if (CharacterName.IsEmpty()) { return ""; }
	FString CleanedName = CharacterName.Replace(TEXT(" "), TEXT(""), ESearchCase::IgnoreCase);
	return CleanedName;
}

/**
 * Saves the character to file. If being saved asynchronously, the returned
 * save object will be the updated object as it is right before the save writes to file.
 * Triggers 'OnCharacterSaved' upon completion (async or not).
 * \param SaveObject The save data being carried between child/parent
 * \param bRunAsync True if the save should run asynchronously
 * \return The USavedCharacter SaveGame object for this character.
 */
USaveGame* ACharacterBase::SaveCharacter(USaveGame* SaveObject, bool bRunAsync)
{
	if (!bCharacterReady)
	{
		UE_LOGFMT(LogTemp, Warning,
			"{Name}({Sv}): Failed to Save Character - ACharacterBase has not"
			" finished initialization.", GetName(), HasAuthority()?"SRV":"CLI");
		return nullptr;
	}

	USavedCharacter* SavedCharacter = Cast<USavedCharacter>( SaveObject );

	// Character has not yet been loaded, or it is a new character
	if (!IsValid(SavedCharacter))
	{
		const ATalesGameStateBase* TalesGameState = Cast<ATalesGameStateBase>
				( GetWorld()->GetGameState() );

		// Validate the game state
		if (!IsValid(TalesGameState))
		{
			UE_LOGFMT(LogTemp, Error,
				"{Name}({Sv}): Failed to Save Character - GameState "
				"is not a Tales-type GameState Object", GetName(), HasAuthority()?"SRV":"CLI");
			return nullptr;
		}

		SavedCharacter = Cast<USavedCharacter>
			( UGameplayStatics::CreateSaveGameObject(USavedCharacter::StaticClass()) );

		SavedCharacter->SaveSlotName = TalesGameState->
			GenerateAlphanumeric(UTalesGlobalData::CharacterSaveFolder());

		SavedCharacter->UserIndex = GetCharacterUserIndex();
	}

	// Write ACharacterBase* specific data to SaveObject ( SavedCharacter )
	SavedCharacter->CharacterData.CharacterName    = GetCharacterName();
	SavedCharacter->CharacterData.CharacterClass   = GetCharacterClass();
	SavedCharacter->CharacterData.CharacterRace    = GetCharacterRace();
	SavedCharacter->CharacterData.ExperiencePoints = 0.f;
	SavedCharacter->CharacterData.CharacterLevel   = 1;

	// If the inventory save does not exist, then this is a new inventory
	// Existing inventories will have a save name
	{
		FString InventoryResponse = "";
		const bool inventorySaveExists = !InventoryComponent->GetInventorySaveName().IsEmpty();
		if (inventorySaveExists)
		{
			// Issue starting items and save
			InventoryComponent->IssueStartingItems();
		}
		SavedCharacter->SavedInventory = InventoryComponent->SaveInventory(InventoryResponse, inventorySaveExists);
	}

	// Save the Mesh Merge data
	{
		// Runs async if the save already exists
		FString MeshMergeResponse = "";
		FString newSaveName = MeshMergeComponent->SaveMeshMerge(
			MeshMergeResponse, !MeshMergeComponent->GetMeshMergeSaveName().IsEmpty());
		SavedCharacter->SavedMeshMerge = newSaveName;
	}

	if (bRunAsync)
	{
		FAsyncSaveGameToSlotDelegate SaveDelegate;
		SaveDelegate.BindUObject(this, &ACharacterBase::SaveGameDelegate);
		UGameplayStatics::AsyncSaveGameToSlot(SavedCharacter,
			SavedCharacter->SaveSlotName, SavedCharacter->UserIndex, SaveDelegate);
		return SavedCharacter;
	}

	const bool bSaved = UGameplayStatics::SaveGameToSlot(SavedCharacter,
			SavedCharacter->SaveSlotName, SavedCharacter->UserIndex);
	if (OnCharacterSaved.IsBound())
	{
		OnCharacterSaved.Broadcast(SavedCharacter->SaveSlotName, SavedCharacter->UserIndex, bSaved);
	}
	if (bSaved)
	{
		return SavedCharacter;
	}
	return nullptr;
}

/**
 * Processes loading a character from a save. Does not work if 'bCharacterRestored'
 * is already set. Will not run if the character has not initialized. Internally
 * calls 'OnCharacterRestored' after load data has been processed. The SlotName and UserIndex
 * is only used by the async delegate. If LoadCharacter is called manually, these values will not be
 * * used and are obtained from the ATalesGameState.
 * \param SlotName The save name to be restored. Only set in async call. Ignored if called manually.
 * \param UserIndex The user index for the character. Only set in async call. Ignored if called manually.
 * \param SaveGame When used as an async load delegate, this is the save object loaded.
 */
bool ACharacterBase::LoadCharacter(const FString& SlotName, const int32 UserIndex, USaveGame* SaveGame)
{
	bool bWasSuccess = false;

	// Prevents the character from loading a save until the defaults have initialized
	if (!bCharacterReady)
	{
		UE_LOGFMT(LogTemp, Display, "{Character}({Sv}): "
			"LoadCharacter() Requested before character was ready. Delaying...",
			GetName(), HasAuthority()?"SRV":"CLI");
		if (OnCharacterRestored.IsBound()) { OnCharacterRestored.Broadcast(bWasSuccess); }
		return false;
	}

	// If the save data is not valid, attempt to find it
	// This will also run if LoadCharacter is called synchronously
	USavedCharacter* SavedCharacter = Cast<USavedCharacter>(SaveGame);
	if (!IsValid(SavedCharacter))
	{
		const ATalesGameStateBase* TalesGameState = Cast<ATalesGameStateBase>
				( GetWorld()->GetGameState() );

		// If the game state is valid, we can attempt to get the selected character
		if (IsValid(TalesGameState))
		{
			if (!TalesGameState->GetIsSaveMetaReady())
			{
				UE_LOGFMT(LogTemp, Display, "{Character}({Sv}): "
					"LoadCharacter() Failed - GameState not ready. Delaying...",
					GetName(), HasAuthority()?"SRV":"CLI");
				return false;
			}

			// If the selected character has an associated save, we are good to go
			if (UGameplayStatics::DoesSaveGameExist(
				TalesGameState->GetCharacterSlotName(),
				TalesGameState->GetCharacterUserIndex()))
			{
				SavedCharacter = Cast<USavedCharacter>(
						UGameplayStatics::LoadGameFromSlot(
							TalesGameState->GetCharacterSlotName(),
							TalesGameState->GetCharacterUserIndex()) );
			}
			// Otherwise, there is no saved character to be loaded
			else
			{
				UE_LOGFMT(LogTemp, Warning, "{Character}({Sv}): "
					"Character Save '{SlotName} ({UserIndex})' does not exist, "
					"or Player is in the Character Creator.",
					GetName(), HasAuthority()?"SRV":"CLI",
					TalesGameState->GetCharacterSlotName(),
					TalesGameState->GetCharacterUserIndex());
			}
		}
		// Game State is not valid or ready
		else
		{
			UE_LOGFMT(LogTemp, Error, "{Character}({Sv}): "
				"Cannot load character - TalesGameState is not ready, or invalid.",
				GetName(), HasAuthority()?"SRV":"CLI");
		}
	}

	// Perform load from data, if the save object now exists
	if (IsValid(SavedCharacter))
	{
		// Send Character Data to server for restoration
		RestoreCharacter(SavedCharacter->CharacterData);

		// Restore Inventory Data
		FString InventoryResponse = "", MeshMergeResponse = "";
		InventoryComponent->LoadInventory(InventoryResponse,
			SavedCharacter->SavedInventory, SavedCharacter->UserIndex, true);

		// This should only be called by a playable client
		if (!HasAuthority() || GetNetMode() == NM_ListenServer)
		{
			MeshMergeComponent->LoadMeshMerge(MeshMergeResponse,
				SavedCharacter->SavedMeshMerge, SavedCharacter->UserIndex);
		}
		bWasSuccess = true;
	}

	if (OnCharacterRestored.IsBound()) { OnCharacterRestored.Broadcast(bWasSuccess); }
	return bWasSuccess;
}

void ACharacterBase::SetEquipmentEnabled(int SlotNumber, bool bMakeReady)
{
	if (InventoryComponent->IsValidEquipmentSlot(SlotNumber))
	{
		EquipmentComponent->ToggleEquipment(SlotNumber, bMakeReady);

		const UEquipmentItemDataAsset* EquipmentData =
			Cast<UEquipmentItemDataAsset>(InventoryComponent->GetItemDataInSlotNumber(SlotNumber));

		if (IsValid(EquipmentData))
		{
			AActor* ChildActor = nullptr;
			if (InventoryComponent->GetPrimarySlotNumber() == SlotNumber)
			{
				ChildActor = InventoryComponent->PrimarySlotChildActor;
			}
			else if (InventoryComponent->GetPrimarySlotNumber() == SlotNumber)
			{
				ChildActor = InventoryComponent->PrimarySlotChildActor;
			}

			// Move child to armed attachment
			if (IsValid(ChildActor))
			{
				const FEquipmentAttachSubdata AttachmentData = (bMakeReady || EquipmentData->bIsPassive)
					? EquipmentData->AttachmentData.AttachDataArmed
					: EquipmentData->AttachmentData.AttachDataStowed;

				const FAttachmentTransformRules AttachRules(
					EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget
					, EAttachmentRule::KeepWorld, true);

				InventoryComponent->PrimarySlotChildActor->AttachToComponent(GetMesh(), AttachRules, AttachmentData.BoneAttachment);
				ChildActor->SetActorRelativeLocation(AttachmentData.OffsetPosition);
				ChildActor->SetActorRelativeRotation(AttachmentData.OffsetRotation);
				ChildActor->SetActorRelativeScale3D(AttachmentData.OffsetScale);
			}
		}
	}
}

void ACharacterBase::SetEquipmentEnabled(const FGameplayTag& EquipmentTag, bool bMakeReady)
{
	SetEquipmentEnabled(InventoryComponent->GetSlotNumberByTag(EquipmentTag), bMakeReady);
}

void ACharacterBase::ToggleEquipment(int SlotNumber)
{
	SetEquipmentEnabled(SlotNumber, !EquipmentComponent->GetIsReady(SlotNumber));
}

void ACharacterBase::ToggleEquipment(const FGameplayTag& EquipmentTag)
{
	ToggleEquipment(InventoryComponent->GetSlotNumberByTag(EquipmentTag));
}

/**
 * \brief Performs actions after an ability is activated (such as an attack animation)
 * \param AbilitySpec The ability that was successfully activated
 */
void ACharacterBase::AbilityActivatedDelegate(const URsGameplayAbilityBase* AbilitySpec)
{
	if (IsValid(AbilitySpec))
	{
		if (GetNetMode() < NM_Client)
		{
			Multicast_AttackAnimation(AbilitySpec->AttackType);
		}
		else
		{
			DoAttackAnimation(AbilitySpec->AttackType);
		}
	}
}

void ACharacterBase::DoAttackAnimation(const FGameplayTag& AttackType)
{
	// Play the corresponding attack animation
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance))
	{
		if (AnimInstance->GetClass()->ImplementsInterface(UAttackAnimInterface::StaticClass()))
		{
			UE_LOGFMT(LogTemp, Display, "{Name}({NetAuthority}): Multicast_AttackAnimation() - Execute_AttackAnimationType({AnimInstance}, {AttackEnum})"
				, GetName(), HasAuthority() ? "SERVER" : "CLIENT", AnimInstance->GetClass()->GetName(), AttackType.ToString());
			IAttackAnimInterface::Execute_AttackAnimationType(AnimInstance, AttackType);
		}
		else
		{
			UE_LOGFMT(LogTemp, Error, "{Name}({NetAuthority}): Multicast_AttackAnimation() - AnimInstance is Valid ({AnimInstance}), but does not implement AnimAttackInterface"
				, GetName(), HasAuthority() ? "SERVER" : "CLIENT", AnimInstance->GetClass()->GetName());
		}
	}
	else
	{
		UE_LOGFMT(LogTemp, Warning, "{Name}({NetAuthority}): Multicast_AttackAnimation() - AnimInstance is invalid or does not implement UAttackAnimInterface"
			, GetName(), HasAuthority() ? "SERVER" : "CLIENT");
	}
}


void ACharacterBase::Multicast_AttackAnimation_Implementation(const FGameplayTag& AttackType)
{
	DoAttackAnimation(AttackType);
}


// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	BindListeners();
	Super::BeginPlay();
	bCharacterReady = true;

	// Load the default unarmed ability
	if (IsValid(GetAbilitySystemComponent()))
	{
		if (HasAuthority())
		{
			APlayerController* PlayerController = Cast<APlayerController>(GetController());
			if (APlayerControllerBase* TalesController = Cast<APlayerControllerBase>(PlayerController))
			{
				const TSubclassOf<URsGameplayAbilityBase> TalesAbilityClass{AbilitySystemComponent->DefaultAttackAbility};
				TalesController->SetPrimaryActionAbility(TalesAbilityClass);
			}
		}
	}

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->OnAbilityActivated.AddDynamic(this, &ACharacterBase::AbilityActivatedDelegate);
	}

	LoadCharacter(); // Attempt to call the game state to load this character

}

void ACharacterBase::OnDamageReceived(class URsAbilityComponent* SourceAsc, const float UnmitigatedDamage,
	const float MitigatedDamage)
{

}


void ACharacterBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ACharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void ACharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
}

void ACharacterBase::CharacterRestoredFromSave(const bool bWasSuccess)
{
	//bCharacterSaveRestored = bWasSuccess;
	if (OnCharacterRestored.IsBound()) { OnCharacterRestored.Broadcast(bWasSuccess); }
	if (HasAuthority() && bWasSuccess)
	{
		Client_CharacterRestored(bWasSuccess);
	}
}

void ACharacterBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	const bool doServerSave =	HasAuthority() &&   bSavesOnServer;
	const bool doClientSave = ! HasAuthority() && ! bSavesOnServer;

	if ( !doServerSave || !doClientSave )
	{
		// Always allow save if this client is the listen server or standalone
		const ENetMode netMode = GetNetMode();
		if (netMode != NM_ListenServer && netMode != NM_Standalone)
		{
			Super::EndPlay(EndPlayReason);
			return;
		}
	}

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

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void ACharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACharacterBase::SaveGameDelegate(const FString& SlotName, const int32 UserIndex, bool bSaved)
{
	OnCharacterSaved.Broadcast(SlotName, UserIndex, bSaved);
}

void ACharacterBase::AbilityInputReceived()
{
}

/**
 * \brief Called whenever an inventory slot changes or updates
 * Does not handle weapon meshes, which is handled separately.
 * \param SlotNumberUpdated The slot number that was updated
 */
void ACharacterBase::InventoryUpdateDelegate(int SlotNumberUpdated)
{
	// If equipment updated, send update to equipment component
	if (InventoryComponent->IsValidEquipmentSlot(SlotNumberUpdated))
	{
		if (HasAuthority())
		{
			if (IsValid(EquipmentComponent))
			{
				// Initialize the Equipment Item (undo old item data and acquire the new one)
				EquipmentComponent->InitEquipmentItem(SlotNumberUpdated);
			}
		}
	}
}

void ACharacterBase::EquipmentUpdateDelegate(int SlotNumberUpdated, bool bIsEnabled)
{
	const FItemStatics& ItemData = InventoryComponent->GetItemInSlotNumber(SlotNumberUpdated);
	const UEquipmentItemDataAsset* EquipData = Cast<UEquipmentItemDataAsset>(ItemData.GetDataAsset());

	if (HasAuthority())
	{
		const bool IsPrimarySlot   = SlotNumberUpdated == InventoryComponent->GetPrimarySlotNumber();
		const bool IsSecondarySlot = SlotNumberUpdated == InventoryComponent->GetSecondarySlotNumber();

		// If the changed slot was the primary or secondary slot, handle the changes to primary/secondary abilities
		if (IsPrimarySlot || IsSecondarySlot)
		{
			if (APlayerControllerBase* TalesController = Cast<APlayerControllerBase>(GetController()))
			{
				TSubclassOf<URsGameplayAbilityBase> NewAbility;

				// Determine the ability based on the slot and equipment data
				if (IsValid(EquipData) && IsValid(EquipData->ActivatedAbility))
				{
					NewAbility = EquipData->ActivatedAbility;
				}
				else
				{
					NewAbility = IsPrimarySlot ? AbilitySystemComponent->DefaultAttackAbility
											   : AbilitySystemComponent->DefaultBlockAbility;
				}

				// Remove the old ability


				// Assign the ability to the correct slot
				if (IsPrimarySlot)
				{
					if (PrimaryAbility.IsValid())
					{
						AbilitySystemComponent->ClearAbility(FGameplayAbilitySpecHandle(PrimaryAbility));
					}

					// The server grants the ability and the client updates the control binding
					PrimaryAbility = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(NewAbility, 1, 0, this));
					TalesController->SetPrimaryActionAbility({NewAbility});
				}
				else
				{
					if (SecondaryAbility.IsValid())
					{
						AbilitySystemComponent->ClearAbility(FGameplayAbilitySpecHandle(SecondaryAbility));
					}
					SecondaryAbility = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(NewAbility, 1, 0, this));
					TalesController->SetSecondaryActionAbility({NewAbility});
				}
			}
		}
	}

	// If the equipment item is NOT a child actor, update the mesh merge data
	if (IsValid(EquipData))
	{
		if (!EquipData->AttachmentData.bSpawnActor)
		{
			MeshMergeComponent->PerformMeshMerge();
		}
	}
}

void ACharacterBase::OnVitalityAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnAttributeUpdated.Broadcast(Data.Attribute, Data.NewValue);
	UE_LOGFMT(LogTemp, Display, "{Name}({NetAuthority}): OnVitalityAttributeChanged({AttributeName}) ({OldValue} -> {AttributeValue})"
		, GetName(), HasAuthority() ? "SERVER":"CLIENT", Data.Attribute.AttributeName, Data.OldValue, Data.NewValue);
	if (Data.Attribute == AttributeVitalitySet->GetCurrentHealthAttribute())
	{
		OnAttributeHealthUpdated.Broadcast(Data.OldValue, Data.NewValue);
		EventHealthChanged(Data.OldValue, Data.NewValue);
		AbilitySystemComponent->CheckIfDead();
	}
	else if (Data.Attribute == AttributeVitalitySet->GetCurrentStaminaAttribute())
	{
		OnAttributeStaminaUpdated.Broadcast(Data.OldValue, Data.NewValue);
		EventStaminaChanged(Data.OldValue, Data.NewValue);
	}
	else if (Data.Attribute == AttributeVitalitySet->GetCurrentMagicAttribute())
	{
		OnAttributeMagicUpdated.Broadcast(Data.OldValue, Data.NewValue);
		EventMagicChanged(Data.OldValue, Data.NewValue);
	}
}

void ACharacterBase::OnCoreStatsChanged(const FOnAttributeChangeData& Data)
{
	OnAttributeUpdated.Broadcast(Data.Attribute, Data.NewValue);
}

void ACharacterBase::OnDamageStatsChanged(const FOnAttributeChangeData& Data)
{
	OnAttributeUpdated.Broadcast(Data.Attribute, Data.NewValue);
}

void ACharacterBase::OnRep_CharacterRace_Implementation(const FGameplayTag& OldRace)
{
	OnCharacterRaceChanged.Broadcast(OldRace, GetCharacterRace());
}

void ACharacterBase::OnRep_CharacterClass_Implementation(const FGameplayTag& OldClass)
{
	OnCharacterClassChanged.Broadcast(OldClass, GetCharacterClass());
}

void ACharacterBase::CharacterDeath_Internal()
{
}

void ACharacterBase::CharacterRevived_Internal()
{
}

/**
 * \brief Takes standard UE-style damage and tries to apply it to the Gameplay Ability System
 * \return
 */
float ACharacterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                 AController* EventInstigator, AActor* DamageCauser)
{
	const float SuperDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	/*
	UE_LOGFMT(LogTemp, Display, "{Name}({NetAuthority}): Taking Damage! (Incoming = {DamageAmount}, Causer = {DamageCauser}"
		, GetName(), HasAuthority()?"SERVER":"CLIENT", DamageAmount, DamageCauser->GetName());
	if (URsAbilityComponent* AbilitySystem = GetAbilitySystemComponent())
	{
		if (IsValid(AbilitySystem->DamageCalcEffect))
		{
			FGameplayEffectContextHandle ContextHandle = AbilitySystem->MakeEffectContext();
			if (ContextHandle.IsValid())
			{
				FGameplayEffectSpecHandle DamageSpecHandle = AbilitySystem->MakeOutgoingSpec(
					AbilitySystem->DamageCalcEffect, 1.f, ContextHandle);
				if (DamageSpecHandle.IsValid())
				{
					DamageSpecHandle.Data->SetSetByCallerMagnitude(TAG_Damage_SetByCaller, DamageAmount);
					if (DamageSpecHandle.Data.IsValid())
					{
						AbilitySystem->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());
						UE_LOGFMT(LogTemp, Display, "{Name}({NetAuthority}): Applying Effect Spec '{EffectSpec}' to Self"
							, GetName(), HasAuthority()?"SERVER":"CLIENT", DamageSpecHandle.Data.Get()->ToSimpleString());
					}
					else
					{
						UE_LOGFMT(LogTemp, Error, "{Name}({NetAuthority}): Damage Spec Handle Data is INVALID"
							, GetName(), HasAuthority()?"SERVER":"CLIENT");
					}
				}
				else
				{
					UE_LOGFMT(LogTemp, Error, "{Name}({NetAuthority}): Damage Spec Handle was INVALID"
						, GetName(), HasAuthority()?"SERVER":"CLIENT");
				}
				return DamageAmount;
			}
			UE_LOGFMT(LogTemp, Warning, "{Name}({NetAuthority}): ContextHandle was INVALID"
				, GetName(), HasAuthority()?"SERVER":"CLIENT");
		}
	}
	UE_LOGFMT(LogTemp, Warning, "{Name}({NetAuthority}): Failed to apply damage to Gameplay Ability System"
		, GetName(), HasAuthority()?"SERVER":"CLIENT");
	*/
	return SuperDamage;
}

void ACharacterBase::InventoryRestoredDelegate(bool bWasSuccess)
{
	if (!HasAuthority())
		return;

	// Check for equipment updates
	const int PrimarySlotNumber = InventoryComponent->GetPrimarySlotNumber();
	const int SecondarySlotNumber = InventoryComponent->GetSecondarySlotNumber();

	const UEquipmentItemDataAsset* PrimaryEquipmentItem   = Cast<UEquipmentItemDataAsset>(InventoryComponent->GetItemDataInSlotNumber(PrimarySlotNumber));
	const UEquipmentItemDataAsset* SecondaryEquipmentItem = Cast<UEquipmentItemDataAsset>(InventoryComponent->GetItemDataInSlotNumber(PrimarySlotNumber));

	// Allows saved inventories to update the equipment / controller with the item in that slot
	if (IsValid(PrimaryEquipmentItem))
		EquipmentUpdateDelegate(PrimarySlotNumber, PrimaryEquipmentItem->bIsPassive);

	if (IsValid(SecondaryEquipmentItem))
		EquipmentUpdateDelegate(SecondarySlotNumber, SecondaryEquipmentItem->bIsPassive);
}

void ACharacterBase::OnDeathStatusChanged(const bool bIsNowDead, const float HealthAtDeath)
{
	if (bIsNowDead)
	{
		CharacterDeath_Internal();
		CharacterDeath();
	}
	else
	{
		CharacterRevived_Internal();
		CharacterRevived();
	}
}

void ACharacterBase::OnRep_FactionsData_Implementation(const TArray<FStFactionData>& OldFactionsData)
{
	TSet<EFaction> ProcessedFactions;

	// Check for changes or additions in current FactionStandings
	for (const FStFactionData& NewFaction : FactionStandings)
	{
		ProcessedFactions.Add(NewFaction.FactionEnum);

		const FStFactionData* OldFaction = OldFactionsData.FindByPredicate([&](const FStFactionData& Faction) {
			return Faction.FactionEnum == NewFaction.FactionEnum;
		});

		// Handle new or updated factions
		if (!OldFaction || OldFaction->FactionValue != NewFaction.FactionValue)
		{
			OnCharacterFactionUpdated.Broadcast(NewFaction.FactionEnum, NewFaction.GetFactionState());
		}
	}

	// Check for removed factions in OldFactionsData
	for (const FStFactionData& OldFaction : OldFactionsData)
	{
		// If a faction was in OldFactionsData but not in FactionStandings, it was removed
		if (!ProcessedFactions.Contains(OldFaction.FactionEnum))
		{
			OnCharacterFactionUpdated.Broadcast(OldFaction.FactionEnum, EFactionState::NONE);
		}
	}
}

void ACharacterBase::RestoreCharacter(const FCharacterData& RestoreData)
{
	// If we are the authority, authorize the restoration
	if (HasAuthority())
	{
		SetCharacterName(RestoreData.CharacterName);
		SetCharacterRace(RestoreData.CharacterRace);
		SetCharacterClass(RestoreData.CharacterClass);

		// Update restored flags to prevent cheating
		bCharacterSaveRestored = true;
		if (OnCharacterRestored.IsBound())
		{
			OnCharacterRestored.Broadcast(true);
		}
		Client_CharacterRestored(true);
	}

	// If we are not the authority, send the data to the server to be handled
	else if (this == UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
	{
		Server_RestoreCharacter(RestoreData);
	}
}

EFactionState ACharacterBase::GetFactionState(EFaction FactionEnum) const
{
	for (auto& CharacterFaction : FactionStandings)
	{
		if (CharacterFaction == FactionEnum)
		{
			return CharacterFaction.GetFactionState();
		}
	}
	return EFactionState::NONE;
}

void ACharacterBase::SetFactionState(EFaction FactionEnum, EFactionState FactionState)
{
	const int ExistingIndex = FactionStandings.Find(FStFactionData(FactionEnum));
	if (ExistingIndex != INDEX_NONE)
	{
		FactionStandings[ExistingIndex].SetFactionState(FactionState);
		OnCharacterFactionUpdated.Broadcast(FactionEnum, FactionStandings[ExistingIndex].GetFactionState());
		return;
	}
	FStFactionData NewFactionData(FactionEnum, FactionState);
	FactionStandings.Add(NewFactionData);
	OnCharacterFactionUpdated.Broadcast(FactionEnum, FactionStandings[ExistingIndex].GetFactionState());
}

void ACharacterBase::UpdateFactionState(EFaction FactionEnum, float StateModifier)
{
	const int ExistingIndex = FactionStandings.Find(FStFactionData(FactionEnum));
	if (ExistingIndex != INDEX_NONE)
	{
		const float OldValue = FactionStandings[ExistingIndex].GetFactionValue();
		FactionStandings[ExistingIndex].SetFactionValue(OldValue + StateModifier);
		OnCharacterFactionUpdated.Broadcast(FactionEnum, FactionStandings[ExistingIndex].GetFactionState());
		return;
	}
	FStFactionData NewFactionData(FactionEnum, EFactionState::NONE);
	NewFactionData.SetFactionValue(NewFactionData.GetFactionValue() + StateModifier);
	FactionStandings.Add(NewFactionData);
	OnCharacterFactionUpdated.Broadcast(FactionEnum, FactionStandings[ExistingIndex].GetFactionState());
}

void ACharacterBase::UpdateCoreStats()
{
	if (!HasAuthority())
		return;
	URsAbilityComponent* AbilitySystem = Cast<URsAbilityComponent>( GetAbilitySystemComponent() );
	if (IsValid(AbilitySystem))
	{
		if (IsValid(AttributeCoreStatsSet))
		{
			UCharacterRaceData* RaceData   = GetCharacterRaceData();
			UCharacterClassData* ClassData = GetCharacterClassData();
			TArray RaceValues  = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
			TArray ClassValues = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
			if (IsValid(RaceData))
			{
				RaceValues[0] += GetAttributeValue(RaceData->CoreStatsModifiers, AttributeCoreStatsSet->GetStrengthAttribute());
				RaceValues[1] += GetAttributeValue(RaceData->CoreStatsModifiers, AttributeCoreStatsSet->GetFortitudeAttribute());
				RaceValues[2] += GetAttributeValue(RaceData->CoreStatsModifiers, AttributeCoreStatsSet->GetDexterityAttribute());
				RaceValues[3] += GetAttributeValue(RaceData->CoreStatsModifiers, AttributeCoreStatsSet->GetAstutenessAttribute());
				RaceValues[4] += GetAttributeValue(RaceData->CoreStatsModifiers, AttributeCoreStatsSet->GetIntellectAttribute());
				RaceValues[5] += GetAttributeValue(RaceData->CoreStatsModifiers, AttributeCoreStatsSet->GetCharismaAttribute());
			}
			if (IsValid(ClassData))
			{
				RaceValues[0] += GetAttributeValue(ClassData->CoreStatsModifiers, AttributeCoreStatsSet->GetStrengthAttribute());
				RaceValues[1] += GetAttributeValue(ClassData->CoreStatsModifiers, AttributeCoreStatsSet->GetFortitudeAttribute());
				RaceValues[2] += GetAttributeValue(ClassData->CoreStatsModifiers, AttributeCoreStatsSet->GetDexterityAttribute());
				RaceValues[3] += GetAttributeValue(ClassData->CoreStatsModifiers, AttributeCoreStatsSet->GetAstutenessAttribute());
				RaceValues[4] += GetAttributeValue(ClassData->CoreStatsModifiers, AttributeCoreStatsSet->GetIntellectAttribute());
				RaceValues[5] += GetAttributeValue(ClassData->CoreStatsModifiers, AttributeCoreStatsSet->GetCharismaAttribute());
			}
			AbilitySystem->SetNumericAttributeBase(AttributeCoreStatsSet->GetStrengthAttribute(),   RaceValues[0] + ClassValues[0]);
			AbilitySystem->SetNumericAttributeBase(AttributeCoreStatsSet->GetFortitudeAttribute(),  RaceValues[1] + ClassValues[1]);
			AbilitySystem->SetNumericAttributeBase(AttributeCoreStatsSet->GetDexterityAttribute(),  RaceValues[2] + ClassValues[2]);
			AbilitySystem->SetNumericAttributeBase(AttributeCoreStatsSet->GetAstutenessAttribute(), RaceValues[3] + ClassValues[3]);
			AbilitySystem->SetNumericAttributeBase(AttributeCoreStatsSet->GetIntellectAttribute(),  RaceValues[4] + ClassValues[4]);
			AbilitySystem->SetNumericAttributeBase(AttributeCoreStatsSet->GetCharismaAttribute(),   RaceValues[5] + ClassValues[5]);
		}
	}
	UpdateVitalityStats();
}

void ACharacterBase::UpdateVitalityStats()
{
	if (!HasAuthority())
		return;

	URsAbilityComponent* AbilitySystem = Cast<URsAbilityComponent>( GetAbilitySystemComponent() );
	if (IsValid(AbilitySystem) && IsValid(AttributeVitalitySet) && IsValid(AttributeCoreStatsSet))
	{
		const float FortitudeModifier = AbilitySystem->GetCoreStatModifier(AttributeCoreStatsSet->GetFortitudeAttribute());
		const float IntellectModifier = AbilitySystem->GetCoreStatModifier(AttributeCoreStatsSet->GetIntellectAttribute());
		const float DexterityModifier = AbilitySystem->GetCoreStatModifier(AttributeCoreStatsSet->GetDexterityAttribute());

		const float NewHealthValue = ModifiedStatValue(FortitudeModifier);
		AbilitySystem->SetNumericAttributeBase(AttributeVitalitySet->GetMaximumHealthAttribute(), NewHealthValue);
		AbilitySystem->SetNumericAttributeBase(AttributeVitalitySet->GetCurrentHealthAttribute(), NewHealthValue);

		const float NewHealthRegenValue = 0.0125 * (1 + FortitudeModifier / 20.f);
		AbilitySystem->SetNumericAttributeBase(AttributeVitalitySet->GetPassiveHealthRegenAttribute(), NewHealthRegenValue);

		const float NewMagicValue = ModifiedStatValue(IntellectModifier);
		AbilitySystem->SetNumericAttributeBase(AttributeVitalitySet->GetMaximumMagicAttribute(), NewMagicValue);
		AbilitySystem->SetNumericAttributeBase(AttributeVitalitySet->GetCurrentMagicAttribute(), NewMagicValue);

		const float NewManaRegenValue = 0.0125 * (1 + IntellectModifier / 20.f);
		AbilitySystem->SetNumericAttributeBase(AttributeVitalitySet->GetPassiveMagicRegenAttribute(), NewManaRegenValue);

		const float NewStaminaValue = ModifiedStatValue(DexterityModifier);
		AbilitySystem->SetNumericAttributeBase(AttributeVitalitySet->GetMaximumStaminaAttribute(), NewStaminaValue);
		AbilitySystem->SetNumericAttributeBase(AttributeVitalitySet->GetCurrentStaminaAttribute(), NewStaminaValue);

		const float NewStaminaRegenValue = 0.035 * (1 + DexterityModifier / 20.f);
		AbilitySystem->SetNumericAttributeBase(AttributeVitalitySet->GetPassiveStaminaRegenAttribute(), NewStaminaRegenValue);

		const float NewHungerValue = ModifiedStatValue(FortitudeModifier);
		AbilitySystem->SetNumericAttributeBase(AttributeVitalitySet->GetMaximumHungerAttribute(), NewHungerValue);
		AbilitySystem->SetNumericAttributeBase(AttributeVitalitySet->GetCurrentHungerAttribute(), NewHungerValue);

		const float NewHungerDrainValue = FMath::Clamp(0.0000035 * (100 / FortitudeModifier), 0.000000001f, 0.01f);
		AbilitySystem->SetNumericAttributeBase(AttributeVitalitySet->GetPassiveHungerDrainAttribute(), NewHungerDrainValue);

		const float NewHydroValue = ModifiedStatValue(FortitudeModifier);
		AbilitySystem->SetNumericAttributeBase(AttributeVitalitySet->GetMaximumHydrationAttribute(), NewHydroValue);
		AbilitySystem->SetNumericAttributeBase(AttributeVitalitySet->GetCurrentHydrationAttribute(), NewHydroValue);

		const float NewHydroDrainValue = FMath::Clamp(0.000035  * (100 / FortitudeModifier), 0.000000001f, 0.01f);
		AbilitySystem->SetNumericAttributeBase(AttributeVitalitySet->GetPassiveHydroDrainAttribute(), NewHydroDrainValue);
	}
}

void ACharacterBase::Server_RestoreCharacter_Implementation(const FCharacterData& RestoreData)
{
	// If saves are server side, deny the restore request and find it ourselves
	if (bSavesOnServer)
	{
		if (OnCharacterRestored.IsBound()) { OnCharacterRestored.Broadcast(true); }
		Client_CharacterRestored(true);
	}
	// Allow restore if saves are client-side
	else
	{
		RestoreCharacter(RestoreData);
	}
}

void ACharacterBase::Client_CharacterRestored_Implementation(const bool bWasSuccess)
{
	bCharacterSaveRestored = bWasSuccess;
	UE_LOGFMT(LogTemp, Log, "{CharName}({Sv}) REPNOTIFY: Save Game {PassOrFail}",
		GetName(), HasAuthority()?"SV":"CL",
		bWasSuccess ? "Restored Successfully" : "Failed to Restore");
	if (OnCharacterRestored.IsBound()) { OnCharacterRestored.Broadcast(bWasSuccess); }
}

void ACharacterBase::BindListeners()
{
	// Ability System Delegates
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->OnDeathStatusChanged.AddDynamic(this, &ACharacterBase::OnDeathStatusChanged);

		if (IsValid(AttributeVitalitySet))
		{
			TArray VitalityAttributes = AttributeVitalitySet->GetAllVitalityAttributes();
			for (const FGameplayAttribute& vAttribute : VitalityAttributes)
			{
				AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
					vAttribute).AddUObject(this, &ACharacterBase::OnVitalityAttributeChanged);
				UE_LOGFMT(LogTemp, Display, "{CharacterName}({Sv}): "
					"Successfully initialized delegate for Vitality Attribute '{vAttribute}'",
					GetCharacterName(), HasAuthority()?"SRV":"CLI", vAttribute.AttributeName);
			}
		}

		if (IsValid(AttributeCoreStatsSet))
		{
			TArray CoreStatAttributes = AttributeCoreStatsSet->GetAllCoreStatAttributes();
			for (const FGameplayAttribute& CoreStat : CoreStatAttributes)
			{
				AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
					CoreStat).AddUObject(this, &ACharacterBase::OnCoreStatsChanged);
				UE_LOGFMT(LogTemp, Display, "{CharacterName}({Sv}): "
					"Successfully initialized delegate for Vitality Attribute '{vAttribute}'",
					GetCharacterName(), HasAuthority()?"SRV":"CLI", CoreStat.AttributeName);
			}
		}
	}
};

bool ACharacterBase::IsDead() const
{
	return IsValid(AbilitySystemComponent) ?
		AbilitySystemComponent->IsDead() : false;
}

/**
 * \brief Performs all respawn logic. When inheriting, ensure to call super or you will need
 * to implement all respawn logic, such as ability system resets.
 */
void ACharacterBase::Respawn(const ERespawnType RespawnType)
{
	if (!HasAuthority())
	{
		return;
	}

	// Resets health to 10% of maximum, triggering the ability system to set "IsDead" to false.
	if (IsValid(AbilitySystemComponent))
	{
		FGameplayAbilitySpec AbilitySpec = AbilitySystemComponent->BuildAbilitySpecFromClass(AbilityOnRevive);
		switch (RespawnType)
		{
		case ERespawnType::Graveyard:
			AbilitySpec = AbilitySystemComponent->BuildAbilitySpecFromClass(AbilityOnGraveyard);
			break;
		case ERespawnType::Entrance:
			AbilitySpec = AbilitySystemComponent->BuildAbilitySpecFromClass(AbilityOnRespawn);
			break;
		default:
			break;
		}

		AbilitySystemComponent->GiveAbilityAndActivateOnce(AbilitySpec);
		AttributeVitalitySet->SetCurrentHealth(AttributeVitalitySet->GetMaximumHealth() * 0.25);
		AbilitySystemComponent->SetDead(false);
	}

}

/**
 * \brief Sets the new name for this character. Typically used during creation/loading.
 * \param ProposedName The new name string to use for this character
 */
void ACharacterBase::SetCharacterName(FString ProposedName)
{
	// TODO - Add checks for symbols, special characters, etc
	const FString oldName = GetCharacterName();
	UE_LOGFMT(LogTemp, Log, "{CharName}({Sv}): Character Name Changed. {OldName} -> {NewName}",
		GetName(), HasAuthority()?"SV":"CL", oldName, ProposedName);
	CharacterName = ProposedName;
}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ACharacterBase::OnRep_CharacterName_Implementation(const FString& OldCharacterName)
{
	UE_LOGFMT(LogTemp, Log, "{CharName}({Sv}) Character Name Changed. {OldName} -> {NewName}",
		GetName(), HasAuthority()?"SV":"CL", OldCharacterName, GetCharacterName());
	if (OnCharacterNameChanged.IsBound()) { OnCharacterNameChanged.Broadcast(); }
}


//////////////////////////////////////////////////////////////////////////
// Replication
void ACharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACharacterBase, CharacterName);
	DOREPLIFETIME(ACharacterBase, SkinColor);

	DOREPLIFETIME(ACharacterBase, PronounObjective);
	DOREPLIFETIME(ACharacterBase, PronounPossessive);
	DOREPLIFETIME(ACharacterBase, PronounSubject);

	DOREPLIFETIME(ACharacterBase, CharacterRace_);
	DOREPLIFETIME(ACharacterBase, CharacterClass_);
	DOREPLIFETIME(ACharacterBase, FactionStandings);
	DOREPLIFETIME(ACharacterBase, FactionMemberships);
}
