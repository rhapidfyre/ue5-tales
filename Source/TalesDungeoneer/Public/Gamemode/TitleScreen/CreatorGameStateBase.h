// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Gamemode/BaseFiles/TalesGameStateBase.h"
#include "Delegates/Delegate.h"

#include "CreatorGameStateBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterCreated, FName, SaveSlotName);


UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ACreatorGameStateBase : public ATalesGameStateBase
{
	GENERATED_BODY()

public:
	
	ACreatorGameStateBase();
	
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintAssignable) FOnCharacterCreated OnCharacterCreated;
	
	UFUNCTION(BlueprintCallable)
	bool CreateNewCharacter(FString& SaveResponse, bool RunAsync = false);
	
};
