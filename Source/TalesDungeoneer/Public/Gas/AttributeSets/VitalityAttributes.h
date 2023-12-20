// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "Delegates/Delegate.h"
#include "../AttributeHelpers.h"

#include "VitalityAttributes.generated.h"

/**
 * Vitality Attributes are any attributes related to the characters well-being,
 * such as health, hunger, hydration, magic. It also includes damage factors
 * and factors the character depends on such as ammunition.
 */
UCLASS()
class TALESDUNGEONEER_API UVitalityAttributes : public UAttributeSet
{
	GENERATED_BODY()
public:
	
	UVitalityAttributes();

	UFUNCTION(BlueprintPure)
	float GetDeathHealthValue() const { return GetMaximumHealth() * DeathPercentage; }
	
	// If health falls this far below zero (Percentage of Max Health)
	// then the attribute system flags itself as 'bDead'
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DeathPercentage = (0 - 0.1);
	
	UPROPERTY(BlueprintReadOnly, Category = "Status Attributes",
		ReplicatedUsing=OnRep_CurrentHealth, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CurrentHealth;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, CurrentHealth);
	
	UPROPERTY(BlueprintReadOnly, Category = "Status Attributes",
		ReplicatedUsing=OnRep_CurrentMagic, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CurrentStamina;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, CurrentStamina);
	
	UPROPERTY(BlueprintReadOnly, Category = "Status Attributes",
		ReplicatedUsing=OnRep_CurrentArmorClass, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CurrentArmorClass;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, CurrentArmorClass);
	
	UPROPERTY(BlueprintReadOnly, Category = "Status Attributes",
		ReplicatedUsing=OnRep_CurrentArmor, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CurrentArmor;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, CurrentArmor);
	
	UPROPERTY(BlueprintReadOnly, Category = "Status Attributes",
		ReplicatedUsing=OnRep_CurrentMagic, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CurrentMagic;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, CurrentMagic);
	
	UPROPERTY(BlueprintReadOnly, Category = "Status Attributes",
		ReplicatedUsing=OnRep_CurrentHunger, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CurrentHunger;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, CurrentHunger);
	
	UPROPERTY(BlueprintReadOnly, Category = "Status Attributes",
		ReplicatedUsing=OnRep_CurrentMagic, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CurrentHydration;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, CurrentHydration);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute Maximums",
		ReplicatedUsing=OnRep_MaximumHealth, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaximumHealth;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, MaximumHealth);
	
	UPROPERTY(BlueprintReadOnly, Category = "Status Attributes",
		ReplicatedUsing=OnRep_MaximumArmorClass, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaximumArmorClass;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, MaximumArmorClass);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute Maximums",
		ReplicatedUsing=OnRep_MaximumArmor, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaximumArmor;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, MaximumArmor);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute Maximums",
		ReplicatedUsing=OnRep_MaximumMagic, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaximumMagic;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, MaximumMagic);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute Maximums",
		ReplicatedUsing=OnRep_MaximumMagic, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaximumStamina;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, MaximumStamina);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute Maximums",
		ReplicatedUsing=OnRep_MaximumHunger, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaximumHunger;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, MaximumHunger);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute Maximums",
		ReplicatedUsing=OnRep_MaximumHydration, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaximumHydration;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, MaximumHydration);
	
	UPROPERTY(BlueprintReadOnly, Category = "Status Attributes",
		ReplicatedUsing=OnRep_PassiveHealthRegen, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData PassiveHealthRegen;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, PassiveHealthRegen);
	
	UPROPERTY(BlueprintReadOnly, Category = "Status Attributes",
		ReplicatedUsing=OnRep_PassiveMagicRegen, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData PassiveMagicRegen;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, PassiveMagicRegen);
	
	UPROPERTY(BlueprintReadOnly, Category = "Status Attributes",
		ReplicatedUsing=OnRep_PassiveStaminaRegen, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData PassiveStaminaRegen;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, PassiveStaminaRegen);
	
	UPROPERTY(BlueprintReadOnly, Category = "Status Attributes",
		ReplicatedUsing=OnRep_PassiveHungerDrain, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData PassiveHungerDrain;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, PassiveHungerDrain);
	
	UPROPERTY(BlueprintReadOnly, Category = "Status Attributes",
		ReplicatedUsing=OnRep_PassiveHydroDrain, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData PassiveHydroDrain;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, PassiveHydroDrain);

	UPROPERTY(BlueprintReadOnly, Category = "Damage Attributes",
		ReplicatedUsing=OnRep_Ammunition, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Ammunition;
	ATTRIBUTE_ACCESSORS(UVitalityAttributes, Ammunition);

protected:

	virtual void PreAttributeBaseChange(
		const FGameplayAttribute& Attribute, float& NewValue) const override;

	virtual void PreAttributeChange(
		const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void ClampAttributeOnChange(
		const FGameplayAttribute& Attribute, float& NewValue) const;

	UFUNCTION()
	virtual void OnRep_CurrentHealth(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_CurrentArmorClass(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_CurrentArmor(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_CurrentMagic(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_CurrentStamina(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_CurrentHunger(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_CurrentHydration(const FGameplayAttributeData& OldData);

	UFUNCTION()
	virtual void OnRep_PassiveHealthRegen(const FGameplayAttributeData& OldData);

	UFUNCTION()
	virtual void OnRep_PassiveStaminaRegen(const FGameplayAttributeData& OldData);

	UFUNCTION()
	virtual void OnRep_PassiveMagicRegen(const FGameplayAttributeData& OldData);

	UFUNCTION()
	virtual void OnRep_PassiveHungerDrain(const FGameplayAttributeData& OldData);

	UFUNCTION()
	virtual void OnRep_PassiveHydroDrain(const FGameplayAttributeData& OldData);

	UFUNCTION()
	virtual void OnRep_MaximumHealth(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_MaximumArmorClass(const FGameplayAttributeData& OldData);

	UFUNCTION()
	virtual void OnRep_MaximumArmor(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_MaximumMagic(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_MaximumStamina(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_MaximumHunger(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_MaximumHydration(const FGameplayAttributeData& OldData);

	UFUNCTION()
	virtual void OnRep_Ammunition(const FGameplayAttributeData& OldData);
};
