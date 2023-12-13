// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "OverheadWidgetComponent.h"


// Sets default values for this component's properties
UOverheadWidgetComponent::UOverheadWidgetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UOverheadWidgetComponent::SetOwningCharacter_Implementation(ACharacterBase* CharacterBase)
{
	_OwningCharacter = CharacterBase;
}