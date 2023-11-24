
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "InventoryComponent.h"

#include "VitalityStatComponent.h"
#include "VitalityEffectsComponent.h"
#include "VitalityWelfareComponent.h"

#include "Components/WeaponComponent.h"
#include "Components/AbilityComponent.h"
#include "Components/MeshMergeComponent.h"
#include "TalesDungeoneer/Entities/SimpleActors/FloatingTextBase.h"

#include "TalesDungeoneer/lib/datastructures/GlobalData.h"
#include "TalesDungeoneer/Widgets/OverheadDataWidgetBase.h"
#include "TalesDungeoneer/Saves/SavedCharacters.h"

#include "CharacterBase.generated.h"


class AFloatingTextBase;
class UFloatingTextWidgetBase;

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

	UPROPERTY(BlueprintAssignable) FOnPrimaryAction OnPrimaryAction;
	UPROPERTY(BlueprintAssignable) FOnPrimaryAttack OnPrimaryAttack;
	UPROPERTY(BlueprintAssignable) FOnSecondaryAction OnSecondaryAction;
	UPROPERTY(BlueprintAssignable) FOnSecondaryAttack OnSecondaryAttack;
	UPROPERTY(BlueprintAssignable) FOnCharacterLevelUp OnCharacterLevelUp;
	UPROPERTY(BlueprintAssignable) FOnExperienceChanged OnExperienceChanged;
	UPROPERTY(BlueprintAssignable) FOnCharacterLevelChanged OnCharacterLevelChanged;
	UPROPERTY(BlueprintAssignable) FOnCharacterRestored OnCharacterRestored;
	UPROPERTY(BlueprintAssignable) FOnCharacterNameChanged OnCharacterNameChanged;
	
	/** Returns CameraBoom sub object **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera sub object **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintPure)
	EFactionState GetFactionConsideration(ACharacterBase* TargetActor, float& FactionValue);
	
	UFUNCTION(BlueprintCallable)
	void ToggleWeapon(EWeaponSlots WeaponSlot = EWeaponSlots::PRIMARY,
						bool ForceDraw = false, bool ForceStow = false);

	/** Called to start an attack.
	 * Once started, the attack will continue until finished.
	 * @param WeaponSlot The weapon slot to attack with (defaults to Primary)
	 */
	virtual void PerformAttack(
		EWeaponSlots WeaponSlot = EWeaponSlots::PRIMARY);
	
	// Called when the UWeaponComponent should start blocking
	// Does nothing if a shield/torch/etc is not equipped
	UFUNCTION(BlueprintCallable) void StartBlocking();
	
	// Called when blocking should stop
	// Does nothing if the character isn't blocking
	UFUNCTION(BlueprintCallable) void StopBlocking();

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
	
	// SERVER-EVENT\nBlueprint override event. Contains the attack logic.
	UFUNCTION(BlueprintNativeEvent) void PrimaryAttack();
	
	// SERVER-EVENT\nBlueprint override event. Contains the attack logic.
	UFUNCTION(BlueprintNativeEvent) void SecondaryAttack();
	
protected: // functions
	
	UFUNCTION(BlueprintCallable) void SetCharacterTeam(ECharacterTeam NewTeam);

	UFUNCTION()	virtual void HotkeyTriggered(UInputAction* HotkeyAction);
	
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void PostRegisterAllComponents() override;

	virtual void CharacterRestoredFromSave(const FString SaveSlotName);

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void Tick(float DeltaTime) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// CPP Virtual Void override for calling PrimaryAttack() (Blueprint Override)
	virtual void PrimaryAttackVirtual();
	// CPP Virtual Void override for calling SecondaryAttack() (Blueprint Override)
	virtual void SecondaryAttackVirtual();
	
	// SERVER-EVENT\nBlueprint override event. Contains the attack logic.
	UFUNCTION(BlueprintNativeEvent) void PrimaryAction();
	// CPP Virtual Void override for calling PrimaryAction() (Blueprint Override)
	virtual void PrimaryActionVirtual();
	
	// SERVER-EVENT\nBlueprint override event. Contains the attack logic.
	UFUNCTION(BlueprintNativeEvent) void SecondaryAction();
	// CPP Virtual Void override for calling SecondaryAction() (Blueprint Override)
	virtual void SecondaryActionVirtual();

	virtual void PostInitializeComponents() override;
	
private: // methods
	
	UFUNCTION(Client, Reliable)
	void Client_CharacterRestored(const FString& SaveSlotName);
	
	// Called when the equipment slot gets updated in InventoryComponent
	void UpdateWeapon(EWeaponSlots WeaponSlot = EWeaponSlots::PRIMARY);

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

	/** Vitality Welfare Component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UVitalityWelfareComponent*	VitalityWelfare;

	/** Vitality Stats Component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UVitalityStatComponent*		VitalityStats;

	/** Vitality Effects Component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UVitalityEffectsComponent*	VitalityEffects;

	/** Weapon Component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UWeaponComponent*	 WeaponComponent;

	/** Ability Component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UAbilityComponent*	 AbilityComponent;

	/** Mesh Merge Component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UMeshMergeComponent*  MeshMergeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Settings") int UnlockPointsOnLevelUp = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Settings")
	TSubclassOf<AFloatingTextBase> DamageTextActor = AFloatingTextBase::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character Settings")
	FName PronounSubject	= "he";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character Settings")
	FName PronounObjective	= "him";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character Settings")
	FName PronounPossessive = "his";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character Settings")
	FSlateColor SkinColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Character Settings")
	TArray<FStCharacterParts> BodyData;
	
private:

	UPROPERTY(Replicated) ECharacterTeam _CharacterTeam = ECharacterTeam::SPECTATOR;
	
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
};
