// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "NativeGameplayTags.h"
#include "Delegates/Delegate.h"
#include "../AttributeHelpers.h"

#include "CoreStatsAttributes.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stats_Strength)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stats_Dexterity)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stats_Fortitude)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stats_Astuteness)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stats_Intellect)
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Stats_Charisma)


/**
 * Vitality Attributes are any attributes related to the characters well-being,
 * such as health, hunger, hydration, magic. It also includes damage factors
 * and factors the character depends on such as ammunition.
 */
UCLASS()
class TALESDUNGEONEER_API UCoreStatsAttributes : public UAttributeSet
{
	GENERATED_BODY()
public:
	
	UCoreStatsAttributes();

	TMulticastDelegate<void(const FOnAttributeChangeData& Attribute)> OnAttributeUpdated;
	
	UPROPERTY(BlueprintReadOnly, Category = "Core Stat Attributes",
		ReplicatedUsing=OnRep_Strength, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS(UCoreStatsAttributes, Strength);
	
	UPROPERTY(BlueprintReadOnly, Category = "Core Stat Attributes",
		ReplicatedUsing=OnRep_Dexterity, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Dexterity;
	ATTRIBUTE_ACCESSORS(UCoreStatsAttributes, Dexterity);
	
	UPROPERTY(BlueprintReadOnly, Category = "Core Stat Attributes",
		ReplicatedUsing=OnRep_Fortitude, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Fortitude;
	ATTRIBUTE_ACCESSORS(UCoreStatsAttributes, Fortitude);
	
	UPROPERTY(BlueprintReadOnly, Category = "Core Stat Attributes",
		ReplicatedUsing=OnRep_Astuteness, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Astuteness;
	ATTRIBUTE_ACCESSORS(UCoreStatsAttributes, Astuteness);
	
	UPROPERTY(BlueprintReadOnly, Category = "Core Stat Attributes",
		ReplicatedUsing=OnRep_Intellect, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Intellect;
	ATTRIBUTE_ACCESSORS(UCoreStatsAttributes, Intellect);
	
	UPROPERTY(BlueprintReadOnly, Category = "Core Stat Attributes",
		ReplicatedUsing=OnRep_Charisma, Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Charisma;
	ATTRIBUTE_ACCESSORS(UCoreStatsAttributes, Charisma);

	TArray<FGameplayAttribute> GetAllCoreStatAttributes() const;

protected:

	virtual void PreAttributeBaseChange(
		const FGameplayAttribute& Attribute, float& NewValue) const override;

	virtual void PreAttributeChange(
		const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void ClampAttributeOnChange(
		const FGameplayAttribute& Attribute, float& NewValue) const;

	UFUNCTION()
	virtual void OnRep_Strength(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_Dexterity(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_Fortitude(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_Astuteness(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_Intellect(const FGameplayAttributeData& OldData);
	
	UFUNCTION()
	virtual void OnRep_Charisma(const FGameplayAttributeData& OldData);
	
};
