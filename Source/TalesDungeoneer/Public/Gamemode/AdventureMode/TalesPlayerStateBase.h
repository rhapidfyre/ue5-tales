// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Delegates/Delegate.h"

#include "TalesPlayerStateBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerNameUpdated);


UCLASS(BlueprintType, Blueprintable)
class TALESDUNGEONEER_API ATalesPlayerStateBase : public APlayerState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable) FOnPlayerNameUpdated OnPlayerNameUpdated;
	
	// Called whenever the player's name is changed to update all delegates
	UFUNCTION(BlueprintCallable) void UpdatePlayerName();
	
};
