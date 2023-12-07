// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"

#include "TalesHudBase.generated.h"

USTRUCT(Blueprintable, BlueprintType)
struct FStHudNotification
{
	GENERATED_BODY();
	FStHudNotification() {};
	FStHudNotification(FString tMessage) { Message = tMessage; }
	FStHudNotification(FString tMessageTitle, FString tNewMessage, int tPriority = 2)
	{
		Title = tMessageTitle; Message = tNewMessage; Priority = tPriority;
	};
	FString Title	= "";
	FString Message = "No Message Given";
	int Priority	= 2;
};


UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ATalesHudBase : public AHUD
{
	GENERATED_BODY()
	
public:
	ATalesHudBase();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void NotifyHud(FString MessageTitle, FString NewMessage, int Priority = 2);

private:

	UPROPERTY()
	TArray<FStHudNotification> NotificationsPending_ = {};
};
