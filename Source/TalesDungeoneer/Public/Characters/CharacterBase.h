
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include "Delegates/Delegate.h"
#include "Gas/Abilities/TalesGameplayAbility.h"
#include "GameplayEffectTypes.h"

#include "InventoryComponent.h"
#include "Components/MeshMergeComponent.h"
#include "lib/Tags/TalesGlobalTags.h"

#include "Saves/SavedCharacters.h"

#include "CharacterBase.generated.h"


// Called anytime the characters name has been modified
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterNameChanged);

// Called when the character has been restored from a save file
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterRestored, const bool, bWasSuccess);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCharacterSaved,
	const FString&, SaveSlotName, const int, UserIndex, const bool, bWasSuccess);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeUpdated,
	const FGameplayAttribute&, AttributeData, const float, NewValue);
	
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeHealthUpdated,
	const float&, OldValue, const float&, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeStaminaUpdated,
	const float&, OldValue, const float&, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeMagicUpdated,
	const float&, OldValue, const float&, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeArmorUpdated,
	const float&, OldValue, const float&, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeHungerUpdated,
	const float&, OldValue, const float&, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAttributeHydrationUpdated,
	const float&, OldValue, const float&, NewValue);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterRaceChanged,
	const FGameplayTag&, OldRace, const FGameplayTag&, NewRace);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterClassChanged,
	const FGameplayTag&, OldRace, const FGameplayTag&, NewRace);

class UOverheadDataWidgetBase;


/**
 * Character Base is the base C++ class for all logic, methods and members that affect ALL
 * characters, regardless of NPC or Player and their race/class/stats.
 *
 * ALL CHARACTERS need controls, as the Dungeon Master will be able to possess them.
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ACharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	
public: // functions
	
	ACharacterBase();
	
	UPROPERTY(BlueprintAssignable) FOnCharacterSaved OnCharacterSaved;
	UPROPERTY(BlueprintAssignable) FOnCharacterRestored OnCharacterRestored;
	UPROPERTY(BlueprintAssignable) FOnCharacterNameChanged OnCharacterNameChanged;
	UPROPERTY(BlueprintAssignable) FOnAttributeUpdated OnAttributeUpdated;
	UPROPERTY(BlueprintAssignable) FOnAttributeHealthUpdated OnAttributeHealthUpdated;
	UPROPERTY(BlueprintAssignable) FOnAttributeStaminaUpdated OnAttributeStaminaUpdated;
	UPROPERTY(BlueprintAssignable) FOnAttributeMagicUpdated OnAttributeMagicUpdated;
	UPROPERTY(BlueprintAssignable) FOnAttributeArmorUpdated OnAttributeArmorUpdated;
	UPROPERTY(BlueprintAssignable) FOnAttributeHungerUpdated OnAttributeHungerUpdated;
	UPROPERTY(BlueprintAssignable) FOnAttributeHydrationUpdated OnAttributeHydrationUpdated;
	UPROPERTY(BlueprintAssignable) FOnCharacterRaceChanged OnCharacterRaceChanged;
	UPROPERTY(BlueprintAssignable) FOnCharacterClassChanged OnCharacterClassChanged;
	
	// Returns CameraBoom sub object
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	// Returns FollowCamera sub object
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintPure)
	float GetRiskLevel() const;

	// Gets the character's role-play-friendly name
	UFUNCTION(BlueprintPure) FString GetCharacterName() const { return CharacterName; }
	
	UFUNCTION(BlueprintCallable) void SetCharacterRace(const FGameplayTag& NewRaceTag);
	UFUNCTION(BlueprintCallable) void SetCharacterClass(const FGameplayTag& NewClassTag);
	
	UFUNCTION(BlueprintPure) FGameplayTag GetCharacterRace() const { return CharacterRace_; }
	UFUNCTION(BlueprintPure) FGameplayTag GetCharacterClass() const { return CharacterClass_; }

	// A safe character name is one with no spaces or special characters,
	// useful for things like save file names.
	UFUNCTION(BlueprintPure) FString GetCharacterSafeName() const;
		
	UFUNCTION(BlueprintPure) static int32 GetCharacterUserIndex() { return 0; }
	
	UFUNCTION(BlueprintCallable)
	void SetCharacterName(FString ProposedName);

	// Returns the AbilitySystemComponent so it can be made private
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual USaveGame* SaveCharacter(USaveGame* SaveObject = nullptr, bool bRunAsync = false);
	virtual bool LoadCharacter(const FString& SlotName = "", const int32 UserIndex = 0, USaveGame* SaveGame = nullptr);
	
protected: // functions
	
	virtual void BeginPlay() override;

	virtual void BindListeners();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void InitializeAbilities(); // Sets up default abilities
	
	virtual void InitializeEffects(); // Sets up default effects

	virtual void PossessedBy(AController* NewController) override;
	
	virtual void OnRep_PlayerState() override;

	virtual void CharacterRestoredFromSave(bool bWasSuccess = false);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void Tick(float DeltaTime) override;
	
	virtual void SaveGameDelegate(const FString& SlotName, const int32 UserIndex, bool bSaved);

	UFUNCTION()
	void InventoryUpdateDelegate(int SlotNumberUpdated);

	virtual void OnVitalityAttributeChanged(const FOnAttributeChangeData& Data);
	virtual void OnCoreStatsChanged(const FOnAttributeChangeData& Data);
	virtual void OnDamageStatsChanged(const FOnAttributeChangeData& Data);

	UFUNCTION(BlueprintImplementableEvent) void EventHealthChanged(float OldValue, float NewValue);
	UFUNCTION(BlueprintImplementableEvent) void EventStaminaChanged(float OldValue, float NewValue);
	UFUNCTION(BlueprintImplementableEvent) void EventMagicChanged(float OldValue, float NewValue);
	UFUNCTION(BlueprintImplementableEvent) void EventArmorChanged(float OldValue, float NewValue);
	UFUNCTION(BlueprintImplementableEvent) void EventHungerChanged(float OldValue, float NewValue);
	UFUNCTION(BlueprintImplementableEvent) void EventHydration(float OldValue, float NewValue);
	
private: // methods
	
	UFUNCTION(Client, Reliable)
	void Client_CharacterRestored(const bool bWasSuccess = false);

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_CharacterName(const FString& OldCharacterName);

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_CharacterRace(const FGameplayTag& OldRace);

	UFUNCTION(NetMulticast, Reliable)
	void OnRep_CharacterClass(const FGameplayTag& OldClass);

	UFUNCTION(Server, Reliable)
	void Server_RestoreCharacter(const FCharacterData& RestoreData);

	void RestoreCharacter(const FCharacterData& RestoreData);
	
public: // members

	// The human-friendly name of the character
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category = "Character Settings", ReplicatedUsing=OnRep_CharacterName)
	FString CharacterName = "";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		Category = "Data Initialization", meta = (AllowPrivateAccess = "true"))
	class UPrimaryCharacterData* CharacterData = nullptr;

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

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Gameplay Ability System", meta = (AllowPrivateAccess = "true"))
	class UVitalityAttributes* AttributeVitalitySet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Gameplay Ability System", meta = (AllowPrivateAccess = "true"))
	class UCoreStatsAttributes* AttributeCoreStatsSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
		Category = "Gameplay Ability System", meta = (AllowPrivateAccess = "true"))
	class UDamageAttributes* AttributeDamageSet;

	// An array of default abilities on spawn, set within blueprint
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Gameplay Ability System", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf <UTalesGameplayAbility> > DefaultAbilities;

	// An array of default effects on spawn, set within blueprint
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Gameplay Ability System", meta = (AllowPrivateAccess = "true"))
	TArray<TSubclassOf <UGameplayEffect> > DefaultEffects;
	
	// If true, character saves should only save server-side
	// If false, character saves to the client who is controlling it
	bool bSavesOnServer = false;

protected:
	
	// True when the inputs have been bound for the AbilitySystemComponent
	// False indicates the input for abilities has not initialized
	bool bIsInputBound = false;
	
private:
	
	UPROPERTY(ReplicatedUsing=OnRep_CharacterRace)
	FGameplayTag CharacterRace_	 = TAG_Character_Race_Human;
	
	UPROPERTY(ReplicatedUsing=OnRep_CharacterClass)
	FGameplayTag CharacterClass_ = TAG_Character_Class_Warrior;

	// Sets true once the character save has been restored. Never returns to false.
	bool bCharacterSaveRestored = false;

	bool bCharacterReady = false;
};
