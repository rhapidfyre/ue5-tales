#include "Weapons/WeaponSystem.h"

UDataTable* UWeaponSystem::GetWeaponDataTable()
{
	const FSoftObjectPath itemTable =
		FSoftObjectPath(TEXT("/Game/TalesContent/DataTables/DT_WeaponData.DT_WeaponData"));
    
	UDataTable* dataTable = Cast<UDataTable>(itemTable.ResolveObject());
	if (dataTable) return dataTable;
	
	dataTable = Cast<UDataTable>(itemTable.TryLoad());
	if (dataTable) return dataTable;

	return nullptr;
}

FString UWeaponSystem::GetWeaponItemName(FStWeaponData weaponData)
{
	return weaponData.DisplayName;
}

bool UWeaponSystem::IsMeleeAttack(EWeaponTypes weaponType)
{
	return (   weaponType == EWeaponTypes::NONE
		|| weaponType == EWeaponTypes::SPEAR
		|| weaponType == EWeaponTypes::SWORD
		|| weaponType == EWeaponTypes::PICKAXE
		);
}

bool UWeaponSystem::IsRangedAttack(EWeaponTypes weaponType)
{
	return !IsMeleeAttack(weaponType);
}

FStWeaponData UWeaponSystem::GetWeaponDataFromName(FName weaponName)
{
	if (weaponName.IsValid())
	{
		if (weaponName != GetInvalidName())
		{
			if ( const UDataTable* dt = GetWeaponDataTable() )
			{
				if (IsValid(dt))
				{
					const FString errorCaught;
					FStWeaponData* itemDataPtr = dt->FindRow<FStWeaponData>(weaponName, errorCaught);
					if (itemDataPtr != nullptr)
					{
						return *itemDataPtr;
					}
				}
			}
		}
	}
	// Return a default item data struct
	return FStWeaponData();
}

bool UWeaponSystem::GetWeaponNameIsValid(FName WeaponName)
{
	if (WeaponName.IsValid())
	{
		const FStWeaponData itemData = GetWeaponDataFromName(WeaponName);
		return GetWeaponIsValid(itemData);
	}
	return false; // Invalid or not found
}

bool UWeaponSystem::GetWeaponIsValid(const FStWeaponData &WeaponData)
{
	return (WeaponData.MeshStatic != nullptr || WeaponData.MeshSkeletal != nullptr);
}
