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

TArray<FStAbilityData> AbilityMergeArray(ECharacterClass AllowedClass,
	TArray<FStAbilityData>& UnsortedArrayA, TArray<FStAbilityData>& UnsortedArrayB)
{
	TArray<FStAbilityData> FinalArray;
	while ( !UnsortedArrayA.IsEmpty() && !UnsortedArrayB.IsEmpty() )
	{
		if (UnsortedArrayA[0].AllowedClasses[AllowedClass] > UnsortedArrayB[0].AllowedClasses[AllowedClass])
		{
			FStAbilityData NewData = UAbilitySystem::GetAbilityDataFromName(UnsortedArrayB[0].GameName);
			FinalArray.Add(UnsortedArrayB[0]);
			UnsortedArrayB.RemoveAt(0);
		}
		else
		{
			FStAbilityData NewData = UAbilitySystem::GetAbilityDataFromName(UnsortedArrayA[0].GameName);
			FinalArray.Add(UnsortedArrayA[0]);
			UnsortedArrayA.RemoveAt(0);
		}
	}
	while ( !UnsortedArrayA.IsEmpty() )
	{
		FStAbilityData NewData = UAbilitySystem::GetAbilityDataFromName(UnsortedArrayA[0].GameName);
		FinalArray.Add(UnsortedArrayA[0]);
		UnsortedArrayA.RemoveAt(0);
	}
	while ( !UnsortedArrayB.IsEmpty() )
	{
		FStAbilityData NewData = UAbilitySystem::GetAbilityDataFromName(UnsortedArrayB[0].GameName);
		FinalArray.Add(UnsortedArrayB[0]);
		UnsortedArrayB.RemoveAt(0);
	}
	return FinalArray;
}

TArray<FStAbilityData> AbilityMergeSort(ECharacterClass AllowedClass,
	TArray<FStAbilityData> UnsortedArray)
{
	const int sizeOfArray = UnsortedArray.Num();
	if (sizeOfArray < 2) return UnsortedArray;

	TArray<FStAbilityData> UnsortedArrayA;
	TArray<FStAbilityData> UnsortedArrayB;
	for (int i = 0; i < sizeOfArray; i++)
	{
		if (i < sizeOfArray/2)
			UnsortedArrayA.Add(UnsortedArray[i]);
		else
			UnsortedArrayB.Add(UnsortedArray[i]);
	}

	UnsortedArrayA = AbilityMergeSort(AllowedClass, UnsortedArrayA);
	UnsortedArrayA = AbilityMergeSort(AllowedClass, UnsortedArrayB);
	return AbilityMergeArray(AllowedClass, UnsortedArrayA, UnsortedArrayB);
}

TArray<FStAbilityData> UAbilitySystem::GetAbilityDataTableSortedByClass(ECharacterClass CharacterClass)
{
	const UDataTable* AbilityTable = GetAbilityDataTable();
	const FString ErrorCaught;
	
	TArray<FName> UnsortedArray = AbilityTable->GetRowNames();
	
	TArray<FStAbilityData> SortedArray;
	for (int i = 0; i < UnsortedArray.Num(); i++)
	{
		FStAbilityData AbilityData = GetAbilityDataFromName(UnsortedArray[i]);
		if (AbilityData.AllowedClasses.Contains(CharacterClass))
			SortedArray.Add(AbilityData);
	}
	return AbilityMergeSort(CharacterClass, SortedArray);
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
		return SpellData.AllowedClasses.Num() > 0;
	}
	return false; // Invalid or not found
}

bool UAbilitySystem::GetAbilityDataIsValid(FStAbilityData AbilityData)
{
	return !AbilityData.DisplayName.IsEmpty();
}
