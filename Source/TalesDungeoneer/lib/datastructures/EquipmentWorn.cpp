// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "EquipmentWorn.h"

#include "lib/ItemData.h"


/**
 * @brief Returns a reference to the data table for worn equipment data
 * @return UDataTable Reference, or nullptr
 */
UDataTable* UEquipmentSystem::GetEquipmentWornDataTable()
{
    const FSoftObjectPath itemTable = FSoftObjectPath(
        "/Game/TalesContent/DataTables/DT_EquipmentData.DT_EquipmentData");
    UDataTable* dataTable = Cast<UDataTable>(itemTable.ResolveObject());
    if (IsValid(dataTable)) return dataTable;
    return Cast<UDataTable>(itemTable.TryLoad());
}


/**
 * @brief 
 * @param EquipmentName The game name of the equipment we're looking for
 * @return A struct containing data of the worn equipment.
 */
FStEquipmentWorn UEquipmentSystem::GetEquipmentWornData(FName EquipmentName)
{
    if (!EquipmentName.IsNone())
    {
        if ( const UDataTable* dt = GetEquipmentWornDataTable() )
        {
            if (IsValid(dt))
            { 
                const FString errorCaught;
                FStEquipmentWorn* itemDataPtr = dt->FindRow<FStEquipmentWorn>(
                            EquipmentName, errorCaught);
                if (itemDataPtr != nullptr)
                {
                    return *itemDataPtr;
                }
            }
        }
    }
    // Return a default item data struct
    return FStEquipmentWorn();
}


bool UEquipmentSystem::GetEquipmentWornDataIsValid(const FStEquipmentWorn& EquipmentData)
{
    return (IsValid(EquipmentData.FemaleMesh) || IsValid(EquipmentData.MaleMesh));
}


/**
 * @brief Checks if the given game name contains proper wearable equipment data
 * @param EquipmentName The equipment name to check for
 * @return True if valid, false if invalid/nonexistent
 */
bool UEquipmentSystem::GetEquipmentWornNameIsValid(FName EquipmentName)
{
    if (!EquipmentName.IsNone())
    {
        FStEquipmentWorn EquipmentData = GetEquipmentWornData(EquipmentName);
        return GetEquipmentWornDataIsValid(EquipmentData);
    }
    return false; // Invalid or not found
}
