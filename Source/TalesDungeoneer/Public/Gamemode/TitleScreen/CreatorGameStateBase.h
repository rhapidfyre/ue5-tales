// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"

#include "Delegates/Delegate.h"
#include "Gamemode/BaseFiles/TalesGameStateBase.h"

#include "CreatorGameStateBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterCreated, FName, SaveSlotName);


UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ACreatorGameStateBase : public ATalesGameStateBase
{
	GENERATED_BODY()

public:
	
	ACreatorGameStateBase();
	
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void SetIsCreatingCharacter(bool isCreating = true);

	UPROPERTY(BlueprintAssignable) FOnCharacterCreated OnCharacterCreated;
	
	UFUNCTION(BlueprintCallable)
	bool CreateNewCharacter(FString& SaveResponse, bool RunAsync = false);

	virtual bool SaveCharacterSync(USaveGame*& SaveGame) override;
	
	UFUNCTION(BlueprintPure)
	bool GetIsCreatingCharacter() const { return bIsCreating; }

private:
	
	UFUNCTION(Client, Reliable)
	void OnRep_IsCreating();

	UPROPERTY(ReplicatedUsing=OnRep_IsCreating)
	bool bIsCreating = false;
	
};
