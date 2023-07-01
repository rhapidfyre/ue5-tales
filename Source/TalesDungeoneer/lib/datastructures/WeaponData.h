
#pragma once

#include "CoreMinimal.h"
#include "PickupActorBase.h"
#include "Engine/DataTable.h"
#include "Engine/EngineTypes.h"
#include "TalesDungeoneer/lib/enums/WeaponEnums.h"

#include "WeaponData.generated.h"


class AWeaponBase;
class UDamageTypeBase;


USTRUCT(BlueprintType)
struct FStWeaponSoundData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float       DelaySoundDrawWeapon	= 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* UseSoundDrawWeapon		= nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float       DelaySoundStowWeapon	= 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* UseSoundStowWeapon		= nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float       DelaySoundWeaponHit		= 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* UseSoundWeaponHit		= nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float       DelaySoundWeaponAttack	= 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* UseSoundWeaponAttack	= nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float       DelaySoundWeaponReload	= 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USoundBase* UseSoundWeaponReload	= nullptr;
};

USTRUCT(BlueprintType)
struct FStWeaponDamageData
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<UDamageTypeBase> DamageType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float BaseDamage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DamageVariance;
};

USTRUCT(BlueprintType)
struct FStWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DisplayName = "Display Name";
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EWeaponTypes WeaponType = EWeaponTypes::NONE;

	// The weapon actor that is spawned when this becomes equipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<AWeaponBase> SpawnClass;

	// The actor that spawns when the weapon is dropped on the ground
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSubclassOf<APickupActorBase> DropClass;

	// The skeletal mesh the weapon should use.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) USkeletalMesh* Mesh = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector MeshOffset = FVector(0,0,0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotator MeshRotOffset = FRotator(0,0,0);
	
	// Minimum time between attacks with this weapon, unmodified
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float AttackDelay = 1.2f;

	// Amount of time between initiation of attack and when the hit detection should be made
	// Useful for things like swinging a sword. The hit does not happen immediately.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float HitDetectDelay = 0.32;
	
	// Time until hit detector deactivates. Will adjust if it ends up larger than attack delay.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float HitDetectStop = 0.6f;

	// Deprecated, use HitDetectDelay
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float HitDelay = 0.0f;
	
	// The time it takes to employ this weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DelayDrawTime = 1.0f;

	// The time it takes to stow this weapon
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float DelayStowTime = 1.0f;
	 
	// How far away from the weapon it can hit, in centimeters.
	// The line trace ends at this distance from the start point.
	// Used for validation (Distance Of Actor 1 -> Actor 2)
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxReachDistance = 16.0f;

	// Maximum hit radius of the weapon. Values below 1.f mean it can only hit one target.
	// For melee weapons, this is the size of the hit detection. Zero means pinpoint accuracy.
	// For ranged weapons, this is the size of the projectile it fires.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float MaxHitRadius = 0.0f;

	// AoE Splash Damage Range, in centimeters.
	// If less than 1.f, the attack will be single target instead of AoE (default)
	// For values 1.f or higher, the attack will hit everything within this range.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SplashSize = 0.f;

	// If SplashSize is set, this is the minimum distance where anything within this zone
	// takes full damage. The damage is then fractional beyond this size, up to no damage at SplashSize.
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float SplashZone = 0.f;

	// The starting point of where attack detection should start
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector OffsetDetectStart = FVector(0,0,0);

	// The starting point of where attack detection should end
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector OffsetDetectEnd = FVector(0,0,0);
	
	// Which bone the weapon should attach to when held
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ActorHeldBone = "weapon_r";
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector ActorHeldOffset = FVector(0,0,0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotator ActorHeldRotation = FRotator(0,0,0);

	// Which bone the weapon should attach to when stowed
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FName ActorHolsterBone = "spine_02";
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector ActorHolsterOffset = FVector(0,0,0);
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotator ActorHolsterRotation = FRotator(0,0,0);

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FStWeaponSoundData WeaponSounds;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FStWeaponDamageData> DamageData;

};