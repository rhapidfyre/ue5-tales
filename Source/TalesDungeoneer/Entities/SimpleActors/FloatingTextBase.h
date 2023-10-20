// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Pawn.h"
#include "TalesDungeoneer/Widgets/FloatingTextWidgetBase.h"
#include "FloatingTextBase.generated.h"

class UFloatingTextWidgetBase;
UCLASS(BlueprintType, Blueprintable)
class TALESDUNGEONEER_API AFloatingTextBase : public APawn
{
	GENERATED_BODY()

public:

	AFloatingTextBase();
	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable) void UpdateFloatingText();

protected:

	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString TextShown = "EmptyString";
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateColor TextColor = FLinearColor::White;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SecondsToShow = 3.f;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UWidgetComponent* WidgetComponent = nullptr;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UArrowComponent* ArrowComponent = nullptr;
};
