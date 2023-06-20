// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"
#include "NiagaraComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "TalesDungeoneer/Characters/CharacterBase.h"

#include "TalesDungeoneer/lib/datastructures/AbilityData.h"


// Sets default values
AProjectileBase::AProjectileBase()
{
	SetupDefaults();
}

AProjectileBase::AProjectileBase(FName AbilityName)
{
	_AbilityName = AbilityName;
	SetupDefaults();
	SetFromAbilityData();
}

void AProjectileBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ProjectileMovement->Velocity = FVector(0.f);
	ProjectileMovement->ProjectileGravityScale = 1.f;
}

void AProjectileBase::SetProjectileData(FName AbilityName)
{
	_AbilityName = AbilityName;
	SetFromAbilityData();
}

void AProjectileBase::SetGravityConsidered(bool AllowGravity)
{
	_GravityScale = AllowGravity ? 1.f : 0.f;
}

void AProjectileBase::SetProjectileScale(float newScale)
{
	UE_LOG(LogTemp, Warning, TEXT("%s: NOT YET IMPLEMENTED"), *GetName());
}

void AProjectileBase::SetProjectileSpeed(FVector SpeedVector)
{
	_SpeedVector = SpeedVector;
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		SphereCollision->OnComponentHit.AddDynamic(this, &AProjectileBase::CheckCollision);
		SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::CheckOverlap);
		
		_OriginLocation = GetActorLocation();
		
		ProjectileMovement->ProjectileGravityScale = _GravityScale;
		ProjectileMovement->Velocity = _SpeedVector;
		ProjectileMovement->Activate(true);
		
	}
	
	
}

void AProjectileBase::Destroyed()
{
	ApplyHitEffect(nullptr, GetActorLocation());
	Super::Destroyed();
}

void AProjectileBase::SetupDefaults()
{
	
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = HasAuthority();
	bReplicates = true;

	SetActorTickEnabled( HasAuthority() );
	
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SetRootComponent(SphereCollision);
	SphereCollision->InitSphereRadius(40.f);
	SphereCollision->SetIsReplicated(true);
	
	Arrow = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
	Arrow->SetHiddenInGame(false);
	Arrow->SetupAttachment(GetRootComponent());
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	ProjectileMovement->SetIsReplicated(true);
	ProjectileMovement->Deactivate();

	// Set Collision Responses
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	FCollisionResponseContainer CollResponse;

	// Check for interception hit
	CollResponse.SetResponse(ECC_Pawn, ECR_Overlap);
	CollResponse.SetResponse(ECC_Destructible, ECR_Overlap);

	// Check for extermination
	CollResponse.SetResponse(ECC_Visibility, ECR_Overlap);
	CollResponse.SetResponse(ECC_WorldDynamic, ECR_Overlap);
	CollResponse.SetResponse(ECC_WorldStatic, ECR_Overlap);
	
	SphereCollision->SetCollisionResponseToChannels(CollResponse);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AProjectileBase::SetFromAbilityData()
{
	FStAbilityData AbilityData = UAbilitySystem::GetAbilityDataFromName(_AbilityName);
	if (UAbilitySystem::GetAbilityDataIsValid(AbilityData))
	{
		_AbilityData = AbilityData;
	}
}

// Called every frame
void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HasAuthority())
	{
		if ((_OriginLocation - GetActorLocation()).Length() > _AbilityData.MaxReach)
		{
			UE_LOG(LogTemp, Display, TEXT("%s(%s): Max Reach Achieved. Destroying."),
				*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
			SetOwner(nullptr);
			SetActorTickEnabled(false);
			Destroy();
		}
	}
}

void AProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AProjectileBase, _GravityScale);
	DOREPLIFETIME(AProjectileBase, _SpeedVector);
	DOREPLIFETIME(AProjectileBase, _AbilityName);
}



void AProjectileBase::CheckCollision(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	UE_LOG(LogTemp, Display, TEXT("%s(%s): Evaluating Collision Hit with actor '%s'"),
		*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *OtherActor->GetName());
	if (IsValid(OtherActor))
	{

		// If an actor is targeted, target that actor specifically
		// Otherwise, target the actor that was hit by the collision
		ACharacterBase* HitCharacter = Cast<ACharacterBase>(GetTargetActor());
		if (!IsValid(HitCharacter))
			HitCharacter = Cast<ACharacterBase>(Hit.GetActor());
		
		ApplyHitEffect(HitCharacter, Hit.ImpactPoint);
		
	}
}

void AProjectileBase::CheckOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Display, TEXT("%s(%s): Evaluating Overlap with actor '%s'"),
		*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"), *OtherActor->GetName());
	ACharacterBase* HitActor = Cast<ACharacterBase>(OtherActor);
	if (IsValid(HitActor))
	{
		const FVector HitVector = SweepResult.ImpactPoint;

		// If an actor is targeted, target that actor specifically
		// Otherwise, target the actor that was hit by the collision
		ACharacterBase* HitCharacter = Cast<ACharacterBase>(GetTargetActor());
		if (!IsValid(HitCharacter))
			HitCharacter = Cast<ACharacterBase>(SweepResult.GetActor());
		
		ApplyHitEffect(HitCharacter, SweepResult.ImpactPoint);
	}
}

void AProjectileBase::Multicast_ImpactEffects_Implementation(FVector HitVector)
{
	const FStSpellData SpellData = UAbilitySystem::GetSpellDataFromName(_AbilityName);
	
	// Use Niagara Effect
	if (IsValid(SpellData.ImpactData.NiagaraEffect))
	{
		UNiagaraComponent* NiagaraEmitter = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),
			SpellData.ImpactData.NiagaraEffect, HitVector, FRotator::ZeroRotator,
			FVector(1.f),true,true);
		if (NiagaraEmitter != nullptr)
		{
			NiagaraEmitter->SetIsReplicated(true);
		}
	}

	// Otherwise, use cascade system
	else if (IsValid(SpellData.ImpactData.CascadeEffect))
	{
		UParticleSystemComponent* ParticleEmitter = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),
			SpellData.ImpactData.CascadeEffect, HitVector, FRotator::ZeroRotator);
		if (IsValid(ParticleEmitter))
		{
			ParticleEmitter->SetIsReplicated(true);
		}
	}
}

void AProjectileBase::ApplyHitEffect(ACharacterBase* HitCharacter, FVector HitVector)
{
	const FStSpellData SpellData		= UAbilitySystem::GetSpellDataFromName(_AbilityName);
	const FStAbilityData AbilityData	= UAbilitySystem::GetAbilityDataFromName(_AbilityName);
	
	Multicast_ImpactEffects(HitVector);
		
	// Apply effects/damages to the character that was hit
	if (IsValid(HitCharacter))
	{
		ACharacterBase* HitInstigator = nullptr;
			
		if (IsValid(GetOwner()))
			Cast<ACharacterBase>(GetOwner());

		// Apply effect to target actor
		HitCharacter->AbilityComponent->ApplyEffect(HitInstigator, _AbilityName);

		// Apply damages
		HitCharacter->VitalityComponent->ModifyVitalityStat(EVitalityCategories::HEALTH,	SpellData.ConsumeHealth);
		HitCharacter->VitalityComponent->ModifyVitalityStat(EVitalityCategories::MAGIC,		SpellData.ConsumeMagic);
		HitCharacter->VitalityComponent->ModifyVitalityStat(EVitalityCategories::STAMINA,	SpellData.ConsumeStamina);
	}
}

void AProjectileBase::OnRep_AbilityName_Implementation()
{
	FStSpellData SpellData = UAbilitySystem::GetSpellDataFromName(_AbilityName);
	if (IsValid(SpellData.VisualEffects.NiagaraEffect))
	{
		UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
				SpellData.VisualEffects.NiagaraEffect, GetRootComponent(),
				FName("root"),FVector(0.f),FRotator(0.f),
				EAttachLocation::SnapToTargetIncludingScale,true);
	}
}
