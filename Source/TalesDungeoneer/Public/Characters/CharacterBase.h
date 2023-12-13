
#pragma once

#include "CoreMinimal.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"
#include "Delegates/Delegate.h"

#include "InventoryComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/MeshMergeComponent.h"

#include "TalesDungeoneer/lib/datastructures/GlobalData.h"
#include "TalesDungeoneer/Widgets/OverheadDataWidgetBase.h"
#include "TalesDungeoneer/Saves/SavedCharacters.h"

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

	virtual void DoAttack(EWeaponSlots WeaponSlot = EWeaponSlots::PRIMARY);

	UFUNCTION(BlueprintPure)
	ECharacterTeam GetCharacterTeam() const { return _CharacterTeam; }
	
	UFUNCTION(BlueprintCallable) void SetCharacterLevel(int NewLevel = 1);
	UFUNCTION(BlueprintPure) int GetCharacterLevel() const { return _CharacterLevel; }

	UFUNCTION(BlueprintPure) int GetExperienceWorth() const { return _ExperienceWorth; }

	UFUNCTION(BlueprintCallable)
	void SetCharacterClass(ECharacterClass NewClass);
	
	UFUNCTION(BlueprintCallable)
	void SetCharacterRace(ECharacterRace NewRace);
	
	UFUNCTION(BlueprintCallable)
	void SetFactionState(EFaction FactionEnum, EFactionState FactionState);

	UFUNCTION(BlueprintCallable)
	void SetFactionValue(EFaction FactionEnum, float PointsToSet = 0.f);
	
	UFUNCTION(BlueprintCallable)
	void IncreaseFaction(EFaction FactionEnum, float PointsToAdd = 1.f);

	UFUNCTION(BlueprintCallable)
	void DecreaseFaction(EFaction FactionEnum, float PointsToLose = 1.f);

	UFUNCTION(BlueprintCallable)
	void SetFactionMembership(EFaction FactionEnum, bool IsMember = true);

	UFUNCTION(BlueprintPure)
	ECharacterClass GetCharacterClass() const { return _CharacterClass; }

	UFUNCTION(BlueprintPure)
	ECharacterRace GetCharacterRace() const { return _CharacterRace; }

	UFUNCTION(BlueprintPure)
	TArray<EFaction> GetFactionMemberships() const { return _FactionMembership; }

	UFUNCTION(BlueprintPure)
	EFactionState GetFactionState(EFaction FactionToCheck) const;

	UFUNCTION(BlueprintPure)
	int GetRiskLevel() const { return _CharacterRisk; }

	UFUNCTION(BlueprintPure)
	float GetExperiencePoints() const { return _ExperiencePoints; }
	
	// Sets the current experience points pool to the given value. Does NOT run any logic.
	UFUNCTION(BlueprintCallable) void SetExperiencePoints(float NewValue = 0.f);

	// Adds experience points without accounting for level differences or scaling
	UFUNCTION(BlueprintCallable) void AddExperiencePoints(float AddValue = 0.f);

	// Removes experience points without accounting for level differences or scaling
	UFUNCTION(BlueprintCallable) void RemoveExperiencePoints(float AddValue = 0.f);

	// Gets the character's role-play-friendly name
	UFUNCTION(BlueprintPure) FString GetCharacterName() const { return CharacterName; }

	/**
	 * @brief Modifies the experience points given, accounting for level differences.
	 * @param AwardLevel The level of the risk (5 = Equal fight to a level 5)
	 * @param BasePoints The amount of unscaled experience points to award
	 */
	UFUNCTION(BlueprintCallable) void AwardExperiencePoints(int AwardLevel = 1, float BasePoints = 0.f);
	
	UFUNCTION(BlueprintPure)
	float GetExperienceNeeded() const
	{
		return _BaseExperience*pow(_ExpGrowthFactor, GetCharacterLevel()-1);
	}

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
			AController* EventInstigator, AActor* DamageCauser) override;
	
	/**
	 * @brief Sets the new name for this character. Typically used during creation/loading.
	 * @param ProposedName The new name string to use for this character
	 */
	UFUNCTION(BlueprintCallable) void SetCharacterName(FString ProposedName);

	virtual void LoadSaveData(const FString& SaveName,
		const int32 UserIndex, USaveGame* SaveData) {};

	virtual bool SaveCharacterData();
	virtual bool LoadCharacterData(const FString SaveSlotName, const int32 UserIndex);
	
protected: // functions
	
	UFUNCTION(BlueprintCallable) void SetCharacterTeam(ECharacterTeam NewTeam);

	UFUNCTION()	virtual void HotkeyTriggered(UInputAction* HotkeyAction);
	
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void CharacterRestoredFromSave(const FString SaveSlotName);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void Tick(float DeltaTime) override;

	// Called for movement input
	void Move(const FInputActionValue& Value);

	// Called for looking input
	void Look(const FInputActionValue& Value);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	
	// C++ Function ... 
	UFUNCTION(BlueprintImplementableEvent)
	void DoPrimaryAttack();
	
	// Blueprint override event. Contains the attack logic.
	UFUNCTION(BlueprintImplementableEvent)
	void DoSecondaryAttack();
	
	// Blueprint override event. Contains the attack logic.
	UFUNCTION(BlueprintImplementableEvent)
	void PrimaryAction();
	
	// Blueprint override event. Contains the attack logic.
	UFUNCTION(BlueprintImplementableEvent)
	void SecondaryAction();
	
private: // methods
	
	UFUNCTION(Client, Reliable)
	void Client_CharacterRestored(const FString& SaveSlotName);
	
	// Called when the equipment slot gets updated in InventoryComponent
	UFUNCTION()
	void UpdateWeapon(EEquipmentSlotType EquipmentEnum = EEquipmentSlotType::PRIMARY);

	UFUNCTION()
	void SpawnDamageText(AActor* DamageInstigator, float DamageTaken);

	// Called when the character starts an ability.
	// Checks if the ability should modify the combat state
	UFUNCTION()	void CheckAbilityStart(FName AbilityName, float CastTime);
	
	// Called when the character successfully activates an ability
	// Checks if the ability should modify the combat state
	UFUNCTION()	void CheckAbilitySuccess(FName AbilityName, bool WasSuccessful);

	UFUNCTION(NetMulticast, Reliable) void OnRep_CharacterName();
	
public: // members
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character Settings",
		ReplicatedUsing=OnRep_CharacterName)
	FString CharacterName = "Unnamed Character";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Settings")
	TSubclassOf<UOverheadDataWidgetBase> HeadDisplayWidget = nullptr;
	
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* JumpInputAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* MoveInputAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* LookInputAction;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* PrimaryAttackInputAction;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* SecondaryAttackInputAction;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* PrimaryInputAction;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "Character Input Settings")
	UInputAction* SecondaryInputAction;
	
	/** Character Inventory */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UInventoryComponent* InventoryComponent;

	/** Weapon Component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UWeaponComponent*	 WeaponComponent;

	/** Mesh Merge Component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UMeshMergeComponent*  MeshMergeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Settings") int UnlockPointsOnLevelUp = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character Settings")
	FName PronounSubject	= "he";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character Settings")
	FName PronounObjective	= "him";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character Settings")
	FName PronounPossessive = "his";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character Settings") 
	FSlateColor SkinColor;
	
private:

	UPROPERTY(Replicated)
	ECharacterTeam _CharacterTeam = ECharacterTeam::SPECTATOR;
	
	UFUNCTION(NetMulticast, Reliable) void OnRep_CharacterLevel(int OldLevel);
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_CharacterLevel) int _CharacterLevel = 1;

	UFUNCTION(Client, Reliable) void OnRep_ExperienceChanged(float OldExperience);
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_ExperienceChanged) float _ExperiencePoints = 0.f;
	
	UPROPERTY(Replicated) ECharacterClass _CharacterClass	= ECharacterClass::WARRIOR;
	UPROPERTY(Replicated) ECharacterRace _CharacterRace		= ECharacterRace::HUMAN;

	UPROPERTY(Replicated) TArray<EFaction> _FactionMembership = {};
	UPROPERTY(Replicated) FStFactionData _FactionData = FStFactionData();
	
	UPROPERTY(Replicated) int _CharacterRisk = 0;

	float _BaseExperience  = 1000.f;
	float _ExpGrowthFactor = 1.2;
	float _ExperienceWorth = 100.f;

	UPROPERTY(Replicated) bool _IsMale = true;

	// Sets true once the character save has been restored
	// Never 
	bool bCharacterSaveRestored = false;

	// If true, character saves should only save server-side
	// If false, character saves to the client who is controlling it
	bool bSavesOnServer = false;
};
