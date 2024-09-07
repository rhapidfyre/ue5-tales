// Take Five Games, LLC

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Gas/Abilities/TalesGameplayAbility.h"

#include "AbilityDataAsset.generated.h"


class UGameplayAbility;

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API UPrimaryAbilityDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:

	// Methods

	UPrimaryAbilityDataAsset();

	UFUNCTION(BlueprintPure)
	UTalesGameplayAbility* GetAbilityReference() const;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Specifications")
	TSubclassOf<UTalesGameplayAbility> AbilityReference;
};
