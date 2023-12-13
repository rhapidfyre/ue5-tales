// Copyright Take Five Games, LLC 2023 - All Rights Reserved


#include "EquipmentWorn.h"

#include "lib/ItemData.h"
#include "TalesDungeoneer/Characters/Components/MeshMergeComponent.h"
#include "TalesDungeoneer/lib/GameplayTags.h"


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

void UEquipmentSystem::GetAllHeadBodyTags(FGameplayTagContainer& TagContainer)
{
    TagContainer = TagContainer.EmptyContainer;
    TagContainer.AddTag(TAG_Character_Body_Head);
    TagContainer.AddTag(TAG_Character_Body_Head_Ears);
    TagContainer.AddTag(TAG_Character_Body_Head_Eyebrow);
    TagContainer.AddTag(TAG_Character_Body_Head_Eyelashes);
    TagContainer.AddTag(TAG_Character_Body_Head_Eyes);
}

void UEquipmentSystem::GetAllUpperBodyTags(FGameplayTagContainer& TagContainer)
{
    TagContainer = TagContainer.EmptyContainer;
    TagContainer.AddTag(TAG_Character_Body_Upper);
    TagContainer.AddTag(TAG_Character_Body_Upper_Neck);
    TagContainer.AddTag(TAG_Character_Body_Upper_Torso);
    TagContainer.AddTag(TAG_Character_Body_Upper_Hand);
}

void UEquipmentSystem::GetAllLowerBodyTags(FGameplayTagContainer& TagContainer)
{
    TagContainer = TagContainer.EmptyContainer;
    TagContainer.AddTag(TAG_Character_Body_Lower);
    TagContainer.AddTag(TAG_Character_Body_Lower_Legs);
    TagContainer.AddTag(TAG_Character_Body_Lower_Underwear);
    TagContainer.AddTag(TAG_Character_Body_Lower_Feet);
}

TArray<FStMeshMergeData> UEquipmentSystem::GetAllDefaultMeshes(bool IsMale)
{
    TArray<FStMeshMergeData> DefaultMeshes;
    
    const UDataTable* EquipmentDt = GetEquipmentWornDataTable();
    if (!IsValid(EquipmentDt))
        return {};
			
    const FGameplayTag DefaultTag = TAG_Character_Body_Default.GetTag();
    
    TArray<FName> AllRowNames = EquipmentDt->GetRowNames();
    for (const FName EquipmentRowName : AllRowNames)
    {
        if (GetEquipmentWornNameIsValid(EquipmentRowName))
        {
            FStEquipmentWorn WornData = GetEquipmentWornData(EquipmentRowName);
            if (WornData.BodyPartTags.HasTag(DefaultTag))
            {
                FStMeshMergeData MeshMergeData(EquipmentRowName, IsMale);
                DefaultMeshes.Add(MeshMergeData);
            }
        }
    }
    
    return DefaultMeshes;
}

FStMeshMergeData UEquipmentSystem::GetDefaultMeshFromTag(FGameplayTag SearchTag, bool IsMale)
{
    
    const UDataTable* EquipmentDt = GetEquipmentWornDataTable();
    if (!IsValid(EquipmentDt))
        return {};
    
    const FGameplayTag DefaultTag = TAG_Character_Body_Default.GetTag();
    if (SearchTag == DefaultTag)
        return {};
    
    TArray<FName> AllRowNames = EquipmentDt->GetRowNames();
    for (const FName EquipmentRowName : AllRowNames)
    {
        if (GetEquipmentWornNameIsValid(EquipmentRowName))
        {
            FStEquipmentWorn WornData = GetEquipmentWornData(EquipmentRowName);
            if (   WornData.BodyPartTags.HasTag(DefaultTag)
                && WornData.BodyPartTags.HasTag(SearchTag))
            {
                FStMeshMergeData MeshMergeData(EquipmentRowName, IsMale);
                return MeshMergeData;
            }
        }
    }
    return {};
}


TArray<FStMeshMergeData> Helper_DefaultMeshFinder(
    const FGameplayTagContainer& TagsToFind, bool UseMaleMesh = true, int MatchesForSuccess = 1)
{
    TArray<FStMeshMergeData> ReturnArray;
    const FGameplayTag DefaultTag = TAG_Character_Body_Default.GetTag();
    
    // Get all entries in the data table & find all of the ones with a default tag.
    const UDataTable* EquipmentTable = UEquipmentSystem::GetEquipmentWornDataTable();
    if (IsValid(EquipmentTable))
    {
        const TArray<FName> RowNames = EquipmentTable->GetRowNames();
        for (const FName RowName : RowNames)
        {
            const FStEquipmentWorn EquipData =
                    UEquipmentSystem::GetEquipmentWornData(RowName);
            
            FGameplayTag TagFound = DefaultTag;
            for (FGameplayTag GameTag : TagsToFind)
            {
                if (EquipData.BodyPartTags.HasTag(GameTag)
                    && !GameTag.MatchesTag(DefaultTag))
                {
                    TagFound = GameTag;
                    break;
                }
            }
            
            if (!TagFound.MatchesTag(DefaultTag))
            {
                // If this item contains a default tag and a valid mesh
                if (EquipData.BodyPartTags.HasTag(DefaultTag))
                {
                    if (    UseMaleMesh && IsValid(EquipData.MaleMesh)
                        || !UseMaleMesh && IsValid(EquipData.FemaleMesh)
                        )
                    {
                        FStMeshMergeData MeshMergeData(RowName, UseMaleMesh);
                        ReturnArray.Add(MeshMergeData);
                    }
                }
            }
            
        }
    }
    return ReturnArray;
}


/**
 * @brief Called when the player's head characteristics are changed.
 *        Ensures all head parts match the chosen body type.
 * @param IsSquareFace True if square-faced (masculine head)
 * @return Returns an array of all default head parts
 */
TArray<FStMeshMergeData> UEquipmentSystem::GetDefaultMeshMergeHeadBody(bool IsSquareFace)
{
    FGameplayTagContainer TagContainer;
    GetAllHeadBodyTags(TagContainer);
    return Helper_DefaultMeshFinder(TagContainer, IsSquareFace);
}


/**
 * @brief Called when the player's upper body type characteristics are changed.
 *        Ensures all upper body parts match the chosen body type.
 * @param IsBusty True if busty (has breast tissue)
 * @return Returns an array of all default upper body parts
 */
TArray<FStMeshMergeData> UEquipmentSystem::GetDefaultMeshMergeUpperBody(bool IsBusty)
{
    FGameplayTagContainer TagContainer;
    GetAllUpperBodyTags(TagContainer);
    return Helper_DefaultMeshFinder(TagContainer, !IsBusty);
}


/**
 * @brief Called when the player's lower body type characteristics are changed.
 *        Ensures all upper body parts match the chosen body type.
 * @param IsWideHip True if wide hips (feminine lower body)
 * @return Returns an array of all default lower body parts
 */
TArray<FStMeshMergeData> UEquipmentSystem::GetDefaultMeshMergeLowerBody(bool IsWideHip)
{
    FGameplayTagContainer TagContainer;
    GetAllLowerBodyTags(TagContainer);
    return Helper_DefaultMeshFinder(TagContainer, !IsWideHip);
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
