

#include "Characters/Components/EquipmentComponent.h"

#include "Characters/CharacterBase.h"
#include "DataAssets/CharacterDefaults.h"

UEquipmentComponent::UEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

/**
 * Enables and disables the equipment functionality and visuals for
 * the specified equipment slot in the InventoryComponent.
 * @param SlotNumber The InventoryComponent equipment slot
 */
void UEquipmentComponent::InitEquipmentItem(int SlotNumber)
{
	if (!IsValid(InventoryReference))
	{
		const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
		InventoryReference = IsValid(CharacterBase) ? CharacterBase->InventoryComponent : nullptr;
	}
	
	if (!IsValid(MeshMergeReference))
	{
		const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
		MeshMergeReference = IsValid(CharacterBase) ? CharacterBase->MeshMergeComponent : nullptr;
	}

	if (!IsValid(InventoryReference) || !IsValid(MeshMergeReference))
	{
		return;
	}

	// Clear the slot's equipment
	SetEquipmentDisabled(SlotNumber);

	// Get the item that is currently in the equipment slot
	const FStItemData itemInSlot = InventoryReference->GetSlotNumberItem(SlotNumber);
	const UEquipmentDataAsset* EquipmentItem = Cast<UEquipmentDataAsset>(itemInSlot.Data);
	if (IsValid(EquipmentItem))
	{
		if (EquipmentItem->bStartEquipped)
			{ SetEquipmentEnabled(SlotNumber); }
	}
	
}

/**
 * Toggles the equipment slot state, or forces the equipment to be enabled.
 * @param SlotNumber The InventoryComponent equipment slot
 * @param bMakeReady True forces equipment to enable. False alternates current state.
 */
void UEquipmentComponent::ToggleEquipment(int SlotNumber, bool bMakeReady)
{
	if (!bMakeReady)
	{
		if (EquipmentSlots.Contains(SlotNumber))
			{ bMakeReady = !EquipmentSlots[SlotNumber]; }
	}
	
	if (bMakeReady)
		{ SetEquipmentEnabled(SlotNumber); }
	else
		{ SetEquipmentDisabled(SlotNumber); }
}

/**
 * Sets the equivalent equipment slot to be enabled/in-use/armed
 * @param SlotNumber The InventoryComponent equipment slot
 */
void UEquipmentComponent::SetEquipmentEnabled(int SlotNumber)
{
	if (EquipmentSlots.Contains(SlotNumber))
	{
		// Get the item that is currently in the equipment slot
		const FStItemData itemInSlot = InventoryReference->GetSlotNumberItem(SlotNumber);
		const UEquipmentDataAsset* equipmentItemData = Cast<UEquipmentDataAsset>(itemInSlot.Data);
		if (IsValid(equipmentItemData))
		{
			const FGameplayTag slotTag = InventoryReference->GetSlotEquipmentTag(SlotNumber);
			const FGameplayTag bodyTag = MeshMergeReference->GetBodyPartFromEquipmentSlot(slotTag);
		
			FMeshMergeMappings mergeMap = MeshMergeReference->GetMeshMappingFromIndex(
											MeshMergeReference->FindMeshMappingByTag(bodyTag) );

			FMeshMergeMappings newMapping = MeshMergeReference->CreateMeshMapping(equipmentItemData,
				InventoryReference->GetSlotEquipmentTag(SlotNumber),	// Equipment Slot (i.e. Equipment.Slot.Primary)
				mergeMap.GameplayTags.HasTag(TAG_Character_Sex_Female));
			
			MeshMergeReference->AddMeshToMerge(newMapping);
			EquipmentSlots[SlotNumber] = true;
			
			MeshMergeReference->PerformMeshMerge();
			NotifySlotToggled(SlotNumber);
		}
	}
}


/**
 * Sets the equivalent equipment slot to be disabled/unused/unarmed
 * @param SlotNumber The InventoryComponent equipment slot
 */
void UEquipmentComponent::SetEquipmentDisabled(int SlotNumber)
{
	if (!EquipmentSlots.Contains(SlotNumber))
		{ EquipmentSlots.Add(SlotNumber, false); }

	const FGameplayTag slotTag = InventoryReference->GetSlotEquipmentTag(SlotNumber);
	MeshMergeReference->RemoveMeshFromMerge(nullptr, slotTag);
	MeshMergeReference->PerformMeshMerge();
	
	EquipmentSlots[SlotNumber] = false;
	NotifySlotToggled(SlotNumber);
}

/**
 * Adjusts the mesh of the equipment in relation to the character mesh
 * @param SlotNumber The InventoryComponent equipment slot
 * @param AttachParent The parent to attach to (opt)
 * @param AttachmentBone The name of the bone to attach to (opt)
 * @param AdjustmentTransform The transform adjustment to the equipment mesh (opt)
 */
void UEquipmentComponent::AdjustAttachment(int SlotNumber,
	AActor* AttachParent, FName AttachmentBone, FTransform AdjustmentTransform)
{
	if (!EquipmentSlots.Contains(SlotNumber))
		{ InitEquipmentItem(SlotNumber); }
	
	if (!IsValid(MeshMergeReference))
	{
		const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
		MeshMergeReference = IsValid(CharacterBase) ? CharacterBase->MeshMergeComponent : nullptr;
	}
	
	if (IsValid(MeshMergeReference) && IsValid(InventoryReference))
	{
		const FStInventorySlot slotReference = InventoryReference->GetSafeReferenceToSlot(SlotNumber);
		if (slotReference.GetIsEquipmentSlot())
		{
			const FStItemData ItemData = InventoryReference->GetSlotNumberItem(SlotNumber);
			const UEquipmentDataAsset* equipmentData = Cast<UEquipmentDataAsset>(ItemData.Data);
			if (IsValid(equipmentData))
			{
				for (auto& MeshMergeMap : MeshMergeReference->GetAllMeshMergeMappings())
				{
					FMeshMergeMappings meshMapping = MeshMergeReference->CreateMeshMapping(
						equipmentData, slotReference.EquipmentTag,
						MeshMergeMap.GameplayTags.HasTag(TAG_Character_Sex_Female) );
					
					MeshMergeReference->AddMeshToMerge(meshMapping);
				}
			}
		}
	}
}

/**
 * Attempts to perform an attack with the specified slot number.
 * @param SlotNumber The InventoryComponent equipment slot. -1 attacks with all weapons.
 * @return True if the attack was successful, false otherwise.
 */
bool UEquipmentComponent::PerformAttack(int SlotNumber)
{
	return false;
}

TMap<int, bool> UEquipmentComponent::GetAllEquipment() const
{
	return EquipmentSlots;
}

bool UEquipmentComponent::GetIsReady(int SlotNumber) const
{
	if (EquipmentSlots.Contains(SlotNumber))
		{ return EquipmentSlots[SlotNumber]; }
	return false;
}

/**
 * Disables blocking if blocking, enables if not blocking.
 * @return True if blocking is enabled, false otherwise.
 */
bool UEquipmentComponent::ToggleBlocking()
{
	return false;
}

/**
 * Sets blocking to active, if possible.
 * @return True if blocking is active
 */
bool UEquipmentComponent::StartBlocking()
{
	return false;
}

/**
 * Sets blocking to inactive, if possible.
 * @return True if not blocking
 */
bool UEquipmentComponent::StopBlocking()
{
	return false;
}

// Server RPC if 'StartBlocking' was called on the client
void UEquipmentComponent::Server_StartBlocking_Implementation()
{
}

// Server RPC if 'ToggleBlocking' was called on the client
void UEquipmentComponent::Server_ToggleBlocking_Implementation()
{
}

// Server RPC if 'PerformAttack' was called on the client
void UEquipmentComponent::Server_PerformAttack_Implementation(int SlotNumber)
{
}

// Server RPC if 'StopBlocking' was called on the client
void UEquipmentComponent::Server_StopBlocking_Implementation()
{
}

/**
 * Checks if the weapon in the specified slot is ready for an attack action
 * @param SlotNumber The InventoryComponent equipment slot
 *					 If -1 (default), returns TRUE if any weapon is ready
 * @return True if weapon is armed/ready/in-use
 */
bool UEquipmentComponent::GetIsWeaponReady(int SlotNumber) const
{
	return false;
}

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(UEquipmentComponent, TargetActor_);
}

void UEquipmentComponent::OnComponentCreated()
{
	if (bVerboseOutput)
		bShowDebug = true;
	Super::OnComponentCreated();
	SetAutoActivate(true);
	SetIsReplicated(true);
	RegisterComponent();
}

void UEquipmentComponent::NotifySlotToggled(int SlotNumber)
{
	if (EquipmentSlots.Contains(SlotNumber))
	{
		if (OnEquipmentSlotToggled.IsBound())
		{
			OnEquipmentSlotToggled.Broadcast(SlotNumber, EquipmentSlots[SlotNumber]);
		}
	}
}

// Forces the attack cooldown timer to reset to zero.
void UEquipmentComponent::ResetAttackCooldown()
{
	if (AttackCooldown_.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(AttackCooldown_);
	}
}
