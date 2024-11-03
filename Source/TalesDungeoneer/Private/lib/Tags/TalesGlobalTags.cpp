// Copyright Take Five Games, LLC 2023 - All Rights Reserved

#include "lib/Tags/TalesGlobalTags.h"


UE_DEFINE_GAMEPLAY_TAG(TAG_Respawners_Graveyard, "Actors.Respawn.Graveyard")
UE_DEFINE_GAMEPLAY_TAG(TAG_Respawners_Entrance, "Actors.Respawn.Entrance")


UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race,			    "Character.Race")

// Playable Races
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Human,	    "Character.Race.Human")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Dwarf,	    "Character.Race.Dwarf")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Elf,		    "Character.Race.Elf")

// NPC Races
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Orc,		    "Character.Race.Orc")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Undead,	    "Character.Race.Undead")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Construct,	"Character.Race.Construct")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Elemental,	"Character.Race.Elemental")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Beast,		"Character.Race.Beast")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Celestial,	"Character.Race.Celestial")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Fey,			"Character.Race.Fey")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Aberration,	"Character.Race.Aberration")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Dragon,	    "Character.Race.Dragon")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Ooze,		    "Character.Race.Ooze")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Planar,	    "Character.Race.Planar")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Plant,	    "Character.Race.Plant")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Race_Diety,	    "Character.Race.Diety")

UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Class,				"Character.Class")

// Playable Classes
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Class_Assassin,	"Character.Class.Assassin")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Class_Cleric,		"Character.Class.Cleric")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Class_Deviant,		"Character.Class.Deviant")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Class_Knight,		"Character.Class.Knight")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Class_Merc,		"Character.Class.Mercenary")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Class_Necro,		"Character.Class.Necromancer")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Class_Ranger,		"Character.Class.Ranger")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Class_Warrior,		"Character.Class.Warrior")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Class_Wizard,		"Character.Class.Wizard")

// NPC Classes
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Class_Noble,		"Character.Class.Noble")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Class_Performer,   "Character.Class.Performer")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Class_Crafter,     "Character.Class.Crafter")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Class_Commoner,    "Character.Class.Commoner")


UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body, 				"Character.Body")

// Used to specify the default body part for the character model
// It will be applied and made visible when the actor component is initialized
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Default, "Character.Body.Default")

UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Armor, 			 "Character.Body.Armor")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Armor_Shoulders, 	 "Character.Body.Armor.Shoulders")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Armor_Vambraces, 	 "Character.Body.Armor.Vambraces")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Armor_Gauntlets, 	 "Character.Body.Armor.Gauntlets")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Armor_Breastplate, "Character.Body.Armor.Breastplate")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Armor_Waist,		 "Character.Body.Armor.Waist")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Armor_Leggings,	 "Character.Body.Armor.Leggings")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Armor_Knees,		 "Character.Body.Armor.Knees")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Armor_Ankles,		 "Character.Body.Armor.Ankles")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Armor_Footwear,	 "Character.Body.Armor.Footwear")

UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Head, 			"Character.Body.Head")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Head_Ears, 		"Character.Body.Head.Ears")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Head_FacialHair, 	"Character.Body.Head.FacialHair")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Head_Hair, 		"Character.Body.Head.Hair")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Head_Head, 		"Character.Body.Head.Head")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Head_Eyes, 		"Character.Body.Head.Eyes")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Head_Eyebrow,		"Character.Body.Head.Eyebrow")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Head_Eyelashes,	"Character.Body.Head.Eyelashes")

UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Upper, 			"Character.Body.Upper")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Upper_Arms, 		"Character.Body.Upper.Arms")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Upper_Hand, 		"Character.Body.Upper.Hand")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Upper_Torso,		"Character.Body.Upper.Torso")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Upper_Underwear,	"Character.Body.Upper.Underwear")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Upper_Neck,		"Character.Body.Upper.Neck")

UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Lower, 			"Character.Body.Lower")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Lower_Legs, 		"Character.Body.Lower.Legs")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Lower_Feet, 		"Character.Body.Lower.Feet")
UE_DEFINE_GAMEPLAY_TAG(TAG_Character_Body_Lower_Underwear,	"Character.Body.Lower.Underwear")
