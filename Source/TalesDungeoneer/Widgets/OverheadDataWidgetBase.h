// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UObject/Object.h"
#include "OverheadDataWidgetBase.generated.h"

/**
 * 
 */
UCLASS()
class TALESDUNGEONEER_API UOverheadDataWidgetBase : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UFUNCTION(BlueprintNativeEvent)
	void SetOwningCharacter(ACharacterBase* CharacterBase);
	
	UFUNCTION(BlueprintPure) ACharacterBase* GetOwningCharacter() const
		{ return _OwningCharacter; }

private:

	UPROPERTY() ACharacterBase* _OwningCharacter = nullptr;
	
};