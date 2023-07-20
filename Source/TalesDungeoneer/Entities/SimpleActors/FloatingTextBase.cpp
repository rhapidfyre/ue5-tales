// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "FloatingTextBase.h"

#include "TalesDungeoneer/Widgets/FloatingTextWidgetBase.h"


AFloatingTextBase::AFloatingTextBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>("Arrow");
	SetRootComponent(ArrowComponent);

	WidgetComponent = CreateDefaultSubobject<UWidgetComponent>("WidgetComponent");
	WidgetComponent->SetupAttachment(ArrowComponent);
	WidgetComponent->SetWidgetClass(UFloatingTextWidgetBase::StaticClass());
}


void AFloatingTextBase::BeginPlay()
{
	Super::BeginPlay();
}

void AFloatingTextBase::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}


void AFloatingTextBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFloatingTextBase::UpdateFloatingText()
{
	UFloatingTextWidgetBase* WidgetBase = Cast<UFloatingTextWidgetBase>(WidgetComponent->GetWidget());
	if (IsValid(WidgetBase))
	{
		WidgetBase->TextColor = this->TextColor;
		WidgetBase->TextShown = this->TextShown;
		WidgetBase->UpdateWidgetText();
	}
	SetLifeSpan(SecondsToShow > 0 ? SecondsToShow : 3.f);
}

