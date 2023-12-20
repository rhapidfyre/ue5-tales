// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterBase.h"

#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "lib/datastructures/GlobalData.h"
#include "Saves/SavedCharacters.h"
#include "Gamemode/BaseFiles/TalesGameStateBase.h"
#include "Gas/Abilities/TalesGameplayAbility.h"
#include "Gas/AttributeSets/TalesAttributes.h"
#include "Kismet/GameplayStatics.h"

#include "Logging/StructuredLog.h"

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

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeVitalitySet	= CreateDefaultSubobject<UVitalityAttributes>("AttributeVitalitySet");
	AttributeCoreStatsSet	= CreateDefaultSubobject<UCoreStatsAttributes>("AttributeCoreStatsSet");
	AttributeDamageSet		= CreateDefaultSubobject<UDamageAttributes>("AttributeDamageSet");

	// Setup listeners for when the inventory changes
	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	if (!InventoryComponent->OnInventoryUpdated.IsAlreadyBound(this, &ACharacterBase::InventoryUpdateDelegate))
	{
		InventoryComponent->OnInventoryUpdated.AddDynamic(this, &ACharacterBase::InventoryUpdateDelegate);
	}

	MeshMergeComponent = CreateDefaultSubobject<UMeshMergeComponent>(TEXT("MeshMergeComponent"));

	// Allow weapon overlap collisions
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_GameTraceChannel4, ECR_Block);
	
}

/**
 *  Calculates difficulty of this character if fought by the player characters.
 *  Takes into account other characters part of this characters group.
 * @return A percentage - The likelihood of success for this encounter
 */
float ACharacterBase::GetRiskLevel() const
{
	return 1.f;
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
 * @param SaveObject The save data being carried between child/parent
 * @param bRunAsync True if the save should run asynchronously
 * @return The USavedCharacter SaveGame object for this character.
 */
USaveGame* ACharacterBase::SaveCharacter(USaveGame* SaveObject, bool bRunAsync)
{
	if (!bCharacterReady)
	{
		UE_LOGFMT(LogCharacterBase, Warning,
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
			UE_LOGFMT(LogCharacterBase, Error,
				"{Name}({Sv}): Failed to Save Character - GameState "
				"is not a Tales-type GameState Object", GetName(), HasAuthority()?"SRV":"CLI");
			return nullptr;
		}

		// We can only create a new save if we're in the character creator
		if (!TalesGameState->GetIsCreatingCharacter())
		{
			UE_LOGFMT(LogCharacterBase, Log,
				"{Name}({Sv}): Cannot create new character save - "
				"Character Creator not in use.", GetName(), HasAuthority()?"SRV":"CLI");
			return nullptr;
		}

		SavedCharacter = Cast<USavedCharacter>
			( UGameplayStatics::CreateSaveGameObject(USavedCharacter::StaticClass()) );
		
		SavedCharacter->SaveSlotName = TalesGameState->
			GenerateAlphanumeric(UGlobalData::CharacterSaveFolder());
		
		SavedCharacter->UserIndex = GetCharacterUserIndex();
	}

	// Write ACharacterBase* specific data to SaveObject ( SavedCharacter )
	
	SavedCharacter->SaveVersion			= UGlobalData::GetAppVersion();
	
	SavedCharacter->CharacterData.CharacterName = GetCharacterName();
	
	SavedCharacter->Skeleton			= MeshMergeComponent->Skeleton;
	SavedCharacter->MeshesToMerge		= MeshMergeComponent->MeshesToMerge;
	SavedCharacter->MeshSectionMappings = MeshMergeComponent->MeshSectionMappings;
	SavedCharacter->UvTransformsPerMesh = MeshMergeComponent->UvTransformsPerMesh;

	// If the inventory save does not exist, this is a new inventory
	FString InventoryResponse = "";
	if (InventoryComponent->GetInventorySaveName().IsEmpty())
	{
		// Issue starting items and save
		UE_LOGFMT(LogCharacterBase, Display, "{Character}({Sv}): Attempting to issue "
			"starting items and create new inventory save...", GetName(), HasAuthority()?"SRV":"CLI");
		
		InventoryComponent->IssueStartingItems();
		SavedCharacter->SavedInventory = InventoryComponent->SaveInventory(InventoryResponse, false);
	}
	
	// If the inventory does exist, save it asynchronously
	else
	{
		InventoryComponent->SaveInventory(InventoryResponse, true);
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
	OnCharacterSaved.Broadcast(SavedCharacter->SaveSlotName, SavedCharacter->UserIndex, bSaved);
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
 * @param SlotName The save name to be restored. Only set in async call. Ignored if called manually.
 * @param UserIndex The user index for the character. Only set in async call. Ignored if called manually.
 * @param SaveGame When used as an async load delegate, this is the save object loaded.
 */
bool ACharacterBase::LoadCharacter(const FString& SlotName, const int32 UserIndex, USaveGame* SaveGame)
{
	bool bWasSuccess = false;
	if (!bCharacterReady)
	{
		UE_LOGFMT(LogCharacterBase, Error, "{Character}({Sv}): "
			"Cannot load character until initialization has completed.",
			GetName(), HasAuthority()?"SRV":"CLI");
		OnCharacterRestored.Broadcast(false);
		return false;
	}

	// If the save data is not valid, attempt to find it
	// This will also run if LoadCharacter is called synchronously
	USavedCharacter* SavedCharacter = Cast<USavedCharacter>( SaveGame );
	if (!IsValid(SavedCharacter))
	{
		const ATalesGameStateBase* TalesGameState = Cast<ATalesGameStateBase>
				( GetWorld()->GetGameState() );

		// If the game state is valid, we can attempt to get the selected character
		if (IsValid(TalesGameState))
		{
			// No saved data and creator open means this is a new character
			if (TalesGameState->GetIsCreatingCharacter())
			{
				UE_LOGFMT(LogCharacterBase, Warning, "{Character}({Sv}): "
					"Cannot Load Character while Character Creator is open.",
					GetName(), HasAuthority()?"SRV":"CLI");
			}
			else
			{
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
					UE_LOGFMT(LogCharacterBase, Warning, "{Character}({Sv}): "
						"Character Save '{SlotName} ({UserIndex})' does not exist, "
						"or Player is in the Character Creator.",
						GetName(), HasAuthority()?"SRV":"CLI",
						TalesGameState->GetCharacterSlotName(),
						TalesGameState->GetCharacterUserIndex());
				}
			}
		}
		// Game State is not valid or ready
		else
		{
			UE_LOGFMT(LogCharacterBase, Error, "{Character}({Sv}): "
				"Cannot load character - TalesGameState is not ready, or invalid.",
				GetName(), HasAuthority()?"SRV":"CLI");
		}
	}

	// Perform load from data, if the save object now exists
	if (IsValid(SavedCharacter))
	{
		// Send Character Data to server for restoration
		Server_RestoreCharacter(SavedCharacter->CharacterData);

		FString InventoryResponse = "";
		InventoryComponent->LoadInventory(InventoryResponse,
			SavedCharacter->SaveSlotName, true);
		
		MeshMergeComponent->InitializeMeshMerge(SavedCharacter);
		
		bWasSuccess = true;
	}
	
	OnCharacterRestored.Broadcast(bWasSuccess);
	return bWasSuccess;
}


// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
	BindListeners();
	bCharacterReady = true;
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

void ACharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitializeAbilities();
	InitializeEffects();
}

void ACharacterBase::InitializeAbilities()
{
	// Only run on server
	if (!HasAuthority() || !AbilitySystemComponent)
	{
		return;
	}

	for (TSubclassOf<UTalesGameplayAbility>& Ability : DefaultAbilities)
	{
		AbilitySystemComponent->GiveAbility(
			FGameplayAbilitySpec(Ability, 1,
				static_cast<int32>(Ability.GetDefaultObject()->AbilityInputID), this));
	}
}

void ACharacterBase::InitializeEffects()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);

	for (const TSubclassOf<UGameplayEffect>& DefaultEffect : DefaultEffects)
	{
		FGameplayEffectSpecHandle SpecHandle =
			AbilitySystemComponent->MakeOutgoingSpec(DefaultEffect, 1, EffectContext);
		
		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle GEHandle =
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}
	}
}

void ACharacterBase::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	if (!IsValid(AbilitySystemComponent))
	{
		return;
	}
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	InitializeEffects();
}

void ACharacterBase::CharacterRestoredFromSave(const bool bWasSuccess)
{
	bCharacterSaveRestored = bWasSuccess;
	OnCharacterRestored.Broadcast(bWasSuccess);
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

void ACharacterBase::InventoryUpdateDelegate(int SlotNumberUpdated, bool bIsEquipment)
{
	UE_LOGFMT(LogTemp, Log, "{CharName}({Sv}): {SlotType} Update Received (Slot #{SlotNum})",
		GetName(), HasAuthority()?"SV":"CL", bIsEquipment?"Equipment":"Inventory", SlotNumberUpdated);
	if (bIsEquipment)
	{
		
	}
	else
	{
		
	}
}

void ACharacterBase::OnVitalityAttributeChanged(const FOnAttributeChangeData& Data)
{
	OnAttributeUpdated.Broadcast(Data.Attribute, Data.NewValue);
	if (Data.Attribute == AttributeVitalitySet->GetCurrentHealthAttribute())
	{
		OnAttributeHealthUpdated.Broadcast(Data.OldValue, Data.NewValue);
		EventHealthChanged(Data.OldValue, Data.NewValue);
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
	else if (Data.Attribute == AttributeVitalitySet->GetCurrentArmorAttribute())
	{
		OnAttributeArmorUpdated.Broadcast(Data.OldValue, Data.NewValue);
		EventArmorChanged(Data.OldValue, Data.NewValue);
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

void ACharacterBase::RestoreCharacter(const FCharacterData& RestoreData)
{
	if (bCharacterSaveRestored)
	{
		UE_LOGFMT(LogTemp, Warning, "{CharName}({Sv}): Restore Denied - "
			"Character Already Restored.", GetName(), HasAuthority()?"SV":"CL");
		return;	
	}
	
	// If we are the authority, authorize the restoration
	if (HasAuthority())
	{
		SetCharacterName(RestoreData.CharacterName);

		// Update restored flags to prevent cheating
		bCharacterSaveRestored = true;
		OnCharacterRestored.Broadcast(true);
		Client_CharacterRestored(true);
	}
	
	// If we are not the authority, send the data to the server to be handled
	else
	{
		Server_RestoreCharacter(RestoreData);
	}
}

void ACharacterBase::Server_RestoreCharacter_Implementation(const FCharacterData& RestoreData)
{
	if (!bCharacterSaveRestored)
	{
		// If saves are server side, deny the restore request and find it ourselves
		if (bSavesOnServer)
		{
			bCharacterSaveRestored = true;
			OnCharacterRestored.Broadcast(true);
			Client_CharacterRestored(true);
		}
		// Allow restore if saves are client-side
		else
		{
			RestoreCharacter(RestoreData);
		}
	}
}

void ACharacterBase::Client_CharacterRestored_Implementation(const bool bWasSuccess)
{
	bCharacterSaveRestored = bWasSuccess;
	UE_LOGFMT(LogTemp, Log, "{CharName}({Sv}) REPNOTIFY: Save Game {PassOrFail}",
		GetName(), HasAuthority()?"SV":"CL",
		bWasSuccess ? "Failed to Restore" : "Restored Successfully");
	OnCharacterRestored.Broadcast(bWasSuccess);
}

void ACharacterBase::BindListeners()
{
	// Everyone should always listen to all character's vitality stats
	//	so we run this on the CharacterBase on all clients
	TArray VitalityAttributes = {
		AttributeVitalitySet->GetCurrentHealthAttribute(),
		AttributeVitalitySet->GetCurrentArmorAttribute(),
		AttributeVitalitySet->GetCurrentStaminaAttribute(),
		AttributeVitalitySet->GetCurrentMagicAttribute(),
		AttributeVitalitySet->GetCurrentHungerAttribute(),
		AttributeVitalitySet->GetCurrentHydrationAttribute()
	};

	// Bind listeners to vitality stat changes
	for (const FGameplayAttribute& vAttribute : VitalityAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
			vAttribute).AddUObject(this, &ACharacterBase::OnVitalityAttributeChanged);
		UE_LOGFMT(LogTemp, Display, "{CharacterName}({Sv}): "
			"Successfully initialized delegate for Vitality Attribute '{vAttribute}'",
			GetCharacterName(), HasAuthority()?"SRV":"CLI", vAttribute.AttributeName);
	}
	
};

/**
 * @brief Sets the new name for this character. Typically used during creation/loading.
 * @param ProposedName The new name string to use for this character
 */
void ACharacterBase::SetCharacterName(FString ProposedName)
{
	// TODO - Add checks for symbols, special characters, etc
	UE_LOGFMT(LogTemp, Log, "{CharName}({Sv}): Character Name Changed. {OldName} -> {NewName}",
		GetName(), HasAuthority()?"SV":"CL", GetCharacterName(), ProposedName);
	CharacterName = ProposedName;
}

UAbilitySystemComponent* ACharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ACharacterBase::OnRep_CharacterName_Implementation(const FString& OldCharacterName)
{
	UE_LOGFMT(LogTemp, Log, "{CharName}({Sv}) REPNOTIFY: Character Name Changed. {OldName} -> {NewName}",
		GetName(), HasAuthority()?"SV":"CL", OldCharacterName, GetCharacterName());
	OnCharacterNameChanged.Broadcast();
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
}
