// Copyright Take Five Games, LLC 2023 - All rights reserved


#pragma once

#include "CoreMinimal.h"
#include "CharacterBase.h"
#include "InputAction.h"
#include "InputActionValue.h"

#include "PlayerCharacterBase.generated.h"


class USavedCharacter;

// Called when this player has fully spawned into the world
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerJoined);

/**
 * Player Character Base is the base C++ class for all logic, methods and members that affect all
 * PLAYER based characters, prior to handling by child classes or dependent blueprint classes.
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API APlayerCharacterBase : public ACharacterBase
{
	GENERATED_BODY()

public: // functions

	UPROPERTY(VisibleAnywhere, BlueprintAssignable) FOnPlayerJoined OnPlayerJoined;
	
	APlayerCharacterBase();

	virtual bool SaveCharacterData() override;
	virtual void LoadCharacterData(
		const FString& SaveSlotName, const int32 UserIndex, USaveGame* SaveGame) override;
	
	UFUNCTION() void AwaitGameState();
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputMappingContext* DefaultMappingContext = nullptr;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* JumpInputAction = nullptr;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* MoveInputAction = nullptr;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* LookInputAction = nullptr;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* PrimaryAttackInputAction = nullptr;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* SecondaryAttackInputAction = nullptr;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* PrimaryInputAction = nullptr;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* SecondaryInputAction = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CharacterSaveFolder = "Characters/";
	
protected:

	UFUNCTION()
	virtual void HotkeyTriggered(UInputAction* HotkeyAction);

	// Called for movement input
	void Move(const FInputActionValue& Value);

	// Called for looking input
	void Look(const FInputActionValue& Value);
	
	virtual void BeginPlay() override;

	virtual void BindInput();

	// Used to reinitialize the character with client's data
	UFUNCTION(Server, Reliable)
	void Server_InitializeCharacter(const FString& NewName);

	UFUNCTION(Server, Reliable)
	void Server_SetupMeshMerge(
		const TArray<FStMeshMergeData>& MeshesToMerge,
		const TArray<FSkelMeshMergeSectionMapping>& MeshSectionMappings,
		const TArray<FSkelMeshMergeUVTransformMapping>& UvTransformsPerMesh);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
private:
	bool	bHasInitialized	= false;
	FString SaveSlotName_	= "";
	int32	SaveUserIndex_	= 0;
};
