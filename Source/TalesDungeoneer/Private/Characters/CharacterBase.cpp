// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterBase.h"

#include "Net/UnrealNetwork.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "lib/datastructures/GlobalData.h"
#include "Saves/SavedCharacters.h"
#include "Gamemode/BaseFiles/TalesGameStateBase.h"
#include "lib/enums/GlobalEnums.h"
#include "Weapons/WeaponSystem.h"
#include "Logging/StructuredLog.h"
#include "Widgets/OverheadDataWidgetBase.h"

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

bool ACharacterBase::SaveCharacterData()
{
	return true;
}

void ACharacterBase::LoadCharacterData(
	const FString& SaveSlotName, const int32 UserIndex, USaveGame* SaveGame)
{
}


// Called when the game starts or when spawned
void ACharacterBase::BeginPlay()
{
	Super::BeginPlay();
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

/**
 * C++ Function for performing a Primary Attack action
 * Needs to be overridden by child classes or it will always return true
 * @return True if attack criteria was met successfully
 */
bool ACharacterBase::PrimaryAction()
{
	return true;
}

/**
 * C++ Function for performing a Secondary Attack action
 * Needs to be overridden by child classes or it will always return true
 * @return True if attack criteria was met successfully
 */
bool ACharacterBase::SecondaryAction()
{
	return true;
}

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

void ACharacterBase::Client_CharacterRestored_Implementation(const FString& SaveSlotName)
{
	bCharacterSaveRestored = true;
	UE_LOGFMT(LogTemp, Log, "{CharName}({Sv}) REPNOTIFY: Save Game '{SaveName}' Loaded Successfully!",
		GetName(), HasAuthority()?"SV":"CL", SaveSlotName);
	OnCharacterRestored.Broadcast(SaveSlotName);
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
