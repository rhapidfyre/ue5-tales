// Starcache Studios, LLC (c) 2024

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "lib/Tags/TalesGlobalTags.h"
#include "TalesRespawnBase.generated.h"


UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ATalesRespawnBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATalesRespawnBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Graveyard = Town Respawn
	// Entrance = Dungeon Entrance
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Respawn Settings")
	FGameplayTag RespawnTag = TAG_Respawners_Graveyard.GetTag();
};
