
#include "Weapons/WeaponMeleeBase.h"

#include "Kismet/BlueprintTypeConversions.h"
#include "Logging/StructuredLog.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Characters/CharacterBase.h"


/** Override of parent method (AWeaponBase)
 * If the parent succeeds, sets up for a melee attack with timers.
 * @return True if the attack preparations were successful. False otherwise.
 */
bool AWeaponMeleeBase::doAttack()
{
	if (Super::doAttack())
	{
		InitiateAttack();
		return true;
	}
	return false;
}

// WARNING - Mesh MUST have a "trace_start" and "trace_end" socket or trace will act funny
// Performs the attack logic by setting up the line trace and looping (if applicable)
void AWeaponMeleeBase::InitiateAttack()
{

}

bool AWeaponMeleeBase::GetIsAttackValid(AActor* HitActor)
{
	if (IsValid(HitActor))
	{

	}
	return false;
}

void AWeaponMeleeBase::UpdateWeapon()
{
	Super::UpdateWeapon();
}

void AWeaponMeleeBase::BeginPlay()
{
	Super::BeginPlay();

	bool DoesMeshHaveSockets = false;
	
	if (IsValid(WeaponMeshSkeleton->GetSkeletalMeshAsset()))
	{
		DoesMeshHaveSockets = (
				WeaponMeshSkeleton->DoesSocketExist(SocketStartName)
			&&	WeaponMeshSkeleton->DoesSocketExist(SocketEndName));
	}
	else
	{
		if (IsValid(WeaponMeshStatic->GetStaticMesh()))
		{
			DoesMeshHaveSockets = (
					WeaponMeshStatic->DoesSocketExist(SocketStartName)
				&&	WeaponMeshStatic->DoesSocketExist(SocketEndName));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("%s Failed to Validate. No mesh specified! Weapon will not work."), *GetName());
		}
	}
	if (!DoesMeshHaveSockets)
	{
		UE_LOG(LogTemp, Error, TEXT("%s Failed to Validate. Mesh does not have '%s' and '%s' sockets!!"),
			*GetName(), *SocketStartName.ToString(), *SocketEndName.ToString());
	}
}

void AWeaponMeleeBase::TargetHitByWeapon(AActor* HitActor)
{
}

void AWeaponMeleeBase::DoAttackTracing()
{
	if (TicksDelayed_ < 1)
	{
		/*
		const ACharacterBase* OwnerCharacter = Cast<ACharacterBase>( GetOwner() );
	
		ECharacterTeam OwnerTeam = IsValid(OwnerCharacter) ?
									OwnerCharacter->GetCharacterTeam()
									: ECharacterTeam::SPECTATOR;

		const FStWeaponData WeaponData = getWeaponData();
	
		// Perform at least one hit detection trace
		const int MaxTargetsHitAtOnce = GetMaxNumTargetsHitAtOnce();
	
		// Weapon can hit multiple targets
		TArray<FHitResult> HitResults;
		FVector StartVector, EndVector;
		const float SphereRadius = WeaponData.MaxHitRadius > 0 ? WeaponData.MaxHitRadius : 24.f;
		StartVector = GetActorLocation();
		EndVector   = GetActorLocation();

		EDrawDebugTrace::Type DebugTrace = GetIsDebuggingMode() ?
							EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
		const FLinearColor TraceColor = FLinearColor::Red;
		const FLinearColor TraceHitColor = FLinearColor::Green;
		const float TraceDrawTime = 1.f;

		// if the weapon has a valid mesh (it should), try to locate the
		// 'trace_start' and 'trace_end' sockets
		if (IsValid(WeaponMeshStatic) || IsValid(WeaponMeshSkeleton))
		{
			// Find out which mesh is being used
			const FName SocketStart = SocketStartName;
			const FName SocketEnd	= SocketEndName;
			USceneComponent* MeshReference = nullptr;
			if (IsValid(WeaponMeshStatic))
				MeshReference = WeaponMeshStatic;
			else
				MeshReference = WeaponMeshSkeleton;

			if (MeshReference->DoesSocketExist(SocketStart))
				StartVector = MeshReference->GetSocketLocation(SocketStart);
			if (MeshReference->DoesSocketExist(SocketEnd))
				EndVector = MeshReference->GetSocketLocation(SocketEnd);
		}

		// Check for blocking hits by Channel 4 (MeleeWeapon)
		ETraceTypeQuery TraceChannel(TraceTypeQuery4);
		TArray ActorsToIgnore({ GetOwner() });
	
		UKismetSystemLibrary::SphereTraceMulti(GetWorld(),
			StartVector, EndVector, SphereRadius, TraceChannel, false,
			ActorsToIgnore, DebugTrace, HitResults, true, TraceColor, TraceHitColor, TraceDrawTime);
		if (!HitResults.IsEmpty())
		{
			for (FHitResult& HitResult : HitResults)
			{
				if (HitTargets_.Num() < MaxTargetsHitAtOnce)
				{
					ACharacterBase* HitActor = Cast<ACharacterBase>(HitResult.GetActor());
					if (IsValid(HitActor))
					{
						// Do not hit members of the same team
						if (OwnerTeam != HitActor->GetCharacterTeam())
						{
							// Don't hit something that has already been hit
							if (!HitTargets_.Contains(HitActor))
							{
								HitTargets_.Add(HitActor);
								Server_RequestWeaponHit(HitActor);
							}
						}
					}
				}
				else
				{
					TimerTicksRemaining_ = 0;
					break;
				}
			}
		}
		*/
	}//ticks delayed

	// Only decrease timer ticks if the timer is no longer delayed
	if (TicksDelayed_ < 1)
		TimerTicksRemaining_ -= 1;
	else
		TicksDelayed_ -= 1;

	// If the timer has expired, kill it
	if (TimerTicksRemaining_ < 1 && TicksDelayed_ < 1)
	{
		TimerTicksRemaining_ = 0;
		GetWorldTimerManager().ClearTimer(AttackTimer_);
		UE_LOGFMT(LogTemp, Display, "{WeaponName} - Tracing Completed. {NumHits} Targets Hit.",
			*GetName(), HitTargets_.Num());
	}
}