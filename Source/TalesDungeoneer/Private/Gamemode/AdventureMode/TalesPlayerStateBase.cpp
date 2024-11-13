// Starcache Studios, LLC (c) 2024

#include "Gamemode/AdventureMode/TalesPlayerStateBase.h"

#include "Characters/CharacterBase.h"


ATalesPlayerStateBase::ATalesPlayerStateBase()
{

	RaceTagsMapped.Add(TAG_Character_Race_Human.GetTag(), nullptr);
	RaceTagsMapped.Add(TAG_Character_Race_Elf.GetTag(), nullptr);
	RaceTagsMapped.Add(TAG_Character_Race_Dwarf.GetTag(), nullptr);

	ClassTagsMapped.Add(TAG_Character_Class_Assassin.GetTag(), nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Cleric.GetTag(), nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Deviant.GetTag(), nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Knight.GetTag(), nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Merc.GetTag(), nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Necro.GetTag(), nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Knight.GetTag(), nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Ranger.GetTag(), nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Warrior.GetTag(), nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Wizard.GetTag(), nullptr);

}

void ATalesPlayerStateBase::UpdatePlayerName()
{
	OnPlayerNameUpdated.Broadcast();
}

UDataAsset* ATalesPlayerStateBase::GetClassDataAsset(const FGameplayTag& ClassTag) const
{
	// Use the tag provided, if it exists
	if (ClassTag.GetGameplayTagParents().HasTag(TAG_Character_Class.GetTag()))
	{
		if (ClassTagsMapped.Contains(ClassTag))
		{
			return *ClassTagsMapped.Find(ClassTag);
		}
	}
	// Otherwise use the existing player character
	const APlayerController* PlayerController = GetPlayerController();
	if (IsValid(PlayerController))
	{
		const ACharacterBase* CharacterBase = Cast<ACharacterBase>( PlayerController->GetPawn() );
		if (IsValid(CharacterBase))
		{
			return *ClassTagsMapped.Find( CharacterBase->GetCharacterClass() );
		}
	}
	return nullptr;
}

TArray<FGameplayTag> ATalesPlayerStateBase::GetAllCharacterClasses() const
{
	TArray<FGameplayTag> allClassTags;
	ClassTagsMapped.GetKeys(allClassTags);
	return allClassTags;
}

UDataAsset* ATalesPlayerStateBase::GetRaceDataAsset(const FGameplayTag& RaceTag) const
{
	// Use the tag provided, if it exists
	if (RaceTag.GetGameplayTagParents().HasTag(TAG_Character_Race.GetTag()))
	{
		if (RaceTagsMapped.Contains(RaceTag))
		{
			return *RaceTagsMapped.Find(RaceTag);
		}
	}
	// Otherwise use the existing player character
	const APlayerController* PlayerController = GetPlayerController();
	if (IsValid(PlayerController))
	{
		const ACharacterBase* CharacterBase = Cast<ACharacterBase>( PlayerController->GetPawn() );
		if (IsValid(CharacterBase))
		{
			return *RaceTagsMapped.Find( CharacterBase->GetCharacterRace() );
		}
	}
	return nullptr;
}

TArray<FGameplayTag> ATalesPlayerStateBase::GetAllCharacterRaces() const
{
	TArray<FGameplayTag> allRaceTags;
	RaceTagsMapped.GetKeys(allRaceTags);
	return allRaceTags;
}
