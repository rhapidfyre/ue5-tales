
#include "ProjectileSpellBase.h"

#include "TalesDungeoneer/Entities/Projectiles/SpellProjectileBase.h"


AProjectileSpellBase::AProjectileSpellBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AProjectileSpellBase::SetProjectileSpeed(float ProjectileSpeed)
{
	_ProjectileSpeed = ProjectileSpeed;
}

void AProjectileSpellBase::SetGravityConsidered(bool AllowGravity)
{
	_GravityScale = AllowGravity ? 1.f : 0.f;
}

void AProjectileSpellBase::FireProjectile()
{
	if (!HasAuthority())
		return;

	const FStSpellData SpellData = GetSpellData();
	
	FVector EndPosition = GetImpactPosition();
	AActor* TargetActor = GetTargetActor();
	if (IsValid(TargetActor))
		EndPosition = TargetActor->GetActorLocation();
	
	const FVector StartPosition = GetActorLocation();

	FRotator FaceRotation = FRotator::ZeroRotator;
	
	AController* PawnController = GetOriginatingActor()->GetInstigatorController();
	if (IsValid(PawnController))
	{
		FVector CamLocation;
		PawnController->GetPlayerViewPoint(CamLocation, FaceRotation);
	}
	
	// If we failed to get a viewpoint, get the difference in impact location
	if (FaceRotation.IsNearlyZero())
	{
		FaceRotation = (EndPosition - StartPosition).GetSafeNormal().Rotation();
	}
	
	for (FStProjectileData ProjectileData : SpellData.ImpactData)
	{
		FTransform SpawnTransform( StartPosition + ProjectileData.SpawnOffset );
		SpawnTransform.SetRotation( (FaceRotation + ProjectileData.SpawnRotation).GetNormalized().Quaternion() );
		SpawnTransform.SetScale3D( FVector(1.f) );
	
		TSubclassOf<ASpellProjectileBase> UsingActor = ASpellProjectileBase::StaticClass();
		if (IsValid(ProjectileData.Projectile))
			UsingActor = ProjectileData.Projectile;
	
		ASpellProjectileBase* Projectile = GetWorld()->
				SpawnActorDeferred<ASpellProjectileBase>(UsingActor, SpawnTransform);
	
		if (IsValid(Projectile))
		{
			Projectile->SetInstigator( GetInstigator() );
			Projectile->SetTargetActor( GetTargetActor() );
			Projectile->SetProjectileData( GetAbilityName() );
			Projectile->FinishSpawning(SpawnTransform);
		}
	}
}

FVector AProjectileSpellBase::GetImpactPosition() const
{
	const AActor* TargetActor = GetTargetActor();
	FVector EndPosition = GetImpactLocation();
	if (IsValid(TargetActor))
	{
		FVector ActorOrigin, BoxExtents;
		TargetActor->GetActorBounds(false,
			ActorOrigin, BoxExtents, false);
		EndPosition = ActorOrigin;
	}
	return EndPosition;
}

void AProjectileSpellBase::BeginPlay()
{
	Super::BeginPlay();
}

bool AProjectileSpellBase::PerformSingleHitTrace(FHitResult& HitResult)
{
	FCollisionQueryParams CollisionQuery;
	return GetWorld()->LineTraceSingleByChannel(HitResult,
		GetActorLocation(), GetImpactPosition(),
		ECC_Visibility, CollisionQuery);
}

bool AProjectileSpellBase::PerformMultiHitTrace(TArray<FHitResult>& HitResults)
{
	const AActor* TargetActor = GetTargetActor();
	FVector EndPosition = GetImpactLocation();
	if (IsValid(TargetActor))
	{
		FVector ActorOrigin, BoxExtents;
		TargetActor->GetActorBounds(false,
			ActorOrigin, BoxExtents, false);
		EndPosition = ActorOrigin;
	}
	FCollisionQueryParams CollisionQuery;
	return GetWorld()->LineTraceMultiByChannel(HitResults,
		GetActorLocation(), EndPosition,
		ECC_Visibility, CollisionQuery);
}

void AProjectileSpellBase::AbilityComplete(bool WasSuccessful)
{
	if (WasSuccessful)
	{
		const FStAbilityData AbilityData = GetAbilityData();
		if (AbilityData.TargetType == EAbilityTarget::PROJECTILE)
		{
			FireProjectile();
		}
	}
	// This calls Destroy(), logic must happen first
	Super::AbilityComplete(WasSuccessful);
}
