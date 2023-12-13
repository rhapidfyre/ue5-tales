#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TalesDungeoneer/lib/datastructures/WeaponData.h"

#include "WeaponSystem.generated.h"

/**
 * All global functionality for the UWeaponSystem, comparable to the UItemSystem.
 * Unlikely the UItemSystem, most of the UWeaponSystem's struct accessing can just be done directly.
 * That's because this system is generally only used by a few classes.. If we change a member variable, it's just
 * 5-10 minutes of a couple files to change everything else to match. These are for more in depth inconvenient things
 * like calling data rows from data tables and validating weapon data.
 */
UCLASS()
class UWeaponSystem : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Weapon System Globals")
	static FName GetInvalidName() { return "None"; }

	UFUNCTION(BlueprintPure, Category = "Weapon System Globals")
	static UDataTable* GetWeaponDataTable();

	UFUNCTION(BlueprintPure, Category = "Weapon System Globals")
	static FString GetWeaponItemName(FStWeaponData weaponData);

	UFUNCTION(BlueprintPure, Category = "Weapon System Globals")
	static bool IsMeleeAttack(EWeaponTypes weaponType);

	UFUNCTION(BlueprintPure, Category = "Weapon System Globals")
	static bool IsRangedAttack(EWeaponTypes weaponType);

	/**
	 * Checks if the given FName is a valid weapon name.
	 * @param weaponName The FName for the weapon to challenge
	 * @param performLookup If true, will check validity in the weapon data table
	 */
	UFUNCTION(BlueprintPure, Category = "Weapon System Globals")
	static bool GetWeaponNameIsValid(FName WeaponName);
	
	UFUNCTION(BlueprintPure, Category = "Weapon System Globals")
	static bool GetWeaponIsValid(const FStWeaponData &WeaponData);

	UFUNCTION(BlueprintPure, Category = "Weapon System Globals")
	static FStWeaponData GetWeaponDataFromName(FName weaponName);
};
