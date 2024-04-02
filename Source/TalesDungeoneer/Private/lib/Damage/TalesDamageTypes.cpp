// Take Five Games, LLC


#include "lib/Damage/TalesDamageTypes.h"

UTalesDamageBase::UTalesDamageBase()
	: DamageValue(0.f)
	, DamageRate(0.f)
	, DamageTypeTag(TAG_Damage.GetTag())
	, DamageTags({})
{
	
}
