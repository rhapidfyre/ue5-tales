
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "WeaponBase.generated.h"


// Only useful on the owning client. Called when the weapon hits something.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHit, AActor*, HitActor);

/**
 * WEAPON BASE
 * A C++ class containing all of the logic; Methods & Members, for all functionality in regards to weapons.
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public: // public functions
	
	// Sets default values for this actor's properties
	AWeaponBase();

	UPROPERTY(BlueprintAssignable, Category = "Weapon Events") FOnHit OnHit;
	
	/**
	 * Set to true when the weapon should be drawn. False to stow.
	 * @param setArmed True for draw, false for stow.
	 */
	UFUNCTION(BlueprintCallable) void setWeaponIsArmed(bool setArmed = false);
	UFUNCTION(BlueprintPure) bool getIsWeaponArmed() const { return bIsWeaponArmed; };

	/**
	 * Received when the client's weapon is saying that it hit something
	 * Requires validation! Trust but verify.
	 * @param hitActor The actor the client says that they hit.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_RequestWeaponHit(AActor* hitActor);

	/** Used on the client making the attack. Performs sounds, animations and
	 * all hit related effects prior to the server event, to ensure everything
	 * appears seamless to the client making the attack.
	 */
	void PerformWeaponHit(AActor* hitActor);

	UFUNCTION(BlueprintPure) bool getIsMeleeWeapon();
	UFUNCTION(BlueprintPure) bool getIsRangedWeapon();
	
	/**
	 * Gets information regarding the weapon's data.
	 * @return The FStWeaponData of this weapon item
	 */
	UFUNCTION(BlueprintCallable) FStWeaponData getWeaponData();
	
	
	virtual bool doAttack();
	virtual void cancelAttack();
	UFUNCTION(BlueprintPure) bool getIsAttacking();
	UFUNCTION(BlueprintNativeEvent) void startAttack();
	UFUNCTION(BlueprintNativeEvent) void stopAttack();

	UFUNCTION(BlueprintCallable) void setWeaponName(FName weaponName);

	UFUNCTION(BlueprintPure) FName getWeaponName() const { return mWeaponName; }

	// Skeletal Mesh of the weapon, for graphics/display/cosmetics
	UPROPERTY(Replicated, EditAnywhere)
		USkeletalMeshComponent* mSkeletalMesh;
	
protected: // protected functions
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostActorCreated() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual bool checkForHit();
	virtual void updateWeapon();
	virtual void startAttackTimer();
	virtual void cancelAttackTimer();
	
	/**
	 * Plays the given sound effect after the given delay from the actor's world location as a 3D noise.
	 * @param soundEffect The USoundBase* being played after the delay.
	 * @param soundDelay The FTimerHandle delay time to being played. If <= 0, plays immediately.
	 */
	UFUNCTION() void soundEffectWithDelay(USoundBase* soundEffect, float soundDelay = 0.0f);
	UFUNCTION() void niagaraEffectWithDelay(UNiagaraSystem* niagaraEffect, float effectDelay = 0.0f, float isLooped = false);
	UFUNCTION() void particleEffectWithDelay(UParticleSystem* particleEffect, float effectDelay = 0.0f, float isLooped = false);
	
	// Called when the weapon is stowed or drawn
	UFUNCTION(NetMulticast, Reliable) virtual void OnRep_IsWeaponArmed();

	// Called when the weapon is changed
	UFUNCTION(NetMulticast, Reliable) virtual void OnRep_WeaponName();
	
private: // private functions
	
	virtual void startStowEffects(float delayTime = 0.f);
	virtual void startDrawEffects(float delayTime = 0.f);
	
	/**
	 * Sends the sound effect to all clients. Usually used in conjunction with soundEffectWithDelay,
	 * although this can be used directly. Ensures that sounds effects for this actor are networked properly.
	 * @param soundEffect The USoundBase* effect to play on all eligible clients.
	 */
	UFUNCTION(NetMulticast, Unreliable) void sendSoundEffect(USoundBase* soundEffect);
	
	// For use with a timer
	UFUNCTION() void PrepSoundEffect(USoundBase* soundEffect) { sendSoundEffect(soundEffect); }
	
	// private variables
	// Replicated boolean for whether the weapon is armed or not
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_IsWeaponArmed)
	bool bIsWeaponArmed = false;
	
	// Replicated FName for the weapon in use.
	// Used to access data tables as things are needed.
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_WeaponName, EditAnywhere)
	FName mWeaponName = "None";
	
	// Animation Instance for Animation Blueprinting (Anim Logic)
	UPROPERTY(Replicated) TSubclassOf<UAnimInstance> mAnimInstance;

	
	FTimerHandle mAttackTimer;

	// Simple boolean for ensuring we don't overlap events/commands
	bool bIsOperating = false;

	// Simple boolean for ensuring the weapon has initialized
	bool bWeaponReady = false;

	bool bShowDebug = false;
	bool bVerboseOutput = true;

	// Server Only
	FDateTime mNextAttackTime = FDateTime::UtcNow();
	
};
