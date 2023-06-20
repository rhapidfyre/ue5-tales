
#include "ProjectileSpellBase.h"

#include "Kismet/KismetMathLibrary.h"
#include "TalesDungeoneer/Entities/ProjectileBase.h"


AProjectileSpellBase::AProjectileSpellBase()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AProjectileSpellBase::SetProjectileSpeed(FVector SpeedVector)
{
	_SpeedVector = SpeedVector;
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
	
	SetGravityConsidered(SpellData.bUseGravity);
	SetProjectileSpeed(SpellData.ImpactData.SpeedDirection);
	
	FVector EndPosition = GetImpactPosition();
	AActor* TargetActor = GetTargetActor();
	if (IsValid(TargetActor))
		EndPosition = TargetActor->GetActorLocation();
	
	const FVector StartPosition = GetActorLocation();
	
	FVector FaceDirection = (StartPosition - EndPosition).GetSafeNormal();
	FRotator FaceRotation = FaceDirection.Rotation();
	
	FTransform SpawnTransform( FaceRotation );
			   SpawnTransform.SetLocation( StartPosition );
			   SpawnTransform.SetScale3D( FVector(SpellData.VisualEffects.EffectScale) );

	TSubclassOf<AProjectileBase> UsingActor = AProjectileBase::StaticClass();
	if (IsValid(SpellData.ProjectileActor))
		UsingActor = SpellData.ProjectileActor;
	
	AProjectileBase* Projectile = GetWorld()->
				SpawnActorDeferred<AProjectileBase>(UsingActor, SpawnTransform);

	if (IsValid(Projectile))
	{
		Projectile->SetGravityConsidered(_GravityScale > 0.f);
		Projectile->SetProjectileSpeed(_SpeedVector);
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
