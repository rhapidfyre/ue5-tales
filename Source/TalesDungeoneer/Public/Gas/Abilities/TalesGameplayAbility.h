// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TalesDungeoneer/TalesDungeoneer.h"

#include "TalesGameplayAbility.generated.h"




/**
 *
 */
UCLASS()
class TALESDUNGEONEER_API UTalesGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:

	UTalesGameplayAbility();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Specifications")
	FText DisplayName;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Specifications")
	FString AbilityDetails;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Specifications")
	TSoftObjectPtr<UTexture2D> AbilityIcon;

	// The time it takes (seconds) for the ability to cool off and be available again
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Specifications")
	float TimeToCooldown;

	// The time it takes (seconds) for this ability to fire once activated (casting time)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Settings")
	float TimeToCast;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Settings")
	FGameplayTag AbilityCategory;

	// All races that are allowed to use this ability
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Settings")
	FGameplayTagContainer AbilityRaces;

	// The tag for which class(es) can use this ability, and the minimum level to acquire it
	// Classes not in the array are unable to use this ability
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Settings")
	TMap<FGameplayTag, int> AbilityClasses;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Settings")
	EAbilityInputID AbilityInputID;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Effects")
	TArray< TSoftObjectPtr<UGameplayEffect> > EffectsSuccessSelf;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Effects")
	TArray< TSoftObjectPtr<UGameplayEffect> > EffectsActivatedSelf;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Effects")
	TArray< TSoftObjectPtr<UGameplayEffect> > EffectsFailedSelf;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Effects")
	TArray< TSoftObjectPtr<UGameplayEffect> > EffectsSuccessTarget;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Effects")
	TArray< TSoftObjectPtr<UGameplayEffect> > EffectsActivatedTarget;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability Effects")
	TArray< TSoftObjectPtr<UGameplayEffect> > EffectsFailedTarget;

};
