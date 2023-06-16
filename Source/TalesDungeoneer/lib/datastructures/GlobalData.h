#pragma once

#include "CoreMinimal.h"

#include "GlobalData.generated.h"

UCLASS()
class UGlobalData : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "GetAppVersion"), Category = "Game Config")
		static FString GetAppVersion();
    
};
