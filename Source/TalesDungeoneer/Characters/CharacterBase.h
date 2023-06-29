
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InventoryComponent.h"
#include "VitalityComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/AbilityComponent.h"
#include "Components/WidgetComponent.h"

#include "CharacterBase.generated.h"

// Called when primary action is triggered
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPrimaryAction);

// Called when secondary action is triggered
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSecondaryAction);

// Called when primary attack is triggered
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPrimaryAttack);

// Called when secondary attack is triggered
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSecondaryAttack);

UENUM(BlueprintType)
enum class ECharacterTeam : uint8
{
	SPECTATOR	UMETA(DisplayName = "Unassigned"),
	PLAYER		UMETA(DisplayName = "Players"),
	DUNGEONEER	UMETA(DisplayName = "Dungeoneer"),
	FRIEND		UMETA(DisplayName = "Friendly NPCs"),
	ENEMY		UMETA(DisplayName = "Enemy NPCs")
};

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

	UPROPERTY(BlueprintCallable) FOnPrimaryAction OnPrimaryAction;
	UPROPERTY(BlueprintCallable) FOnPrimaryAttack OnPrimaryAttack;
	UPROPERTY(BlueprintCallable) FOnSecondaryAction OnSecondaryAction;
	UPROPERTY(BlueprintCallable) FOnSecondaryAttack OnSecondaryAttack;
	
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
	
	UFUNCTION(BlueprintPure)
	int GetCharacterLevel() const { return _CharacterLevel; }

	UFUNCTION(BlueprintPure)
	ECharacterClass GetCharacterClass() const { return _CharacterClass; }

	UFUNCTION(BlueprintPure)
	int GetRiskLevel() const { return _CharacterRisk; }
	
protected: // functions
	
	UFUNCTION(BlueprintCallable) void SetCharacterTeam(ECharacterTeam NewTeam);

	UFUNCTION()	virtual void HotkeyTriggered(UInputAction* HotkeyAction);
	
	virtual void BeginPlay() override;
	
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

	
	
private:

	UPROPERTY(Replicated) ECharacterTeam _CharacterTeam = ECharacterTeam::SPECTATOR;

	UPROPERTY(Replicated) int _CharacterLevel = 1;
	
	UPROPERTY(Replicated) ECharacterClass _CharacterClass = ECharacterClass::WARRIOR;

	UPROPERTY(Replicated) int _CharacterRisk = 0;
};
