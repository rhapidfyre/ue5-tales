

#include "Characters/Components/EquipmentComponent.h"

#include "Characters/CharacterBase.h"
#include "DataAssets/CharacterDefaults.h"
#include "T5GInventorySystem/Public/Data/InventoryTags.h"
#include "lib/ItemData.h"
#include "lib/Logs/TalesLogging.h"
#include "Logging/StructuredLog.h"
#include "Net/UnrealNetwork.h"


UEquipmentComponent::UEquipmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

UInventoryComponent* UEquipmentComponent::GetInventoryReference()
{
	if (!IsValid(InventoryReference))
	{
		if (ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() ))
		{
			InventoryReference = CharacterBase->InventoryComponent;
		}
	}
	return InventoryReference;
}

/**
 * Enables and disables the equipment functionality and visuals for
 * the specified equipment slot in the InventoryComponent.
 * \param SlotNumber The InventoryComponent equipment slot
 */
void UEquipmentComponent::InitEquipmentItem(int SlotNumber)
{
	GetInventoryReference();

	if (!IsValid(MeshMergeReference))
	{
		const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
		MeshMergeReference = IsValid(CharacterBase) ? CharacterBase->MeshMergeComponent : nullptr;
	}

	// Clear the slot's equipment
	SetEquipmentDisabled(SlotNumber);

	// Get the item that is currently in the equipment slot
	const UItemDataAsset* itemInSlot = GetInventoryReference()->GetItemDataInSlotNumber(SlotNumber);
	const UEquipmentItemDataAsset* EquipmentItem = Cast<UEquipmentItemDataAsset>(itemInSlot);
	if (IsValid(EquipmentItem))
	{
		// If the item is passive, then it should be equipped immediately (chestplate, boots, etc.)
		if (EquipmentItem->bIsPassive)
		{
			SetEquipmentEnabled(SlotNumber);
		}
	}

}

/**
 * Toggles the equipment slot state, or forces the equipment to be enabled.
 * \param SlotNumber The InventoryComponent equipment slot
 * \param bMakeReady True forces equipment to enable. False alternates current state.
 */
void UEquipmentComponent::ToggleEquipment(int SlotNumber, bool bMakeReady)
{
	if (bMakeReady)
	{
		SetEquipmentEnabled(SlotNumber);
	}
	else
	{
		SetEquipmentDisabled(SlotNumber);
	}
}

/**
 * Sets the equivalent equipment slot to be enabled, performing a mesh merge
 * \param SlotNumber The InventoryComponent equipment slot
 */
void UEquipmentComponent::SetEquipmentEnabled(int SlotNumber)
{
	// Get the item that is currently in the equipment slot
	const UItemDataAsset* itemInSlot = GetInventoryReference()->GetItemDataInSlotNumber(SlotNumber);
	const UEquipmentItemDataAsset* equipmentItemData = Cast<UEquipmentItemDataAsset>(itemInSlot);

	if (IsValid(equipmentItemData))
	{
		// If a new actor is NOT spawned, it should be part of the mesh merge
		if (!equipmentItemData->AttachmentData.bSpawnActor)
		{
			const FGameplayTag slotTag = GetInventoryReference()->GetSlotEquipmentTag(SlotNumber);
			const FGameplayTag bodyTag = MeshMergeReference->GetBodyPartFromEquipmentSlot(slotTag);

			FMeshMergeMappings mergeMap = MeshMergeReference->GetMeshMappingFromIndex(
											MeshMergeReference->FindMeshMappingByTag(bodyTag) );

			FMeshMergeMappings newMapping = MeshMergeReference->CreateMeshMapping(equipmentItemData,
				GetInventoryReference()->GetSlotEquipmentTag(SlotNumber),	// Equipment Slot (i.e. Equipment.Slot.Primary)
				mergeMap.GameplayTags.HasTag(TAG_Character_Sex_Female));

			MeshMergeReference->AddMeshToMerge(newMapping);
			MeshMergeReference->PerformMeshMerge();
		}

		UE_LOGFMT(LogEquipment, Display, "{Name}({NetAuthority}): Equipment Item '{ItemName}' (Slot #{SlotNumber}) is READY / ARMED!"
			, GetOwner()->GetName(), GetOwner()->HasAuthority() ? "SERVER" : "CLIENT"
			, IsValid(itemInSlot) ? itemInSlot->GetItemDisplayNameAsString() : "(none)", SlotNumber);

		EquipmentSlots.Add(SlotNumber, true);
	}
	else
	{
		EquipmentSlots.Add(SlotNumber, false);
	}
	NotifySlotToggled(SlotNumber);
}


/**
 * Sets the equivalent equipment slot to be disabled/unused/unarmed
 * \param SlotNumber The InventoryComponent equipment slot
 */
void UEquipmentComponent::SetEquipmentDisabled(int SlotNumber)
{
	// Remove the item from the mesh merge, if present
	const UItemDataAsset* ItemInSlot = GetInventoryReference()->GetItemDataInSlotNumber(SlotNumber);
	const UEquipmentItemDataAsset* EquipData = Cast<UEquipmentItemDataAsset>(ItemInSlot);
	if (IsValid(EquipData))
	{
		if (!EquipData->AttachmentData.bSpawnActor)
		{
			const FGameplayTag SlotTag = GetInventoryReference()->GetSlotEquipmentTag(SlotNumber);
			MeshMergeReference->RemoveMeshFromMerge(nullptr, SlotTag);
			MeshMergeReference->PerformMeshMerge();
		}
	}

	UE_LOGFMT(LogEquipment, Display, "{Name}({NetAuthority}): Equipment Item '{ItemName}' (Slot #{SlotNumber}) is INACTIVE / STOWED."
		, GetOwner()->GetName(), GetOwner()->HasAuthority() ? "SERVER" : "CLIENT"
		, IsValid(ItemInSlot) ? ItemInSlot->GetItemDisplayNameAsString() : "(none)", SlotNumber);
	EquipmentSlots.Add(SlotNumber, false);
	NotifySlotToggled(SlotNumber);
}

/**
 * Adjusts the mesh of the equipment in relation to the character mesh
 * \param SlotNumber The InventoryComponent equipment slot
 * \param AttachParent The parent to attach to (opt)
 * \param AttachmentBone The name of the bone to attach to (opt)
 * \param AdjustmentTransform The transform adjustment to the equipment mesh (opt)
 */
void UEquipmentComponent::AdjustAttachment(int SlotNumber,
	AActor* AttachParent, FName AttachmentBone, FTransform AdjustmentTransform)
{
	if (!EquipmentSlots.Contains(SlotNumber))
	{
		InitEquipmentItem(SlotNumber);
	}

	const UItemDataAsset* ItemData = GetInventoryReference()->GetItemDataInSlotNumber(SlotNumber);
	const UEquipmentItemDataAsset* equipmentData = Cast<UEquipmentItemDataAsset>(ItemData);
	if (!IsValid(equipmentData))
	{
		UE_LOGFMT(LogEquipment, Warning, "Not Valid Equipment");
		return;
	}

	if (!equipmentData->AttachmentData.bSpawnActor)
	{
		if (!IsValid(MeshMergeReference))
		{
			const ACharacterBase* CharacterBase = Cast<ACharacterBase>( GetOwner() );
			MeshMergeReference = IsValid(CharacterBase) ? CharacterBase->MeshMergeComponent : nullptr;
		}

		if (IsValid(MeshMergeReference) && IsValid(GetInventoryReference()))
		{
			FInventorySlot SlotReference;
			GetInventoryReference()->GetInventorySlot(SlotNumber, SlotReference);
			if (SlotReference.SlotInventoryTag == TAG_Inventory_Slot_Equipment.GetTag())
			{
				if (IsValid(equipmentData))
				{
					for (auto& MeshMergeMap : MeshMergeReference->GetAllMeshMergeMappings())
					{
						FMeshMergeMappings meshMapping = MeshMergeReference->CreateMeshMapping(
							equipmentData, SlotReference.SlotEquipmentTag,
							MeshMergeMap.GameplayTags.HasTag(TAG_Character_Sex_Female) );

						MeshMergeReference->AddMeshToMerge(meshMapping);
					}
				}
			}
		}
	}
}

/**
 * Attempts to perform an attack with the specified slot number.
 * \param SlotNumber The InventoryComponent equipment slot. -1 attacks with all weapons.
 * \return True if the attack was successful, false otherwise.
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
	return EquipmentSlots.Contains(SlotNumber) ? EquipmentSlots[SlotNumber] : false;
}

bool UEquipmentComponent::IsOperating(int SlotNumber)
{
	const UInventoryComponent* InvReference = GetInventoryReference();
	if (InventoryReference)
	{
		if (InvReference->GetPrimarySlotNumber() == SlotNumber)
		{
			return bPrimaryOperating;
		}
		if (InvReference->GetSecondarySlotNumber() == SlotNumber)
		{
			return bSecondaryOperating;
		}
	}
	return false;
}

/**
 * Disables blocking if blocking, enables if not blocking.
 * \return True if blocking is enabled, false otherwise.
 */
bool UEquipmentComponent::ToggleBlocking()
{
	return false;
}

/**
 * Sets blocking to active, if possible.
 * \return True if blocking is active
 */
bool UEquipmentComponent::StartBlocking()
{
	return false;
}

/**
 * Sets blocking to inactive, if possible.
 * \return True if not blocking
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
 * \param SlotNumber The InventoryComponent equipment slot
 *					 If -1 (default), returns TRUE if any weapon is ready
 * \return True if weapon is armed/ready/in-use
 */
bool UEquipmentComponent::GetIsWeaponReady(int SlotNumber) const
{
	return false;
}

void UEquipmentComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UEquipmentComponent, bPrimaryArmed);
	DOREPLIFETIME(UEquipmentComponent, bSecondaryArmed);
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

void UEquipmentComponent::Multicast_EquipmentSlotToggled_Implementation(int SlotNumber, bool bValue)
{
	if (OnEquipmentSlotToggled.IsBound())
	{
		EquipmentSlots.Add(SlotNumber, bValue);
		OnEquipmentSlotToggled.Broadcast(SlotNumber, bValue);
	}
}

void UEquipmentComponent::NotifySlotToggled(int SlotNumber)
{
	if (EquipmentSlots.Contains(SlotNumber))
	{
		if (GetInventoryReference()->GetPrimarySlotNumber() == SlotNumber)
			bPrimaryArmed = true;

		if (GetInventoryReference()->GetSecondarySlotNumber() == SlotNumber)
			bSecondaryArmed = true;

		if (GetNetMode() < NM_Client)
		{
			if (OnEquipmentSlotToggled.IsBound())
			{
				OnEquipmentSlotToggled.Broadcast(SlotNumber, EquipmentSlots[SlotNumber]);
			}
			Multicast_EquipmentSlotToggled(SlotNumber, EquipmentSlots[SlotNumber]);
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

void UEquipmentComponent::OnRep_PrimaryArmed_Implementation()
{
	const int SlotNumber = GetInventoryReference()->GetPrimarySlotNumber();
	EquipmentSlots.Add(SlotNumber, bPrimaryArmed);
	OnEquipmentSlotToggled.Broadcast(SlotNumber, bPrimaryArmed);
}

void UEquipmentComponent::OnRep_SecondaryArmed_Implementation()
{
	const int SlotNumber = GetInventoryReference()->GetSecondarySlotNumber();
	EquipmentSlots.Add(SlotNumber, bSecondaryArmed);
	OnEquipmentSlotToggled.Broadcast(SlotNumber, bSecondaryArmed);
}
