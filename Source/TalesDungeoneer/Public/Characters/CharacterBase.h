
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Delegates/Delegate.h"

#include "InventoryComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/MeshMergeComponent.h"

#include "lib/datastructures/GlobalData.h"
#include "Saves/SavedCharacters.h"

#include "CharacterBase.generated.h"


// Called when primary action is triggered
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPrimaryAction);

// Called when secondary action is triggered
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSecondaryAction);

// Called when primary attack is triggered
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPrimaryAttack);

// Called when secondary attack is triggered
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSecondaryAttack);

// Called when the character gains a level by an increase in experience
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterLevelUp, int, NewLevel);

// Called anytime the characters experience has been modified, up or down
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExperienceChanged);

// Called anytime the characters name has been modified
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterNameChanged);

// Called anytime the characters level has been modified, up or down
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterLevelChanged);

// Called when the character has been restored from a save file
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterRestored, FString, SaveSlotName);


class UOverheadDataWidgetBase;


/**
 * Character Base is the base C++ class for all logic, methods and members that affect ALL
 * characters, regardless of NPC or Player and their race/class/stats.
 *
 * ALL CHARACTERS need controls, as the Dungeon Master will be able to possess them.
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ACharacterBase : public ACharacter
{
	GENERATED_BODY()

	
public: // functions
	
	ACharacterBase();
	
	UPROPERTY(BlueprintAssignable) // Primary Action Called.. Pre-logic
	FOnPrimaryAction OnPrimaryAction;
	
	UPROPERTY(BlueprintAssignable) // After successful primary attack
	FOnPrimaryAttack OnPrimaryAttack;
	
	UPROPERTY(BlueprintAssignable) // Secondary Action Called.. Pre-Logic
	FOnSecondaryAction OnSecondaryAction;
	
	UPROPERTY(BlueprintAssignable) // After successful secondary attack
	FOnSecondaryAttack 	OnSecondaryAttack;
	
	UPROPERTY(BlueprintAssignable) // After level-up logic has completed
	FOnCharacterLevelUp	OnCharacterLevelUp;
	
	UPROPERTY(BlueprintAssignable) // After experience points have changed
	FOnExperienceChanged OnExperienceChanged;
	
	UPROPERTY(BlueprintAssignable) // After level-up/down logic has finished
	FOnCharacterLevelChanged OnCharacterLevelChanged;
	
	UPROPERTY(BlueprintAssignable) // After character restored from save logic
	FOnCharacterRestored OnCharacterRestored;
	
	UPROPERTY(BlueprintAssignable) // After the characters display name changes
	FOnCharacterNameChanged	OnCharacterNameChanged;
	
	// Returns CameraBoom sub object
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	// Returns FollowCamera sub object
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintPure)
	float GetRiskLevel() const;

	// Gets the character's role-play-friendly name
	UFUNCTION(BlueprintPure)
	FString GetCharacterName() const { return CharacterName; }

	UFUNCTION(BlueprintPure)
	FString GetSafeCharacterName() const
	{ return CharacterName.Replace(TEXT(" "), TEXT(""), ESearchCase::IgnoreCase); }	
	
	UFUNCTION(BlueprintCallable)
	void SetCharacterName(FString ProposedName);

	virtual bool SaveCharacterData();
	
	virtual void LoadCharacterData(
		const FString& SaveSlotName, const int32 UserIndex, USaveGame* SaveGame);
	
protected: // functions
	
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void CharacterRestoredFromSave(const FString SaveSlotName);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void InventoryUpdateDelegate(int SlotNumberUpdated, bool bIsEquipment);
	
	// C++ Function for performing a Primary Attack action
	virtual bool PrimaryAction();
	
	// C++ Function for performing a Secondary Attack action
	virtual bool SecondaryAction();
	
	// Blueprint event called by DoPrimaryAttack after Primary Attack was triggered
	UFUNCTION(BlueprintImplementableEvent)
	void DoPrimaryAttack();
	
	// Blueprint event called by DoSecondaryAttack after Primary Attack was triggered
	UFUNCTION(BlueprintImplementableEvent)
	void DoSecondaryAttack();
	
private: // methods
	
	UFUNCTION(Client, Reliable)
	void Client_CharacterRestored(const FString& SaveSlotName);

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_CharacterName(const FString& OldCharacterName);
	
public: // members

	// The human-friendly name of the character
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category = "Character Settings", ReplicatedUsing=OnRep_CharacterName)
	FString CharacterName = "Unnamed";

	// The widget to display over the characters head
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Settings")
	TSubclassOf<UOverheadDataWidgetBase> HeadDisplayWidget = nullptr;
	
	// Camera boom positioning the camera behind the character
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	// Follow camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	// Character Inventory
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UInventoryComponent* InventoryComponent;

	// Collects fragmented body/equipment parts and merges them into one mesh
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMeshMergeComponent*  MeshMergeComponent;

	// TODO - Deprecate this - Move it to GAS
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Settings")
	int UnlockPointsOnLevelUp = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character Settings")
	FName PronounSubject	= "he";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character Settings")
	FName PronounObjective	= "him";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character Settings")
	FName PronounPossessive = "his";
	
	// TODO - Deprecate this & move it to Mesh Merge Component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character Settings") 
	FSlateColor SkinColor;

	// If true, character saves should only save server-side
	// If false, character saves to the client who is controlling it
	bool bSavesOnServer = false;
	
private:

	// Sets true once the character save has been restored. Never returns to false.
	bool bCharacterSaveRestored = false;
};
