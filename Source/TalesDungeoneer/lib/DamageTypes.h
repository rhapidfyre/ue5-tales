#pragma once

#include "CoreMinimal.h"
#include "enums/GlobalEnums.h"
#include "enums/WeaponEnums.h"
#include "GameFramework/DamageType.h"

#include "DamageTypes.generated.h"

/**
 * The base damage type for all Adventure Zero related damage
 */
UCLASS()
class UDamageTypeBase : public UDamageType
{
	GENERATED_BODY()
public:
	UDamageTypeBase() {};
	UDamageTypeBase(EDamageType dType) : damageType(dType) {};
	UPROPERTY() float damageValue		= 1.0f;
	UPROPERTY() EDamageType damageType	= EDamageType::PHYSICAL;
	UPROPERTY() EWeaponTypes weaponType = EWeaponTypes::NONE;
};


UCLASS(BlueprintType, Blueprintable)
class UDamageTypeAcid : public UDamageTypeBase
{
	GENERATED_BODY()
public:
	UDamageTypeAcid() : UDamageTypeBase(EDamageType::ACID) {};
	UPROPERTY() EElementalType elementType = EElementalType::NATURE;
};

UCLASS(BlueprintType, Blueprintable)
class UDamageTypeCold : public UDamageTypeBase
{
	GENERATED_BODY()
public:
	UDamageTypeCold() : UDamageTypeBase(EDamageType::COLD) {};
	UPROPERTY() EElementalType elementType = EElementalType::FROST;
};

UCLASS(BlueprintType, Blueprintable)
class UDamageTypeDark : public UDamageTypeBase
{
	GENERATED_BODY()
public:
	UDamageTypeDark() : UDamageTypeBase(EDamageType::DARK) {};
	UPROPERTY() EElementalType elementType = EElementalType::DARK;
};

UCLASS(BlueprintType, Blueprintable)
class UDamageTypeHeat : public UDamageTypeBase
{
	GENERATED_BODY()
public:
	UDamageTypeHeat() : UDamageTypeBase(EDamageType::HEAT) {};
	UPROPERTY() EElementalType elementType = EElementalType::FIRE;
};

UCLASS(BlueprintType, Blueprintable)
class UDamageTypeHoly : public UDamageTypeBase
{
	GENERATED_BODY()
public:
	UDamageTypeHoly() : UDamageTypeBase(EDamageType::HOLY) {};
	UPROPERTY() EElementalType elementType = EElementalType::NATURE;
};

UCLASS(BlueprintType, Blueprintable)
class UDamageTypeShock : public UDamageTypeBase
{
	GENERATED_BODY()
public:
	UDamageTypeShock() : UDamageTypeBase(EDamageType::SHOCK) {};
	UPROPERTY() EElementalType elementType = EElementalType::SHOCK;
};

UCLASS(BlueprintType, Blueprintable)
class UDamageTypeSlash : public UDamageTypeBase
{
	GENERATED_BODY()
public:
	UDamageTypeSlash() : UDamageTypeBase(EDamageType::SLASH) {};
	UPROPERTY() EElementalType elementType = EElementalType::NONE;
};

UCLASS(BlueprintType, Blueprintable)
class UDamageTypeSonic : public UDamageTypeBase
{
	GENERATED_BODY()
public:
	UDamageTypeSonic() : UDamageTypeBase(EDamageType::SONIC) {};
	UPROPERTY() EElementalType elementType = EElementalType::AIR;
};

UCLASS(BlueprintType, Blueprintable)
class UDamageTypeToxic : public UDamageTypeBase
{
	GENERATED_BODY()
public:
	UDamageTypeToxic() : UDamageTypeBase(EDamageType::TOXIC) {};
	UPROPERTY() EElementalType elementType = EElementalType::NATURE;
};

UCLASS(BlueprintType, Blueprintable)
class UDamageTypePierce : public UDamageTypeBase
{
	GENERATED_BODY()
public:
	UDamageTypePierce() : UDamageTypeBase(EDamageType::PIERCE) {};
	UPROPERTY() EElementalType elementType = EElementalType::NONE;
};

UCLASS(BlueprintType, Blueprintable)
class UDamageTypePhysical : public UDamageTypeBase
{
	GENERATED_BODY()
public:
	UDamageTypePhysical() : UDamageTypeBase(EDamageType::PHYSICAL) {};
};