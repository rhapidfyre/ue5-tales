// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "OverheadWidgetComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TALESDUNGEONEER_API UOverheadWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:

	UOverheadWidgetComponent();

	UFUNCTION(BlueprintNativeEvent)
	void SetOwningCharacter(ACharacterBase* CharacterBase);

	UFUNCTION(BlueprintPure)
	ACharacterBase* GetOwningCharacter(ACharacterBase* CharacterBase) const
		{ return _OwningCharacter; }

private:

	UPROPERTY() ACharacterBase* _OwningCharacter = nullptr;
};
