#include "cbase.h"
#include "vehicle_base.h"
#include "in_buttons.h"
#include "ar_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CAR_Powerup : public CBaseAnimating
{
public:
	DECLARE_CLASS(CAR_Powerup, CBaseAnimating);
	DECLARE_DATADESC();
	void Precache(void);
	void Spawn(void);
	void PowerupThink(void);
	virtual void StartTouch(CBaseEntity *pOther);
};

LINK_ENTITY_TO_CLASS(race_powerup, CAR_Powerup);

BEGIN_DATADESC(CAR_Powerup)
	DEFINE_ENTITYFUNC(StartTouch),
END_DATADESC()

void CAR_Powerup::Precache()
{
	BaseClass::Precache();
	PrecacheModel("models/pickup/pickup.mdl");
}

void CAR_Powerup::Spawn(void)
{
	Precache();

	UseClientSideAnimation();

	SetPlaybackRate(1.0);
	SetModel("models/pickup/pickup.mdl");
	ResetSequence(LookupSequence("Idle"));

	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	VPhysicsInitNormal(SOLID_BBOX, GetSolidFlags(), false);

	SetThink(&CAR_Powerup::PowerupThink);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CAR_Powerup::PowerupThink()
{
	StudioFrameAdvance();
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CAR_Powerup::StartTouch(CBaseEntity *pOther)
{
	if(stricmp(pOther->GetClassname(), "prop_vehicle_airboat") == 0) {
		DevMsg("POWERUP TOUCH: %s\n", pOther->GetClassname());
		
		CPropVehicleDriveable *pDrivable = dynamic_cast<CPropVehicleDriveable*>(pOther);
		if (pDrivable) {
			CBaseEntity *pDriver = pDrivable->GetDriver();
			if (pDriver) {
				CAR_Player *pPlayer = dynamic_cast<CAR_Player*>(pDriver);
				if (pPlayer) {
					pPlayer->CreatePowerup();
				}
			}
		}
	}

	BaseClass::StartTouch(pOther);
}
