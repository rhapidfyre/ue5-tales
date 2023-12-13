// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AiController.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AIPerceptionComponent.h"

#include "AiControllerBase.generated.h"


UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API AAiControllerBase : public AAIController
{
	GENERATED_BODY()

public:

	AAiControllerBase();

	UFUNCTION(BlueprintPure) bool GetAiShouldPatrol() const { return _PatrolArea; }

protected:
	
	virtual void BeginPlay() override;

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void PostInitializeComponents() override;

	virtual void Tick(float DeltaSeconds) override;

public:
	
	// Sets the behavior tree this Ai Controller will use
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UBehaviorTree* BehaviorTree = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	UAIPerceptionComponent* AiPerception = nullptr;;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ai Settings")
	float GainSightRadius = 1024.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ai Settings")
	float LoseSightRadius = 2048.f;

private:

	bool _PatrolArea = false;
	
	
};
