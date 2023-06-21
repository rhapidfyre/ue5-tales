
#include "ProjectileSpellBase.h"

#include "AiController.h"
#include "TalesDungeoneer/Entities/ProjectileBase.h"


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
	
	SetGravityConsidered(SpellData.ImpactData.bUseGravity);
	
	FVector EndPosition = GetImpactPosition();
	AActor* TargetActor = GetTargetActor();
	if (IsValid(TargetActor))
		EndPosition = TargetActor->GetActorLocation();
	
	const FVector StartPosition = GetActorLocation();

	/*
	FVector FaceDirection = (EndPosition - StartPosition).GetSafeNormal();
	FRotator FaceRotation = FaceDirection.Rotation();
	*/

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
	
	FTransform SpawnTransform( StartPosition );
			   SpawnTransform.SetRotation( FaceRotation.Quaternion() );
			   SpawnTransform.SetScale3D( FVector(SpellData.VisualEffects.EffectScale) );
	
	TSubclassOf<AProjectileBase> UsingActor = AProjectileBase::StaticClass();
	if (IsValid(SpellData.ImpactData.ProjectileActor))
		UsingActor = SpellData.ImpactData.ProjectileActor;
	
	AProjectileBase* Projectile = GetWorld()->
	SpawnActorDeferred<AProjectileBase>(UsingActor, SpawnTransform);
	
	if (IsValid(Projectile))
	{
		Projectile->SetGravityConsidered(_GravityScale > 0.f);
		Projectile->SetProjectileSpeed(SpellData.ImpactData.ProjectileSpeed * FaceRotation.Vector());
		Projectile->SetProjectileData( GetAbilityName() );
		Projectile->SetTargetActor( GetTargetActor() );
		
		Projectile->FinishSpawning(SpawnTransform);
		
		UE_LOG(LogTemp, Display, TEXT("%s(%s): Projectile Spawned - %f"),
			*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), _GravityScale);
		
	}

	UE_LOG(LogTemp, Display, TEXT("(%s)(%s): Spawn Position '%s', Owner Actor Position '%s'"),
		*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"),
		*Projectile->GetActorLocation().ToString(),*GetActorLocation().ToString());
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
