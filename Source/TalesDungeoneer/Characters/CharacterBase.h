
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"

#include "InventoryComponent.h"
#include "VitalityComponent.h"

#include "Components/WeaponComponent.h"
#include "Components/AbilityComponent.h"
#include "Components/MeshMergeComponent.h"
#include "Components/WidgetComponent.h"
#include "TalesDungeoneer/Entities/SimpleActors/FloatingTextBase.h"

#include "TalesDungeoneer/lib/datastructures/GlobalData.h"

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

// Called anytime the characters level has been modified, up or down
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterLevelChanged);


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
	
	/** Returns CameraBoom sub object **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera sub object **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION(BlueprintCallable)
	void ToggleWeapon(EWeaponSlots WeaponSlot = EWeaponSlots::PRIMARY,
						bool ForceDraw = false, bool ForceStow = false);

	/** Called to start an attack.
	 * Once started, the attack will continue until finished.
	 * @param WeaponSlot The weapon slot to attack with (defaults to Primary)
	 */
	UFUNCTION(BlueprintCallable)
	void PerformAttack(EWeaponSlots WeaponSlot = EWeaponSlots::PRIMARY);

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

	UFUNCTION(BlueprintPure)
	ECharacterClass GetCharacterClass() const { return _CharacterClass; }

	UFUNCTION(BlueprintPure)
	ECharacterRace GetCharacterRace() const { return _CharacterRace; }

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
	UFUNCTION(BlueprintPure) FString GetCharacterName() const { return _CharacterName; }

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
	
	/**
	 * @brief Sets the new name for this character. Typically used during creation/loading.
	 * @param ProposedName The new name string to use for this character
	 */
	UFUNCTION(BlueprintCallable) void SetCharacterName(FString ProposedName);

	UFUNCTION(BlueprintCallable)
	virtual void LoadSaveData(const FString& SaveName,
		const int32 UserIndex, USaveGame* SaveData);
	
protected: // functions
	
	UFUNCTION(BlueprintCallable) void SetCharacterTeam(ECharacterTeam NewTeam);

	UFUNCTION()	virtual void HotkeyTriggered(UInputAction* HotkeyAction);
	
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void PostRegisterAllComponents() override;
	
	virtual void Tick(float DeltaTime) override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// SERVER-EVENT\nBlueprint override event. Contains the attack logic.
	UFUNCTION(BlueprintNativeEvent) void PrimaryAttack();
	// CPP Virtual Void override for calling PrimaryAttack() (Blueprint Override)
	virtual void PrimaryAttackVirtual();

	// SERVER-EVENT\nBlueprint override event. Contains the attack logic.
	UFUNCTION(BlueprintNativeEvent) void SecondaryAttack();
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
	
	// Called when the equipment slot gets updated in InventoryComponent
	void UpdateWeapon(EWeaponSlots WeaponSlot = EWeaponSlots::PRIMARY);

	UFUNCTION()
	void SpawnDamageText(AActor* DamageTaker, AActor* DamageInstigator, float DamageTaken);

	// Called when the character starts an ability.
	// Checks if the ability should modify the combat state
	UFUNCTION()	void CheckAbilityStart(FName AbilityName, float CastTime);
	
	// Called when the character successfully activates an ability
	// Checks if the ability should modify the combat state
	UFUNCTION()	void CheckAbilitySuccess(FName AbilityName, bool WasSuccessful);
	
public: // members
	
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpInputAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveInputAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookInputAction;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PrimaryAttackInputAction;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SecondaryAttackInputAction;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PrimaryInputAction;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SecondaryInputAction;
	
	/** Character Inventory */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UInventoryComponent* InventoryComponent;

	/** Vitality Component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UVitalityComponent*	 VitalityComponent;

	/** Weapon Component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UWeaponComponent*	 WeaponComponent;

	/** Ability Component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UAbilityComponent*	 AbilityComponent;

	/** Overhead Component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UWidgetComponent*	 OverheadWidget;

	/** Mesh Merge Component */
	UPROPERTY(EditAnywhere, BlueprintReadWrite) UMeshMergeComponent*  MeshMergeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int UnlockPointsOnLevelUp = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AFloatingTextBase> DamageTextActor = AFloatingTextBase::StaticClass();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FName PronounSubject	= "he";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FName PronounObjective	= "him";
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FName PronounPossessive = "his";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	FSlateColor SkinColor;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated)
	TArray<FStCharacterParts> BodyData;
	
private:

	
	UPROPERTY(Replicated) ECharacterTeam _CharacterTeam = ECharacterTeam::SPECTATOR;
	UFUNCTION(Client, Reliable) void OnRep_CharacterLevel(int OldLevel);
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_CharacterLevel) int _CharacterLevel = 1;

	UFUNCTION(Client, Reliable) void OnRep_ExperienceChanged();
	UPROPERTY(Replicated, ReplicatedUsing=OnRep_ExperienceChanged) float _ExperiencePoints = 0.f;
	
	UPROPERTY(Replicated) ECharacterClass _CharacterClass	= ECharacterClass::WARRIOR;
	UPROPERTY(Replicated) ECharacterRace _CharacterRace		= ECharacterRace::HUMAN;

	UPROPERTY(Replicated) FString _CharacterName = "Unnamed Character";
	
	UPROPERTY(Replicated) int _CharacterRisk = 0;

	//UPROPERTY(Replicated) FStCharacterStats _CharacterStats = FStCharacterStats();

	float _BaseExperience  = 1000.f;
	float _ExpGrowthFactor = 1.2;
	float _ExperienceWorth = 100.f;

	UPROPERTY(Replicated) bool _IsMale = true;
};
