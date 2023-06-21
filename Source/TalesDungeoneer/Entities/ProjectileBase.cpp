// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"
#include "Net/UnrealNetwork.h"

#include "TalesDungeoneer/Characters/CharacterBase.h"


// Sets default values
AProjectileBase::AProjectileBase()
{
	SetupDefaults();
}

void AProjectileBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ProjectileMovement->Velocity = FVector(0.f);
	ProjectileMovement->ProjectileGravityScale = 1.f;
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

void AProjectileBase::SetMaxTravelDistance(float MaxTravelDistance)
{
	_MaxTravelDistance = MaxTravelDistance > 0.f ? MaxTravelDistance : 0.f;
}

// Called when the game starts or when spawned
void AProjectileBase::BeginPlay()
{
	UE_LOG(LogTemp, Display, TEXT("%s(%s): BeginPlay()"),
		*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
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
	UE_LOG(LogTemp, Display, TEXT("%s(%s): Destroyed()"),
		*GetName(), HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
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

void AProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AProjectileBase, _GravityScale);
	DOREPLIFETIME(AProjectileBase, _SpeedVector);
}

// Called every frame
void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (HasAuthority())
	{
		const float travelDistance = GetMaxTravelDistance();
		if (travelDistance > 0.f)
		{
			if ((GetOriginLocation() - GetActorLocation()).Length() > travelDistance)
			{
				SetOwner(nullptr);
				SetActorTickEnabled(false);
				Destroy();
			}
		}
	}
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
		{
			HitCharacter = Cast<ACharacterBase>(Hit.GetActor());
		}
		const FVector HitVector = HitCharacter->GetActorLocation();
		ApplyHitEffect(HitCharacter, HitVector);
		
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
		// If an actor is targeted, target that actor specifically
		// Otherwise, target the actor that was hit by the collision
		ACharacterBase* HitCharacter = Cast<ACharacterBase>(GetTargetActor());
		if (!IsValid(HitCharacter))
		{
			HitCharacter = Cast<ACharacterBase>(SweepResult.GetActor());
		}
		const FVector HitVector = HitCharacter->GetActorLocation();
		ApplyHitEffect(HitCharacter, HitVector);
	}
}

void AProjectileBase::ApplyHitEffect(AActor* HitActor, FVector HitVector) {}
