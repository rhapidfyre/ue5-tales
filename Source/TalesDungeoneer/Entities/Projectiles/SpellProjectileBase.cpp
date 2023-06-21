
#include "SpellProjectileBase.h"

#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TalesDungeoneer/Characters/CharacterBase.h"
#include "lib/VitalityEnums.h"
#include "Net/UnrealNetwork.h"

void ASpellProjectileBase::PlayAbilityEffects(FStAbilityFx AbilityEffect)
{
	if (IsValid(AbilityEffect.NiagaraEffect))
	{
		if (AbilityEffect.bAttachNiagaraToActor || AbilityEffect.bAttachNiagaraToSkeleton)
		{
			UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
					AbilityEffect.NiagaraEffect, GetRootComponent(),
					NAME_None,FVector::ZeroVector,FRotator::ZeroRotator,
					EAttachLocation::SnapToTargetIncludingScale,true);
			
			if (IsValid(NiagaraComponent))
			{
				LoopingNiagaraEmitters.Add(NiagaraComponent);
				NiagaraComponent->SetWorldScale3D(FVector(AbilityEffect.EffectScale));
				NiagaraComponent->Activate();
			}
		}
		else
		{
			UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),
					AbilityEffect.NiagaraEffect, GetActorLocation(), GetActorRotation(),
					FVector(AbilityEffect.EffectScale),true, true);
		}
	}
	if (IsValid(AbilityEffect.SoundEffect))
	{
		UAudioComponent* ImpactSound = UGameplayStatics::SpawnSoundAtLocation(GetWorld(),
			AbilityEffect.SoundEffect, GetActorLocation(),
			FRotator::ZeroRotator, AbilityEffect.SoundVolume);
		if (IsValid(ImpactSound))
		{
			LoopingSoundEmitters.Add(ImpactSound);
			ImpactSound->bAutoDestroy = true;
			ImpactSound->Play();
		}
	}
}

void ASpellProjectileBase::ExecuteSpellEffect(FStAbilityFx AbilityFx, bool StopOnDestroyed)
{
	if (IsValid(AbilityFx.NiagaraEffect))
	{
		if (AbilityFx.DelayEffect)
		{
			FTimerHandle LoopTimer;
			FTimerDelegate LoopDelegate;
			LoopDelegate.BindUFunction(this, FName("PlayAbilityEffects"), AbilityFx);
			GetWorld()->GetTimerManager().SetTimer(LoopTimer, LoopDelegate, 1.f, false);
		}
		else
		{
			PlayAbilityEffects(AbilityFx);
		}
	}
	
	if (IsValid(AbilityFx.SoundEffect))
	{
		UAudioComponent* ImpactSound = UGameplayStatics::SpawnSoundAtLocation(GetWorld(),
			AbilityFx.SoundEffect, GetActorLocation());
		if (IsValid(ImpactSound))
		{
			LoopingSoundEmitters.Add(ImpactSound);
			ImpactSound->bAutoDestroy = true;
			ImpactSound->Play();
		}
	}
	
}

ASpellProjectileBase::ASpellProjectileBase(FName AbilityName)
{
	SetProjectileData(AbilityName);
}

void ASpellProjectileBase::SetProjectileData(FName AbilityName)
{
	_AbilityName = AbilityName;
	SetFromAbilityData();
}

void ASpellProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
	FStSpellData SpellData = UAbilitySystem::GetSpellDataFromName(_AbilityName);
	for (FStProjectileData ProjectileData : SpellData.ImpactData)
	{
		for (const FStAbilityFx AbilityEffect : ProjectileData.EffectsOnSpawn)
		{
			ExecuteSpellEffect(AbilityEffect);
		}
		
		// There needs to be a delay timer for this
		for (const FStAbilityFx AbilityEffect : ProjectileData.EffectsLooped)
		{
			ExecuteSpellEffect(AbilityEffect, true);
		}
		
	}
	
}

void ASpellProjectileBase::Destroyed()
{
	ApplyHitEffect(GetTargetActor(), GetActorLocation());
	
	for (UNiagaraComponent* NiagaraComponent : LoopingNiagaraEmitters)
	{
		if (IsValid(NiagaraComponent))
		{
			NiagaraComponent->Deactivate();
			NiagaraComponent->DestroyInstance();
		}
	}
	
	for (UAudioComponent* AudioComponent : LoopingSoundEmitters)
	{
		if (IsValid(AudioComponent))
		{
			AudioComponent->Stop();
			AudioComponent->DestroyComponent();
		}
	}
	
	Super::Destroyed();
}

void ASpellProjectileBase::SetFromAbilityData()
{
	const FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(_AbilityName);
	if (UAbilitySystem::GetAbilityDataIsValid(AbilityData))
	{
		SetMaxTravelDistance(AbilityData.MaxReach);
		_AbilityData = AbilityData;
	}
}

void ASpellProjectileBase::ApplyHitEffect(AActor* HitActor, FVector HitVector)
{
	const FStSpellData SpellData		= UAbilitySystem::GetSpellDataFromName(_AbilityName);
	const FStAbilityData AbilityData	= UAbilitySystem::GetAbilityDataFromName(_AbilityName);

	// Create the effects actor (Client & Server)
	for (FStProjectileData ProjectileData : SpellData.ImpactData)
	{
		for (FStAbilityFx AbilityFx : ProjectileData.EffectsFinal)
		{
			// We must play the effect immediately, as this actor is being destroyed
			PlayAbilityEffects(AbilityFx);
		}
		
	}
	
	// Apply effects/damages to the character that was hit (Server Only)
	if (IsValid(HitActor) && HasAuthority())
	{
		ACharacterBase* HitInstigator = nullptr;
			
		if (IsValid(GetOwner()))
			HitInstigator = Cast<ACharacterBase>(GetOwner());

		ACharacterBase* HitCharacter = Cast<ACharacterBase>(HitActor);
		if (IsValid(HitCharacter))
		{
			// Apply effect to target actor
			HitCharacter->AbilityComponent->ApplyEffect(HitInstigator, _AbilityName);

			// Apply damages
			HitCharacter->VitalityComponent->ModifyVitalityStat(EVitalityCategories::HEALTH,	SpellData.ConsumeHealth);
			HitCharacter->VitalityComponent->ModifyVitalityStat(EVitalityCategories::MAGIC,		SpellData.ConsumeMagic);
			HitCharacter->VitalityComponent->ModifyVitalityStat(EVitalityCategories::STAMINA,	SpellData.ConsumeStamina);
		}
	}
	
	Super::ApplyHitEffect(HitActor, HitVector);
}


/**************************************
 *			REPLICATION & NETWORKING
 */

void ASpellProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASpellProjectileBase, _AbilityName);
}