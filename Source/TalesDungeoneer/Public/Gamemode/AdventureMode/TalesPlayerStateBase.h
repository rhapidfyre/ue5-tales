// Starcache Studios, LLC (c) 2024

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerState.h"
#include "Delegates/Delegate.h"

#include "TalesPlayerStateBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerNameUpdated);


UCLASS(BlueprintType, Blueprintable)
class TALESDUNGEONEER_API ATalesPlayerStateBase : public APlayerState
{
	GENERATED_BODY()

public:

	ATalesPlayerStateBase();

	UPROPERTY(BlueprintAssignable) FOnPlayerNameUpdated OnPlayerNameUpdated;

	// Called whenever the player's name is changed to update all delegates
	UFUNCTION(BlueprintCallable) void UpdatePlayerName();

	// Returns the data asset for the current character class.
	// Check for nullptr before accessing result
	UFUNCTION(BlueprintPure) UDataAsset* GetClassDataAsset(const FGameplayTag& ClassTag) const;
	UFUNCTION(BlueprintPure) TArray<FGameplayTag> GetAllCharacterClasses() const;

	// Returns the data asset for the current character race.
	// Check for nullptr before accessing result
	UFUNCTION(BlueprintPure) UDataAsset* GetRaceDataAsset(const FGameplayTag& RaceTag) const;
	UFUNCTION(BlueprintPure) TArray<FGameplayTag> GetAllCharacterRaces() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<FGameplayTag, UDataAsset*> RaceTagsMapped  = {};
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<FGameplayTag, UDataAsset*> ClassTagsMapped = {};

};
