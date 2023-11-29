// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TalesDungeoneer/lib/enums/WeaponEnums.h"
#include "WeaponComponent.generated.h"

/**
 * Bridges ACharacterBase class to the AWeaponBase
 * Allows communication between the two classes while keeping them separate.
 * Simply add this component to the character class to use it.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TALESDUNGEONEER_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	// Sets default values for this component's properties
	UWeaponComponent();

	/**
	 * Toggles the weapon given by the makeReady value & slot
	 * Does not validate anything. Performs what you tell it to do.
	 * @param weaponType The EWeaponSlots of the slot being toggled. Defaults to PRIMARY.
	 * @param makeReady If true, arms the weapon. If false (default), stows the weapon.
	 */
	UFUNCTION(BlueprintCallable)
	void SetToggleWeapon(EWeaponSlots weaponType = EWeaponSlots::PRIMARY, bool makeReady = false);

	/**
	 * Adjusts the weapon's attachment based on it's armed status.
	 * To change what these values actually use, edit DT_WeaponData in the UE Asset Browser
	 * @param weaponSlot The weapon slot we're adjusting
	 */
	UFUNCTION(BlueprintCallable)
	void AdjustWeaponAttachment(EWeaponSlots weaponSlot = EWeaponSlots::PRIMARY);
	
	/**
	 * Receives a request from the client (player controller) to toggle the weapon in the given slot.
	 * Performs validation checks.
	 * @param weaponType The EWeaponSlots of the slot being toggled
	 * @param makeReady If true, arms the weapon. If false (default), stows the weapon.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ToggleWeapon(EWeaponSlots weaponType = EWeaponSlots::PRIMARY, bool makeReady = false);

	/**
	 * Called to perform a hit with the selected weapon in the given slot.
	 * On the client, this just performs psuedo hits so the client doesn't appear to lag.
	 * On the server, this performs the actual hit detection and multicasts the attack animation.
	 * @param weaponType The EWeaponSlots that is performing the hit check
	 * @return True if the attack passed all validation
	 */
	UFUNCTION(BlueprintCallable)
	bool PerformAttack(EWeaponSlots weaponType = EWeaponSlots::PRIMARY);

	/** Called to start or stop a blocking action, such as using a shield. */
	UFUNCTION(BlueprintCallable, Category = "Weapon Actions")
	void SetIsBlocking(bool IsBlocking = true);
	
	/** Gets whether or not the Weapon Component considers the player to be blocking.
	 * @return Returns true if blocking, false if not blocking.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon Actions")
	bool GetIsBlocking();
	
	/**
	 * Called to set a hard-target. If null, free-targeting will be used.
	 * Hard-Target allows characters to "lock on" to certain targets to ensure
	 * their attacks always inquire as to whether they are hit or not.
	 * @param targetActor The AActor pointer that the character is targeting.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon Data Setters")
	void SetTargetedActor(AActor* targetActor);

	/**
	 * Clears the targeted actor.
	 * 'SetTargetedActor(nullptr)' works, but this method is cleaner.
	 */
	UFUNCTION(BlueprintCallable, Category = "Weapon Data Setters")
	void ClearTargetedActor();
	void UnsetWeapon(EWeaponSlots weaponSlot);

	UFUNCTION(BlueprintPure, Category = "Weapon Data Accessors")
	AWeaponBase* GetWeaponInSlot(
		EWeaponSlots weaponSlot = EWeaponSlots::PRIMARY) const { return _PrimaryWeapon; }

	UFUNCTION(BlueprintPure, Category = "Weapon Data Accessors")
	bool GetIsWeaponReady(EWeaponSlots WeaponSlot = EWeaponSlots::PRIMARY) const;

	UFUNCTION(BlueprintCallable, Category = "Weapon Data Setters")
	void SetWeapon( FName weaponName,
					EWeaponSlots weaponSlot = EWeaponSlots::PRIMARY);
	
	UFUNCTION(BlueprintPure)
	EWeaponTypes GetWeaponStyle() const { return _WeaponStyle; }

	void SetPrimarySlotNumber(int slotNumber = -1)   { _PrimarySlot = slotNumber;   }
	
	void SetSecondarySlotNumber(int slotNumber = -1) { _SecondarySlot = slotNumber; }
	
	// Where KEY is the weapon type and VALUE is the animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon System Settings")
	TMap<EWeaponTypes, UAnimMontage*> AttackAnimations;

	// Where KEY is the sound, and VALUE is the delay before playing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon System Settings")
	TMap<USoundBase*, float> AttackSounds = {};

	// Where KEY is the weapon type and VALUE is the animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon System Settings")
	TMap<EWeaponTypes, UAnimMontage*> StowAnimations;
	
	// Where KEY is the sound, and VALUE is the delay before playing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon System Settings")
	TArray<USoundBase*> StowSounds = {};
	
	// Where KEY is the weapon type and VALUE is the animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon System Settings")
	TMap<EWeaponTypes, UAnimMontage*> DrawAnimations;
	
	// Where KEY is the sound, and VALUE is the delay before playing
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon System Settings")
	TArray<USoundBase*> DrawSounds = {};
	
protected:
	
	// Called when the game starts
	virtual void BeginPlay() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnComponentCreated() override;

	// Called when a weapon has finished operating and is now ready
	// for another operation (drawing/sheathing)
	UFUNCTION() virtual void WeaponSlotReady(EWeaponSlots WeaponSlot);

	virtual void OnUnregister() override;

private:
	
	/**
	 * Spins up a timer to play a sound effect with the given delay. Only useful for delayed sounds.
	 * For sounds with no delay, just call Multicast_WeaponSoundEffect directly.
	 * @param soundToPlay The sound to be played after the delay.
	 * @param soundDelay The amount of time to delay until the sound is multicast.
	*/
	UFUNCTION()
	void DelaySoundEffect(USoundBase* soundToPlay, float soundDelay = 0.0f);

	// Simple function called by delaySoundEffects timer to send multicast event
	UFUNCTION()
	void SendSoundEffect(ACharacter* characterRef, USoundBase* soundToPlay);

	/**
	 * Called when the '_TargetActor' private variable has been changed.
	 * Always validate and check for nullptr.
	 */
	UFUNCTION(Client, Reliable)
	void OnRep_TargetChanged();

	/**
	 * Received from a client when they request to perform an attack.
	 * If it's valid, it will multicast the animation to everyone.
	 * @param weaponType The EWeaponSlots performing the attack. Defaults to PRIMARY.
	 */
	UFUNCTION(Server, Unreliable)
	void Server_RequestAttack(EWeaponSlots weaponType = EWeaponSlots::PRIMARY);

	/**
	 * Using the given character and animation montage, sends all players the animation to play.
	 * @param characterRef the ACharacter* playing the animation
	 * @param animToPlay The UAnimMontage* to play on the given character
	 */
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_SendAnimation(ACharacter* characterRef, UAnimMontage* animToPlay);

	/**
	 * Sends a sound effect to the client which will be played immediately upon being received
	 * @param characterRef The character emitting the noise
	 * @param soundEffect The sound to be played
	 */
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_WeaponSoundEffect(ACharacter* characterRef, USoundBase* soundEffect);

	/**
	 * Called internally from OnRep when a weapon's ready state changes
	 * @param WeaponSlot The weapon slot that is changing
	 * @return The time it takes for the weapon to operate (draw/sheathe)
	 */
	float WeaponReadyChanged(EWeaponSlots WeaponSlot = EWeaponSlots::PRIMARY);
	
	UPROPERTY(Replicated) AWeaponBase* _PrimaryWeapon;
	UPROPERTY(Replicated) AWeaponBase* _SecondaryWeapon;	

	// Quick ints to save runtime performance later
	int _PrimarySlot = -1;
	int _SecondarySlot = -1;

	// Used for hard-targeting instead of free-targeting
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_TargetChanged) AActor* _TargetActor;

	// The currently selected weapon style
	UPROPERTY(Replicated) EWeaponTypes _WeaponStyle	= EWeaponTypes::NONE;
	
	// The GameTimeInSeconds of when the next attack may be requested
	// This is dependent on the 'delay' value of the weapon in use
	UPROPERTY() FDateTime _NextAttackTime = FDateTime::UtcNow();
	
	bool bShowDebug		= true;
	bool bVerboseOutput = true;

	// True if the primary item is acting and no actions can occur
	bool bPrimaryOperating		= false;

	// True if the secondary item is acting and no actions can occur
	bool bSecondaryOperating	= false;

	// TRUE if the player is actively blocking attacks with a shield device
	bool bBlocking      = true;
};
