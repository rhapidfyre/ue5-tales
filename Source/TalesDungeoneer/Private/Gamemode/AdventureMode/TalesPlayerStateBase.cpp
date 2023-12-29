// Fill out your copyright notice in the Description page of Project Settings.

#include "Gamemode/AdventureMode/TalesPlayerStateBase.h"

#include "Characters/CharacterBase.h"


ATalesPlayerStateBase::ATalesPlayerStateBase()
{
	
	RaceTagsMapped.Add(TAG_Character_Race_Human, nullptr);
	RaceTagsMapped.Add(TAG_Character_Race_Elf, nullptr);
	RaceTagsMapped.Add(TAG_Character_Race_Dwarf, nullptr);
	
	ClassTagsMapped.Add(TAG_Character_Class_Assassin, nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Cleric, nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Deviant, nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Knight, nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Merc, nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Necro, nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Knight, nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Ranger, nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Warrior, nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Wizard, nullptr);
	
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
