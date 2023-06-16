
#include "AbilityComponent.h"
#include "../CharacterBase.h"
#include "TalesDungeoneer/Entities/AbilityEffectBase.h"


UAbilityComponent::UAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}


void UAbilityComponent::ActivateAbility(const FName AbilityName)
{
	if (GetOwner()->HasAuthority())
	{
		const ACharacterBase* Instigator = Cast<ACharacterBase>(GetOwner());
		FTransform SpawnTransform(Instigator->GetActorTransform());
		SpawnTransform.SetScale3D(FVector(1.f));

		// Spawns the ability effect actor
		AAbilityEffectBase* AbilityEffect = GetWorld()->SpawnActorDeferred<AAbilityEffectBase>(
			AAbilityEffectBase::StaticClass(), SpawnTransform);
		
		if (IsValid(AbilityEffect))
		{
			if (bShowDebug)
			{
				UE_LOG(LogTemp, Display, TEXT("%s(%s): Ability Actor Spawned"),
					*GetName(), GetOwner()->HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"))
			}
			
			AbilityEffect->SetOwner( GetOwner() );
			AbilityEffect->Set
			if (AbilityEffect)

			
			OnAbilityActivated.Broadcast(AbilityEffect); // Trigger Delegates
		}
	}
	else
	{
		Server_RequestAbility(AbilityName);
	}
}

void UAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

#ifdef UE_BUILD_DEBUG
	bShowDebug = true;
#endif
	
}

void UAbilityComponent::OnComponentCreated()
{
	Super::OnComponentCreated();
	
	SetAutoActivate(true);
	SetIsReplicated(true);
	
	RegisterComponent();
}

void UAbilityComponent::Client_AbilityAdded_Implementation(AAbilityEffectBase* NewEffect)
{
	// Trigger Delegates, such as the HUD
	OnAbilityActivated.Broadcast(NewEffect);
	NewEffect->OnEffectExpired.AddDynamic(this, &UAbilityComponent::AbilityExpired);
}

void UAbilityComponent::Server_RequestAbility_Implementation(FName AbilityName)
{
	if (GetOwner()->HasAuthority())
		ActivateAbility(AbilityName);
}

//-------------------------------- REPLICATION
void UAbilityComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}