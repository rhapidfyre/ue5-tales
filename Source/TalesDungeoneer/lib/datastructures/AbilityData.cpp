// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilityData.h"

UDataTable* UAbilitySystem::GetAbilityDataTable()
{
	const FSoftObjectPath itemTable = FSoftObjectPath(
		"/Game/TalesContent/DataTables/DT_AbilityData.DT_AbilityData");
	UDataTable* dataTable = Cast<UDataTable>(itemTable.ResolveObject());
	if (IsValid(dataTable)) return dataTable;
	return Cast<UDataTable>(itemTable.TryLoad());
}

FStAbilityData UAbilitySystem::GetAbilityDataFromName(FName AbilityName)
{
	if (!AbilityName.IsNone())
	{
		if ( const UDataTable* dt = GetAbilityDataTable() )
		{
			if (IsValid(dt))
			{
				const FString ErrorCaught;
				FStAbilityData* abilityPtr = dt->FindRow<FStAbilityData>(AbilityName, ErrorCaught);

				if (abilityPtr != nullptr)
					return *abilityPtr;
				
			}
		}
	}
	// Return a default item data struct
	return FStAbilityData();
}

bool UAbilitySystem::GetAbilityNameIsValid(FName AbilityName)
{
	if (!AbilityName.IsNone())
	{
		const FStAbilityData itemData = (GetAbilityDataFromName(AbilityName));
		return !itemData.DisplayName.IsEmpty();
	}
	return false; // Invalid or not found
}

bool UAbilitySystem::GetAbilityDataIsValid(FStAbilityData AbilityData)
{
	return !AbilityData.DisplayName.IsEmpty();
}
