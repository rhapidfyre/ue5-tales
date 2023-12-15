
#pragma once

#include "CoreMinimal.h"
#include "NiagaraSystem.h"
#include "GameFramework/Actor.h"
#include "lib/datastructures/WeaponData.h"

#include "WeaponBase.generated.h"


// Only useful on the owning client. Called when the weapon hits something.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHit, AActor*, HitActor);

DECLARE_LOG_CATEGORY_EXTERN(LogWeapons, Log, Error);

UENUM()
enum class EWeaponEffectType : uint8
{
	ATTACK, HIT, MISS, BREAK, FIRE, RELOAD
};
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

	UPROPERTY(BlueprintAssignable, Category = "Weapon Events")
	FOnHit OnHit;
	
	UFUNCTION(BlueprintCallable)
	bool setWeaponIsArmed(bool setArmed = false);
	
	UFUNCTION(BlueprintPure)
	bool getIsWeaponArmed() const { return bIsWeaponArmed; };

	UFUNCTION(Server, Unreliable, BlueprintCallable)
	void Server_RequestWeaponHit(AActor* HitActor);

	UFUNCTION(BlueprintCallable)
	void PerformWeaponHit(AActor* hitActor);

	UFUNCTION(BlueprintPure)
	bool getIsMeleeWeapon();
	
	UFUNCTION(BlueprintPure)
	bool getIsRangedWeapon();

	UFUNCTION(BlueprintPure) bool GetIsDebuggingMode() const { return bIsDebugging; }
	
	/**
	 * Gets information regarding the weapon's data.
	 * @return The FStWeaponData of this weapon item
	 */
	UFUNCTION(BlueprintCallable)
	FStWeaponData getWeaponData();
	
	virtual bool doAttack();
	
	virtual void cancelAttack();
	
	UFUNCTION(BlueprintPure) bool
	getIsAttacking();
	
	UFUNCTION(BlueprintNativeEvent)
	void startAttack();
	
	UFUNCTION(BlueprintNativeEvent)
	void stopAttack();

	UFUNCTION(BlueprintCallable)
	void setWeaponName(FName weaponName);

	UFUNCTION(BlueprintPure)
	FName getWeaponName() const { return mWeaponName; }
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* WeaponRoot = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Components")
	UStaticMeshComponent* WeaponMeshStatic = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Components")
	USkeletalMeshComponent* WeaponMeshSkeleton = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Components")
	USceneComponent* WeaponGripLeft = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Components")
	USceneComponent* WeaponGripRight = nullptr;
	
	// The static mesh to use. Overriden by UsingSkeletalMesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
	UStaticMesh* UsingStaticMesh = nullptr;

	// Overrides UsingStaticMesh
	// Weapon will always use a skeletal mesh if provided.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Settings")
	USkeletalMesh* UsingSkeletalMesh = nullptr;

	
	
	
protected: // protected functions
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PostActorCreated() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnConstruction(const FTransform& Transform) override;
	
	virtual bool CheckForHit(TArray<AActor*> &HitActors);
	virtual void UpdateWeapon();
	virtual void startAttackTimer();
	virtual void cancelAttackTimer();

	virtual bool GetIsAttackValid(AActor* HitActor);
	UFUNCTION(BlueprintNativeEvent)	bool IsAttackValid(AActor* HitActor);
	
	UFUNCTION(BlueprintPure) int GetMaxNumTargetsHitAtOnce() const { return MaxTargetsHitAtOnce_; }

	// Internal Weapon Operations
	UFUNCTION(NetMulticast, Unreliable) void Multicast_PlayWeaponAttack();
	UFUNCTION(NetMulticast, Unreliable) void Multicast_PlayWeaponHit();
	UFUNCTION(NetMulticast, Unreliable) void Multicast_PlayWeaponStow();
	UFUNCTION(NetMulticast, Unreliable) void Multicast_PlayWeaponDraw();

	UFUNCTION(Server, Unreliable) void Server_PlayWeaponEffect(const EWeaponEffectType WeaponEffect);
	
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

#ifdef UE_BUILD_DEBUG
	bool bIsDebugging = true;
#else
	bool bIsDebugging = false;
#endif
	
	FTimerHandle mAttackTimer;

	// Simple boolean for ensuring the weapon has initialized
	bool bWeaponReady = false;

	bool bShowDebug = false;
	bool bVerboseOutput = true;

	// The maximum targets this weapon can hit per attack
	int MaxTargetsHitAtOnce_ = 1;
	
};
