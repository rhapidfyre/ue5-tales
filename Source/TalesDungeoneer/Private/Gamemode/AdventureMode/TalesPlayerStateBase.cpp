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
	ClassTagsMapped.Add(TAG_Character_Class_Warrior, nullptr);
	ClassTagsMapped.Add(TAG_Character_Class_Wizard, nullptr);
	
}

void ATalesPlayerStateBase::UpdatePlayerName()
{
	OnPlayerNameUpdated.Broadcast();
}

UDataAsset* ATalesPlayerStateBase::GetClassDataAsset() const
{
	FGameplayTag ClassTag = TAG_Character_Class_Warrior;
	const APlayerController* PlayerController = GetPlayerController();
	if (IsValid(PlayerController))
	{
		const ACharacterBase* CharacterBase = Cast<ACharacterBase>( PlayerController->GetPawn() );
		if (IsValid(CharacterBase))
		{
			ClassTag = CharacterBase->GetCharacterClass();
		}
	}
	return *ClassTagsMapped.Find(ClassTag);
}

UDataAsset* ATalesPlayerStateBase::GetRaceDataAsset() const
{
	FGameplayTag RaceTag = TAG_Character_Race_Human;
	const APlayerController* PlayerController = GetPlayerController();
	if (IsValid(PlayerController))
	{
		const ACharacterBase* CharacterBase = Cast<ACharacterBase>( PlayerController->GetPawn() );
		if (IsValid(CharacterBase))
		{
			RaceTag = CharacterBase->GetCharacterRace();
		}
	}
	return *RaceTagsMapped.Find(RaceTag);
}
