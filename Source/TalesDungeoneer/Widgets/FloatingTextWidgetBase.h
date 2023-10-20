// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "UObject/Object.h"
#include "FloatingTextWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class TALESDUNGEONEER_API UFloatingTextWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TextShown = "EmptyString";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateColor TextColor = FLinearColor::White;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UWidgetAnimation* SpawnAnimation = nullptr;

	virtual void UpdateWidgetText();
	
	UFUNCTION(BlueprintNativeEvent) void UpdateWidget();

protected:

	virtual void NativeConstruct() override;

	virtual void NativeOnInitialized() override;
	
};
