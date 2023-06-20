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

UDataTable* UAbilitySystem::GetSpellDataTable()
{
	const FSoftObjectPath itemTable = FSoftObjectPath(
		"/Game/TalesContent/DataTables/DT_SpellData.DT_SpellData");
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

FStSpellData UAbilitySystem::GetSpellDataFromName(FName SpellName)
{
	if (!SpellName.IsNone())
	{
		if ( const UDataTable* dt = GetSpellDataTable() )
		{
			if (IsValid(dt))
			{
				const FString ErrorCaught;
				FStSpellData* spellPtr = dt->FindRow<FStSpellData>(SpellName, ErrorCaught);

				if (spellPtr != nullptr)
					return *spellPtr;
				
			}
		}
	}
	// Return a default item data struct
	return FStSpellData();
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

bool UAbilitySystem::GetSpellNameIsValid(FName SpellName)
{
	if (!SpellName.IsNone())
	{
		const FStSpellData SpellData = (GetSpellDataFromName(SpellName));
		return SpellData.AllowedClass.Num() > 0;
	}
	return false; // Invalid or not found
}

bool UAbilitySystem::GetAbilityDataIsValid(FStAbilityData AbilityData)
{
	return !AbilityData.DisplayName.IsEmpty();
}
