
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
	FTransform ActorTransform = GetActorTransform();
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
					AbilityEffect.NiagaraEffect, ActorTransform.GetLocation(), ActorTransform.GetRotation().Rotator(),
					FVector(AbilityEffect.EffectScale),true, true);
		}
	}
	if (IsValid(AbilityEffect.SoundEffect))
	{
		UAudioComponent* ImpactSound = UGameplayStatics::SpawnSoundAtLocation(GetWorld(),
			AbilityEffect.SoundEffect, ActorTransform.GetLocation(),
			FRotator::ZeroRotator, AbilityEffect.SoundVolume);	
		if (IsValid(ImpactSound))
		{
			ImpactSound->bAutoDestroy = true;
			if (!AbilityEffect.SoundEffect->IsOneShot())
				LoopingSoundEmitters.Add(ImpactSound);
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
	
	//TODO - Need to add a delay to this
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
	SetupDefaults();
}

void ASpellProjectileBase::SetProjectileData(FName AbilityName)
{
	_AbilityName = AbilityName;
}

void ASpellProjectileBase::BeginPlay()
{
	SetFromAbilityData();
	
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
	_AbilityData = UAbilitySystem::GetAbilityDataFromName(_AbilityName);
	_SpellData   = UAbilitySystem::GetSpellDataFromName(_AbilityName);
	
	SetMaxTravelDistance(_AbilityData.MaxReach);
	
	if (_SpellData.ImpactData.Num() > 0)
	{
		SetProjectileScale(1.f);//_SpellData.ImpactData[0].ProjectileScale);
		SetProjectileSpeed( FVector(_SpellData.ImpactData[0].ProjectileSpeed) * GetActorForwardVector());
		SetGravityConsidered( _SpellData.ImpactData[0].bUseGravity );
	}
}

void ASpellProjectileBase::ApplyHitEffect(AActor* HitActor, FVector HitVector)
{
	// Create the effects actor (Client & Server)
	for (FStProjectileData ProjectileData : _SpellData.ImpactData)
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
			HitCharacter->VitalityComponent->ModifyVitalityStat(EVitalityCategories::HEALTH,  _SpellData.ConsumeHealth);
			HitCharacter->VitalityComponent->ModifyVitalityStat(EVitalityCategories::MAGIC,	  _SpellData.ConsumeMagic);
			HitCharacter->VitalityComponent->ModifyVitalityStat(EVitalityCategories::STAMINA, _SpellData.ConsumeStamina);
		}
	}
}


/**************************************
 *			REPLICATION & NETWORKING
 */

void ASpellProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASpellProjectileBase, _AbilityName);
}