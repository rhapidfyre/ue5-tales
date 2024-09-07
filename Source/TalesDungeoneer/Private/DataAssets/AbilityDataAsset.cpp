// Take Five Games, LLC


#include "DataAssets/AbilityDataAsset.h"

UPrimaryAbilityDataAsset::UPrimaryAbilityDataAsset()
	: AbilityReference(nullptr)
{
}

/**
 * \brief Performs a synchronous lookup for the ability associated with this asset.
 *        Slow. Try to use GetAbilityReferenceAsync() when possible.
 * \return The ability, or nullptr on failure
 */
UTalesGameplayAbility* UPrimaryAbilityDataAsset::GetAbilityReference() const
{
	if (!IsValid(AbilityReference)) return nullptr;
	return AbilityReference.GetDefaultObject();
}
