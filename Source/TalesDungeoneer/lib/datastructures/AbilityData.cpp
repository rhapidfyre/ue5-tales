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

TArray<FStAbilityData> UAbilitySystem::GetAbilityDataTableSortedByClass(ECharacterClass CharacterClass)
{
	const UDataTable* AbilityTable = GetAbilityDataTable();
	TArray<FName> UnsortedArray = AbilityTable->GetRowNames();
	
	TArray<FStAbilityData> SortedArray;
	for (int i = 0; i < UnsortedArray.Num(); i++)
	{
		FStAbilityData AbilityData = GetAbilityDataFromName(UnsortedArray[i]);
		if (AbilityData.AllowedClasses.Contains(CharacterClass))
			SortedArray.Add(AbilityData);
	}

	// Uses UEs built in MergeSort algorithm
	SortedArray.StableSort(
		[CharacterClass](const FStAbilityData& AbilityA, const FStAbilityData& AbilityB)
		{
			return AbilityA.AllowedClasses[CharacterClass] < AbilityB.AllowedClasses[CharacterClass];
		}
	);
	
	return SortedArray;
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
				{
					abilityPtr->GameName = AbilityName;
					return *abilityPtr;
				}
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
				{
					// AbilityData must be used to retrieve struct with a set GameName value
					// Calling GetAbilityDataFromName().GameName directly will result in FName::None
					const FStAbilityData AbilityData = GetAbilityDataFromName(SpellName);
					spellPtr->GameName = AbilityData.GameName;
					return *spellPtr;
				}
			}
		}
	}
	// Return a default item data struct
	return FStSpellData();
}

bool UAbilitySystem::GetAbilityNameIsValid(FName AbilityName, bool PerformLookup)
{
	if (!AbilityName.IsNone())
	{
		if (!PerformLookup)
			return true;
		const FStAbilityData itemData = (GetAbilityDataFromName(AbilityName));
		return !itemData.DisplayName.IsEmpty();
	}
	return false; // Invalid or not found
}

bool UAbilitySystem::GetSpellNameIsValid(FName SpellName, bool PerformLookup)
{
	if (!SpellName.IsNone())
	{
		if (!PerformLookup)
			return true;
		if (GetAbilityNameIsValid(SpellName, PerformLookup))
		{
			const FStAbilityData AbilityData = GetAbilityDataFromName(SpellName);
			return AbilityData.AllowedClasses.Num() > 0;
		}
	}
	return false; // Invalid or not found
}

bool UAbilitySystem::GetNameIsValidAbilityOrSpell(FName GameName, bool PerformLookup)
{
	if (GetAbilityNameIsValid(GameName, PerformLookup))
		return true;
	return GetSpellNameIsValid(GameName, PerformLookup);
}

bool UAbilitySystem::GetAbilityDataIsValid(FStAbilityData AbilityData)
{
	return !AbilityData.DisplayName.IsEmpty();
}
