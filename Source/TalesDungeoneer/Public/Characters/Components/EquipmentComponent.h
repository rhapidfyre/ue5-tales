// Starcache Studios, LLC (c) 2024

#pragma once

#include "CoreMinimal.h"
#include "InventoryComponent.h"
#include "MeshMergeComponent.h"
#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"

#include "EquipmentComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipmentSlotToggled, int, SlotNumber, bool, bIsEnabled);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TALESDUNGEONEER_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

#pragma region ** Events & Methods **


public: //public events

	// Called when an equipment slot is enabled or disabled
	UPROPERTY(BlueprintAssignable) FOnEquipmentSlotToggled OnEquipmentSlotToggled;

	// Sets default values for this component's properties
	UEquipmentComponent();

	// Use this to call the inventory each time. If the inventory reference is invalid, it will initiate it.
	UInventoryComponent* GetInventoryReference();

	// Looks for the corresponding equipment slot, then adds/removes the equipment
	// slot as appropriate, and manages the mesh/appearance of the item.
	void InitEquipmentItem(int SlotNumber);

	// Dons or doffs the given equipment slot number
	// TODO - Will probably bridge the inventory system with the mesh merge component
	UFUNCTION(BlueprintCallable)
	void ToggleEquipment(int SlotNumber, bool bMakeReady = true);

	// Dons the equipment at the given equipment slot number
	// TODO - Will probably bridge the inventory system with the mesh merge component
	UFUNCTION(BlueprintCallable)
	void SetEquipmentEnabled(int SlotNumber);

	// Doffs the equipment at the given equipment slot number
	// TODO - Will probably bridge the inventory system with the mesh merge component
	UFUNCTION(BlueprintCallable)
	void SetEquipmentDisabled(int SlotNumber);

	// Makes adjustments to the visuals of the equipment
	UFUNCTION(BlueprintCallable)
	void AdjustAttachment(int SlotNumber, AActor* AttachParent = nullptr,
		FName AttachmentBone = "", FTransform AdjustmentTransform = FTransform());

	UFUNCTION(BlueprintCallable) bool PerformAttack(int SlotNumber);

	UFUNCTION(Server, Reliable)
	void Server_PerformAttack(int SlotNumber);

	UFUNCTION(BlueprintPure)		TMap<int, bool> GetAllEquipment() const;

	UFUNCTION(BlueprintPure)		bool GetIsReady(int SlotNumber) const;

	// Returns TRUE if the equipment is currently operating (animating, changing slots, etc.) and shouldn't respond to changes
	UFUNCTION(BlueprintPure)		bool IsOperating(int SlotNumber);

	UFUNCTION(BlueprintPure)		bool GetIsArmed() const { return bBlocking; }
	UFUNCTION(BlueprintPure)		bool GetIsBlocking() const { return bBlocking; }

	UFUNCTION(BlueprintCallable)	bool ToggleBlocking();
	UFUNCTION(BlueprintCallable)	bool StartBlocking();
	UFUNCTION(BlueprintCallable)	bool StopBlocking();

	UFUNCTION(Server, Reliable)		void Server_ToggleBlocking();
	UFUNCTION(Server, Reliable)		void Server_StartBlocking();
	UFUNCTION(Server, Reliable)		void Server_StopBlocking();

	UFUNCTION(BlueprintPure)	bool GetIsWeaponReady(int SlotNumber = -1) const;


protected: // protected methods

	virtual void GetLifetimeReplicatedProps(
		TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnComponentCreated() override;

	virtual void NotifySlotToggled(int SlotNumber);


private: // private methods

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EquipmentSlotToggled(int SlotNumber, bool bValue);

	UFUNCTION()	void ResetAttackCooldown();

#pragma endregion


public: // public members

	UPROPERTY(BlueprintReadWrite) bool bCanBlock = true;

private: // private members


	// If this timer is active, draw/sheathe/attack will not work
	UPROPERTY() FTimerHandle AttackCooldown_;

	UPROPERTY() UInventoryComponent* InventoryReference = nullptr;
	UPROPERTY() UMeshMergeComponent* MeshMergeReference = nullptr;


	// Slots containing all equipment items (chest plate, etc)
	TMap<int, bool> EquipmentSlots = {};

	UFUNCTION(NetMulticast, Unreliable) void OnRep_PrimaryArmed();
	UPROPERTY(ReplicatedUsing=OnRep_PrimaryArmed) bool bPrimaryArmed = false;
	bool bPrimaryOperating = false;

	UFUNCTION(NetMulticast, Unreliable) void OnRep_SecondaryArmed();
	UPROPERTY(ReplicatedUsing=OnRep_SecondaryArmed) bool bSecondaryArmed = false;
	bool bSecondaryOperating = false;

	bool bBlocking      = true; // TRUE if a blocking device is enabled
	bool bArmed			= true; // TRUE if a weapon is ready

#ifdef UE_BUILD_DEBUG

	bool bShowDebug		= true;
	bool bVerboseOutput = true;

#endif


};
