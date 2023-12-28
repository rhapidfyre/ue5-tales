
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "WeaponBase.generated.h"


DECLARE_LOG_CATEGORY_EXTERN(LogWeapons, Log, Error);


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
	
	UFUNCTION(BlueprintCallable)
	bool setWeaponIsArmed(bool setArmed = false);
	
	UFUNCTION(BlueprintPure)
	bool getIsWeaponArmed() const { return bIsWeaponArmed; };

	UFUNCTION(BlueprintPure)
	bool getIsMeleeWeapon();
	
	UFUNCTION(BlueprintPure)
	bool getIsRangedWeapon();

	UFUNCTION(BlueprintPure) bool GetIsDebuggingMode() const { return bIsDebugging; }
	
	virtual bool doAttack();
	
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
	
	virtual void UpdateWeapon();

	virtual bool GetIsAttackValid(AActor* HitActor);
	UFUNCTION(BlueprintNativeEvent)
	bool IsAttackValid(AActor* HitActor);
	
	// Called when the weapon is stowed or drawn
	UFUNCTION(NetMulticast, Reliable)
	virtual void OnRep_IsWeaponArmed();
	
private: // private functions
	
	// private variables
	// Replicated boolean for whether the weapon is armed or not
	UPROPERTY(Replicated, ReplicatedUsing = OnRep_IsWeaponArmed)
	bool bIsWeaponArmed = false;

#ifdef UE_BUILD_DEBUG
	bool bIsDebugging = true;
#else
	bool bIsDebugging = false;
#endif

	// Simple boolean for ensuring the weapon has initialized
	bool bWeaponReady	= false;
	bool bShowDebug		= false;
	bool bVerboseOutput = true;
	
};
