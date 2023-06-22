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
	
	RootMesh->SetNotifyRigidBodyCollision(true);
	
	SphereCollision->SetHiddenInGame(true);
	SphereCollision->SetVisibility(false);
	SphereCollision->SetGenerateOverlapEvents(true);
	SphereCollision->SetCollisionProfileName(FName("Projectile"));
	SphereCollision->SetCollisionResponseToAllChannels(ECR_Overlap);
	SphereCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereCollision->Deactivate();

	_OriginLocation = GetActorLocation();
	
	if (!IsValid(UsingMesh))
	{
		UsingMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr,
			TEXT("StaticMesh'/Game/TalesContent/Meshes/GeoShapes/Sphere.Sphere'")));
	}
	
	if (IsValid(UsingMesh))
	{
		const bool IsAuthority = HasAuthority();
		RootMesh->SetStaticMesh(UsingMesh);
		RootMesh->SetCollisionObjectType(ECC_WorldDynamic);
		RootMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
		RootMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		RootMesh->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Overlap);
		RootMesh->SetCollisionResponseToChannel(ECC_Destructible, ECR_Overlap);
		RootMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		RootMesh->SetSimulatePhysics(false);
	}
	
}

void AProjectileBase::SetGravityConsidered(bool AllowGravity)
{
	_GravityScale = AllowGravity ? 1.f : 0.f;
}

void AProjectileBase::SetProjectileScale(float newScale)
{
	UE_LOG(LogTemp, Warning, TEXT("%s(%s): NOT YET IMPLEMENTED"), *GetName(),
		HasAuthority()?TEXT("SERVER"):TEXT("CLIENT"));
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
	Super::BeginPlay();

	RootMesh->SetEnableGravity(_GravityScale > 0.f);
	ProjectileMovement->ProjectileGravityScale = _GravityScale;
	ProjectileMovement->Velocity = _SpeedVector;
	
	RootMesh->OnComponentHit.AddDynamic(this, &AProjectileBase::CheckCollision);
	RootMesh->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::CheckOverlap);
	//SphereCollision->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::CheckOverlap);
	
	ProjectileMovement->Activate(true);
	//SphereCollision->Activate(true);
}

void AProjectileBase::Destroyed()
{
	Super::Destroyed();
}

void AProjectileBase::SetupDefaults()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	
	SetActorEnableCollision(true);
	SetActorTickEnabled(true);

	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Root"));
	SetRootComponent(RootMesh);
	
	SphereCollision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	SphereCollision->InitSphereRadius(40.f);
	SphereCollision->SetupAttachment(RootMesh);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	ProjectileMovement->Deactivate();
}

// Called every frame
void AProjectileBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	const float travelDistance = GetMaxTravelDistance();
	if (travelDistance > 0.f)
	{
		const FVector OriginLoc = GetOriginLocation();
		const FVector ActorLoc = GetActorLocation();
		const float TravelLength = (OriginLoc-ActorLoc).Length();
		if (TravelLength > travelDistance)
		{
			SetOwner(nullptr);
			Destroy();
		}
	}
}

void AProjectileBase::CheckCollision(UPrimitiveComponent* HitComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	const FString ServerOrClientText = HasAuthority()?TEXT("SERVER"):TEXT("CLIENT");
	UE_LOG(LogTemp, Display, TEXT("%s(%s): Evaluating Collision with actor '%s'"),
		*GetName(), *ServerOrClientText, *OtherActor->GetName());
	if (IsValid(OtherActor))
	{
		ACharacterBase* InstigatingActor = Cast<ACharacterBase>(GetInstigator()); //GetInstigatingActor();
		if (OtherActor != InstigatingActor && OtherActor != this)
		{
			// If an actor is targeted, target that actor specifically
			// Otherwise, target the actor that was hit by the collision
			ACharacterBase* HitCharacter = Cast<ACharacterBase>(GetTargetActor());
			if (!IsValid(HitCharacter))
			{
				const FVector HitVector = GetActorLocation();
				ApplyHitEffect(OtherActor, HitVector);
				Destroy();
			}
			else
			{
				if (IsValid(HitCharacter))
				{
					if (HitCharacter != GetInstigatingActor())
					{
						const FVector HitVector = HitCharacter->GetActorLocation();
						ApplyHitEffect(HitCharacter, HitVector);
						Destroy();
					}
				}
			}
		}
	}
}

void AProjectileBase::CheckOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const FString ServerOrClientText = HasAuthority()?TEXT("SERVER"):TEXT("CLIENT");
	UE_LOG(LogTemp, Display, TEXT("%s(%s): Evaluating Overlap (%s) with actor '%s'"),
		*GetName(), *ServerOrClientText, *this->GetName(), *OtherActor->GetName());
	if (IsValid(OtherActor))
	{
		ACharacterBase* InstigatingActor = Cast<ACharacterBase>(GetInstigator()); //GetInstigatingActor();
		if (OtherActor != InstigatingActor && OtherActor != this)
		{
			// If an actor is targeted, target that actor specifically
			// Otherwise, target the actor that was hit by the collision
			ACharacterBase* HitCharacter = Cast<ACharacterBase>(GetTargetActor());
			if (!IsValid(HitCharacter))
			{
				const FVector HitVector = GetActorLocation();
				ApplyHitEffect(OtherActor, HitVector);
				Destroy();
			}
			else
			{
				if (IsValid(HitCharacter))
				{
					if (HitCharacter != GetInstigatingActor())
					{
						const FVector HitVector = HitCharacter->GetActorLocation();
						ApplyHitEffect(HitCharacter, HitVector);
						Destroy();
					}
				}
			}
		}
	}
}

void AProjectileBase::ApplyHitEffect(AActor* HitActor, FVector HitVector)
{
	// Does Nothing, implemented by children
}


/**************************************
 *			REPLICATION & NETWORKING
 */

void AProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AProjectileBase, _GravityScale);
	DOREPLIFETIME(AProjectileBase, _SpeedVector);
	DOREPLIFETIME(AProjectileBase, _InstigatingActor);
	DOREPLIFETIME(AProjectileBase, _TargetActor);
	DOREPLIFETIME(AProjectileBase, _MaxTravelDistance);
	DOREPLIFETIME(AProjectileBase, _GravityScale);
	DOREPLIFETIME(AProjectileBase, _SpeedVector);
	DOREPLIFETIME(AProjectileBase, _OriginLocation);
}
