// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "NativeGameplayTags.h"
#include "Delegates/Delegate.h"
#include "../AttributeHelpers.h"

#include "DamageAttributes.generated.h"


UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_SetByCaller)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_World)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Physical_Generic)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Physical_Blunt)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Physical_Slash)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Physical_Pierce)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Physical_Bite)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Physical_Kick)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Physical_Claw)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Physical_Sting)

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Elemental_Generic)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Elemental_Fire)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Elemental_Frost)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Elemental_Acid)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Elemental_Shock)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Elemental_Radioactive)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Elemental_Sonic)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Magic_Generic)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Magic_Holy)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Damage_Magic_DarkMagic)

/**
 *
 */
UCLASS()
class TALESDUNGEONEER_API UDamageAttributes : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	
	UDamageAttributes();
	
	UPROPERTY(BlueprintReadOnly, Category = "Damage Attributes",
		ReplicatedUsing=OnRep_IncomingDamage, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, IncomingDamage);
	
	UPROPERTY(BlueprintReadOnly, Category = "Damage Attributes",
		ReplicatedUsing=OnRep_CriticalChance, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CriticalChance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, CriticalChance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Damage Attributes",
		ReplicatedUsing=OnRep_CriticalMultiplier, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CriticalMultiplier;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, CriticalMultiplier);
	
	UPROPERTY(BlueprintReadOnly, Category = "Damage Attributes",
		ReplicatedUsing=OnRep_LuckyChance, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData LuckyChance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, LuckyChance);

	UPROPERTY(BlueprintReadOnly, Category = "Damage Attributes",
		ReplicatedUsing=OnRep_DamageModifier, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData DamageModifier;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, DamageModifier);

	UPROPERTY(BlueprintReadOnly, Category = "Damage Attributes",
		ReplicatedUsing=OnRep_DamageMultiplier, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData DamageMultiplier;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, DamageMultiplier);

	
	/**
	 * Damage Resistances
	 */
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes",
		ReplicatedUsing=OnRep_BluntResistance, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BluntResistance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, BluntResistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes",
		ReplicatedUsing=OnRep_SlashResistance, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData SlashResistance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, SlashResistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes",
		ReplicatedUsing=OnRep_PierceResistance, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData PierceResistance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, PierceResistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BiteResistance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, BiteResistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData KickResistance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, KickResistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData ClawResistance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, ClawResistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData StingResistance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, StingResistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes",
		ReplicatedUsing=OnRep_FireResistance, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData FireResistance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, FireResistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes",
		ReplicatedUsing=OnRep_FrostResistance, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData FrostResistance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, FrostResistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes",
		ReplicatedUsing=OnRep_AcidResistance, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData AcidResistance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, AcidResistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes",
		ReplicatedUsing=OnRep_ShockResistance, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData ShockResistance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, ShockResistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes",
		ReplicatedUsing=OnRep_RadioResistance, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData RadioResistance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, RadioResistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes",
		ReplicatedUsing=OnRep_SonicResistance, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData SonicResistance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, SonicResistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes",
		ReplicatedUsing=OnRep_HolyResistance, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData HolyResistance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, HolyResistance);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes",
		ReplicatedUsing=OnRep_DarkResistance, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData DarkResistance;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, DarkResistance);

	
	/**
	 * Damage Bonuses
	 */
	
	UPROPERTY(BlueprintReadOnly, Category = "Bonus Attributes",
		ReplicatedUsing=OnRep_BluntBonus, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BluntBonus;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, BluntBonus);
	
	UPROPERTY(BlueprintReadOnly, Category = "Bonus Attributes",
		ReplicatedUsing=OnRep_SlashBonus, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData SlashBonus;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, SlashBonus);
	
	UPROPERTY(BlueprintReadOnly, Category = "Bonus Attributes",
		ReplicatedUsing=OnRep_PierceBonus, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData PierceBonus;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, PierceBonus);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData BiteBonus;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, BiteBonus);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData KickBonus;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, KickBonus);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData ClawBonus;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, ClawBonus);
	
	UPROPERTY(BlueprintReadOnly, Category = "Resistance Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData StingBonus;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, StingBonus);
	
	UPROPERTY(BlueprintReadOnly, Category = "Bonus Attributes",
		ReplicatedUsing=OnRep_FireBonus, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData FireBonus;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, FireBonus);
	
	UPROPERTY(BlueprintReadOnly, Category = "Bonus Attributes",
		ReplicatedUsing=OnRep_FrostBonus, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData FrostBonus;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, FrostBonus);
	
	UPROPERTY(BlueprintReadOnly, Category = "Bonus Attributes",
		ReplicatedUsing=OnRep_AcidBonus, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData AcidBonus;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, AcidBonus);
	
	UPROPERTY(BlueprintReadOnly, Category = "Bonus Attributes",
		ReplicatedUsing=OnRep_ShockBonus, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData ShockBonus;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, ShockBonus);
	
	UPROPERTY(BlueprintReadOnly, Category = "Bonus Attributes",
		ReplicatedUsing=OnRep_RadioBonus, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData RadioBonus;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, RadioBonus);
	
	UPROPERTY(BlueprintReadOnly, Category = "Bonus Attributes",
		ReplicatedUsing=OnRep_SonicBonus, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData SonicBonus;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, SonicBonus);
	
	UPROPERTY(BlueprintReadOnly, Category = "Bonus Attributes",
		ReplicatedUsing=OnRep_HolyBonus, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData HolyBonus;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, HolyBonus);
	
	UPROPERTY(BlueprintReadOnly, Category = "Bonus Attributes",
		ReplicatedUsing=OnRep_DarkBonus, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData DarkBonus;
	ATTRIBUTE_ACCESSORS(UDamageAttributes, DarkBonus);
	

	/**
	 * Protected Methods/Members & Replication
	 */
	
protected:

	virtual void PreAttributeBaseChange(
		const FGameplayAttribute& Attribute, float& NewValue) const override;

	virtual void PreAttributeChange(
		const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void ClampAttributeOnChange(
	const FGameplayAttribute& Attribute, float& NewValue) const;

	UFUNCTION() virtual void OnRep_IncomingDamage(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_CriticalChance(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_CriticalMultiplier(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_LuckyChance(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_DamageModifier(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_DamageMultiplier(const FGameplayAttributeData& OldData);
	
	UFUNCTION() virtual void OnRep_BluntResistance(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_SlashResistance(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_PierceResistance(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_FireResistance(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_FrostResistance(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_AcidResistance(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_ShockResistance(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_RadioResistance(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_SonicResistance(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_HolyResistance(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_DarkResistance(const FGameplayAttributeData& OldData);
	
	UFUNCTION() virtual void OnRep_BluntBonus(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_SlashBonus(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_PierceBonus(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_FireBonus(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_FrostBonus(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_AcidBonus(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_ShockBonus(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_RadioBonus(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_SonicBonus(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_HolyBonus(const FGameplayAttributeData& OldData);
	UFUNCTION() virtual void OnRep_DarkBonus(const FGameplayAttributeData& OldData);
	
};