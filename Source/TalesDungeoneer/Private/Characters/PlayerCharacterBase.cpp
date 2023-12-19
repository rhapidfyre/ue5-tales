// Copyright Take Five Games, LLC 2023 - All rights reserved


#include "Characters/PlayerCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Logging/StructuredLog.h"
#include "Gamemode/BaseFiles/TalesGameStateBase.h"
#include "lib/datastructures/GlobalData.h"
#include "Saves/SavedCharacters.h"
#include "TalesDungeoneer/TalesDungeoneer.h"


// Sets default values
APlayerCharacterBase::APlayerCharacterBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
}

/** Saves the character's components and internal data to file.
 * Runs SYNCHRONOUSLY. If you want this to run async, it needs to be called
 * by something that is running on the worker thread.
 */
bool APlayerCharacterBase::SaveCharacterData()
{
	USaveGame* SaveData;
	if (!UGameplayStatics::DoesSaveGameExist(CharacterSaveFolder + SaveSlotName_, SaveUserIndex_))
	{
		// If the character's name is set, then this is a new character being created
		// If it is not set, then the character is waiting to be loaded or created
		if (GetSafeCharacterName().IsEmpty())
		{
			UE_LOGFMT(LogTemp, Warning,
				"{CharacterName}({Sv}): Unable to create new character. Character has no name set.",
				GetName(), HasAuthority()?"SV":"CL");
			return false;	
		}
		
		
		SaveSlotName_ = GetSafeCharacterName();
		if (SaveSlotName_.IsEmpty())
		{
			UE_LOGFMT(LogTemp, Warning,
				"{CharacterName}({Sv}): Unable to save character. Character must be loaded first.",
				GetName(), HasAuthority()?"SV":"CL");
			return false;
		}
		
		SaveData = UGameplayStatics::CreateSaveGameObject( USavedCharacter::StaticClass() );
	}
	else
	{
		SaveData = UGameplayStatics::LoadGameFromSlot(CharacterSaveFolder + SaveSlotName_, SaveUserIndex_);
	}

	if (!IsValid(SaveData))
	{
		UE_LOGFMT(LogTemp, Warning,
			"{CharacterName}({Sv}): Unable to find save object '{SaveName} ({SaveIndex})'",
			GetName(), HasAuthority()?"SV":"CL", SaveSlotName_, SaveUserIndex_);
		return false;
	}

	// If the save is valid, collect all of the data we want to save
	USavedCharacter* CharacterSave = Cast<USavedCharacter>(SaveData);
	if (IsValid(CharacterSave))
	{
		// Set Character Persona Data
		CharacterSave->CharacterName  		= GetCharacterName();

		// Save Mesh Mesh Data
		CharacterSave->Skeleton            = MeshMergeComponent->Skeleton;
		CharacterSave->MeshSectionMappings = MeshMergeComponent->MeshSectionMappings;
		CharacterSave->UvTransformsPerMesh = MeshMergeComponent->UvTransformsPerMesh;
		CharacterSave->MeshesToMerge       = MeshMergeComponent->MeshesToMerge;

		// The inventory component saves internally.
		// We just need to remember the name of the save file to restore it.
		if (IsValid(InventoryComponent))
		{
			// If the inventory is SAVED before it is LOADED, it will create a new save file.
			// This should only happen on new characters.
			FString ResponseString = "";
			FString InvSaveName = InventoryComponent->SaveInventory(ResponseString, true);
			
			if (!InvSaveName.IsEmpty())
			{
				CharacterSave->SavedInventory = InvSaveName;
			}
			else
			{
				UE_LOGFMT(LogTemp, Error,
					"{CharacterName}({Sv}): Failed to Save Inventory. Reason: {ResponseStr}",
					GetName(), HasAuthority()?"SV":"CL", ResponseString);
			}
		}
		
		// Save the version of the game when this character was saved
		CharacterSave->SaveVersion = UGlobalData::GetAppVersion();

		// Performs the actual hard file save
		UE_LOGFMT(LogTemp, Log,
			"{CharacterName}({Sv}): Successfully saved character to '{SaveName}({Index})'",
			GetName(), HasAuthority()?"SV":"CL", SaveSlotName_, SaveUserIndex_);
		return UGameplayStatics::SaveGameToSlot(CharacterSave,
					CharacterSaveFolder + SaveSlotName_, SaveUserIndex_);
	}
	UE_LOGFMT(LogTemp, Warning,
		"{CharacterName}({Sv}): Save '{SaveName}({Index})' found, but it is not a character save",
		GetName(), HasAuthority()?"SV":"CL", SaveSlotName_, SaveUserIndex_);
	return false;
}

/** Loads an existing character and the characters components.
 * Runs SYNCHRONOUSLY, so if you want this to run Async, you need to call it
 * from an async request and pass the SaveGame data to this function.
 */
void APlayerCharacterBase::LoadCharacterData(
	const FString& SaveSlotName, const int32 UserIndex, USaveGame* SaveGame)
{
	Super::LoadCharacterData(SaveSlotName, UserIndex, SaveGame);
	const bool isSaveValid = IsValid(SaveGame);
	if (UGameplayStatics::DoesSaveGameExist(CharacterSaveFolder + SaveSlotName, UserIndex) || isSaveValid)
	{
		SaveSlotName_	= SaveSlotName;
		SaveUserIndex_	= UserIndex;
		USaveGame* SaveData = isSaveValid ? SaveGame :
			UGameplayStatics::LoadGameFromSlot(CharacterSaveFolder + SaveSlotName_, SaveUserIndex_);
		
		const USavedCharacter* CharacterData = Cast<USavedCharacter>( SaveData );
		if (IsValid(CharacterData))
		{
			Server_InitializeCharacter(CharacterData->CharacterName);
			
			// Restore Character Design
			Server_SetupMeshMerge(CharacterData->MeshesToMerge,
				CharacterData->MeshSectionMappings,
				CharacterData->UvTransformsPerMesh);
	
			if (IsValid(InventoryComponent))
			{
				FString ResponseString = "";
				if (!InventoryComponent->LoadInventory(
					ResponseString,	CharacterData->SavedInventory, true))
				{
					UE_LOGFMT(LogTemp, Warning,
						"{CharacterName}({Sv}): Failed to Restore Saved Inventory. Reason: {ResponseStr}",
						GetName(), HasAuthority()?"SV":"CL", ResponseString);
				}
			}
			
			UE_LOGFMT(LogTemp, Log, "{CharacterName}({sv}): Successfully restored character from Save Slot '{slot}'",
				GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *SaveSlotName_);
			CharacterRestoredFromSave(SaveSlotName);
		}
		else
		{
			UE_LOGFMT(LogTemp, Warning,
				"{CharacterName}({Sv}): Save '{SaveName}({Index})' exists, but it is not a character save.",
				GetName(), HasAuthority()?"SV":"CL", SaveSlotName_, SaveUserIndex_);
		}
		
	}
	else
	{
		UE_LOGFMT(LogTemp, Warning,
			"{CharacterName}({Sv}): No character save data found. '{SaveName}({Index})",
			GetName(), HasAuthority()?"SV":"CL", SaveSlotName, UserIndex);
	}
}

void APlayerCharacterBase::AwaitGameState()
{
	const AGameStateBase* GameStateBase = GetWorld()->GetGameState();
	const ATalesGameStateBase* TalesGameState = Cast<ATalesGameStateBase>(GameStateBase);
	
	if (IsValid(TalesGameState))
	{
		while (!TalesGameState->GetIsSaveMetaReady())
		{
			
		}
		const FString SaveSlotName = TalesGameState->GetSelectedCharacterSaveSlotName();
		if (UGameplayStatics::DoesSaveGameExist(CharacterSaveFolder + SaveSlotName, 0))
		{
			USavedCharacter* SavedCharacter = Cast<USavedCharacter>(
				UGameplayStatics::LoadGameFromSlot(CharacterSaveFolder + SaveSlotName, 0));
			if (IsValid(SavedCharacter))
			{
				UE_LOGFMT(LogTemp, Display,
					"{CharacterName}({Sv}): (Async Response) Restoring character data from save '{SaveName} ({Index})'",
					GetName(), HasAuthority()?"SV":"CL", SaveSlotName_, SaveUserIndex_);
				LoadCharacterData(SaveSlotName_, SaveUserIndex_, nullptr);
			}
			else
			{
				UE_LOGFMT(LogTemp, Error,
					"{CharacterName}({Sv}): (Async Response) The found save data was not a character save.",
					GetName(), HasAuthority()?"SV":"CL");
			}
		}
		else
		{
			UE_LOGFMT(LogTemp, Warning,
				"{CharacterName}({Sv}): (Async Response) No character save data found.",
				GetName(), HasAuthority()?"SV":"CL");
		}
	}
	
	// Re-fire if game state isn't valid
	// This shouldn't happen but it stops any potential initialization issues
	else
	{
		FTimerHandle TimerReference;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUObject(this, &APlayerCharacterBase::AwaitGameState);
		GetWorld()->GetTimerManager().SetTimer(TimerReference,	TimerDelegate,
			1, false);
	}
	
}

void APlayerCharacterBase::HotkeyTriggered(UInputAction* HotkeyAction)
{
	UE_LOGFMT(LogTemp, Display,
		"{CharacterName}({Sv}): Hotkey '{KeyName}' Triggered",
		GetName(), HasAuthority()?"SV":"CL", HotkeyAction->GetName());
}

void APlayerCharacterBase::BeginPlay()
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
	
	if ((HasAuthority() && !bSavesOnServer) || (!HasAuthority() && bSavesOnServer))
	{
		const ENetMode netMode = GetNetMode();
		if (netMode != NM_ListenServer && netMode != NM_Standalone)
		{
			UE_LOGFMT(LogTemp, Display,
				"{CharacterName}({Sv}): Character restoration criteria not met. "
				"({SaveType})", GetName(), HasAuthority()?"SV":"CL",
				bSavesOnServer ? "Saves Serverside" : "Saves on Client");
			return;
		}
	}
	
	const AGameStateBase* GameStateBase = GetWorld()->GetGameState();
	const ATalesGameStateBase* TalesGameState = Cast<ATalesGameStateBase>(GameStateBase);
	if (IsValid(TalesGameState))
	{
		// Run async initialization of game state to reload character data
		UE_LOGFMT(LogTemp, Display,
			"{CharacterName}({Sv}): Requesting Async restoration of character data.",
			GetName(), HasAuthority()?"SV":"CL");

		SaveSlotName_	= TalesGameState->GetSelectedCharacterSaveSlotName();
		SaveUserIndex_	= 0;
		
		FTimerHandle TimerReference;
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindUObject(this, &APlayerCharacterBase::AwaitGameState);
		GetWorld()->GetTimerManager().SetTimer(TimerReference, TimerDelegate,
			1, false);
	}
	else
	{
		UE_LOGFMT(LogTemp, Error,
			"{CharacterName}({Sv}): Unable to restore character save data. "
			"GameState is not of type ATalesGameStateBase*",
			GetName(), HasAuthority()?"SV":"CL");
	}
	
	OnPlayerJoined.Broadcast();
}

void APlayerCharacterBase::BindInput()
{
	if (bIsInputBound || !IsValid(AbilitySystemComponent) || !IsValid(InputComponent))
	{
		return;
	}

	FTopLevelAssetPath EnumAssetPath = FTopLevelAssetPath(
		FName("/Script/TalesDungeoneer"), FName("EAbilityInputID"));
	
	GetAbilitySystemComponent()->BindAbilityActivationToInputComponent(InputComponent,
		FGameplayAbilityInputBinds(
			FString("Confirm"),
			FString("Cancel"),
			EnumAssetPath,
		static_cast<int32>(EAbilityInputID::Confirm),
		static_cast<int32>(EAbilityInputID::Cancel)));
	bIsInputBound = true;
}

void APlayerCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Jumping
		EnhancedInputComponent->BindAction(JumpInputAction,
			ETriggerEvent::Triggered, this, &APlayerCharacterBase::Jump);
		
		EnhancedInputComponent->BindAction(JumpInputAction,
			ETriggerEvent::Completed, this, &APlayerCharacterBase::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveInputAction,
		ETriggerEvent::Triggered, this, &APlayerCharacterBase::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookInputAction,
		ETriggerEvent::Triggered, this, &APlayerCharacterBase::Look);

	}
	
	// Call the function that handles ability system bindings
	BindInput();
}

void APlayerCharacterBase::Move(const FInputActionValue& Value)
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

void APlayerCharacterBase::Look(const FInputActionValue& Value)
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

void APlayerCharacterBase::Server_SetupMeshMerge_Implementation(
	const TArray<FStMeshMergeData>& MeshesToMerge,
	const TArray<FSkelMeshMergeSectionMapping>& MeshSectionMappings,
	const TArray<FSkelMeshMergeUVTransformMapping>& UvTransformsPerMesh)
{
	if (IsValid(MeshMergeComponent))
	{
		MeshMergeComponent->MeshesToMerge		= MeshesToMerge;
		MeshMergeComponent->MeshSectionMappings = MeshSectionMappings;
		MeshMergeComponent->UvTransformsPerMesh = UvTransformsPerMesh;
		MeshMergeComponent->PerformMeshMerge();
	}
}

void APlayerCharacterBase::Server_InitializeCharacter_Implementation(
	const FString& NewName)
{
	if (bHasInitialized)
	{
		UE_LOGFMT(LogTemp, Warning,
			"{CharacterName}({Sv}): Server_InitializeCharacter() Failed - "
			"Character has already been restored from save this session.",
			GetName(), HasAuthority()?"SV":"CL");
		return;
	}
	
	if (HasAuthority())
	{
		UE_LOGFMT(LogTemp, Display,
			"{CharacterName}({Sv}): Server_InitializeCharacter executed Successfully",
			GetName(), HasAuthority()?"SV":"CL");
		SetCharacterName(NewName);
		bHasInitialized = true; // Prevents the clients from injecting
	}
}
	