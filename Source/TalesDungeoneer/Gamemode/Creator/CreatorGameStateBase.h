// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameFramework/SaveGame.h"
#include "Delegates/Delegate.h"

#include "CreatorGameStateBase.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveGameObjectReady,
	USaveGame*, SaveObject);


UCLASS(Blueprintable, BlueprintType)
class TALESDUNGEONEER_API ACreatorGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	
	ACreatorGameState();

	// Called when an async request to 'LoadSaveGameMeta' successfully
	// retrieves a save game object.
	UPROPERTY(VisibleAnywhere, BlueprintAssignable)
	FOnSaveGameObjectReady OnSaveGameObjectReady;

	// Called whenever save data has been modified
	UFUNCTION(BlueprintCallable) bool SaveTheGame();

	UFUNCTION(BlueprintCallable)
	void SaveTheGameAsync();
	
	UFUNCTION(BlueprintCallable)
	void SetSaveGameMetaName(FString SaveSlotName);

	UFUNCTION(BlueprintCallable)
	FString GetSaveGameMetaName() const { return _SaveMetaName; };

	// Retrieves a USaveGame object with the current meta
	// Use 'SetSaveGameMetaName' to change which object is pulled.
	UFUNCTION(BlueprintCallable) USaveGame* GetSaveGameMeta();

	// Performs an async load of the save game
	void LoadSaveGameMeta(FString SaveSlotName);
	
protected:
	
	virtual void BeginPlay() override;

	// Called when LoadSaveGameMeta (async) successfully executes
	void SaveGameMetaLoaded(const FString& SlotName,
			const int32 UserIndex, USaveGame* LoadedGameData);

private:

	UFUNCTION()	void SaveGameDelegate(const FString& SlotName,
		const int32 UserIndex, bool bSuccess);

	bool CreateSaveGameIfNotExists();

	UPROPERTY() FString _SaveMetaName;
	
};
