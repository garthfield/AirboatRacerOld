#include "cbase.h"
#include "vehicle_base.h"
#include "in_buttons.h"

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
	void ExecutePowerup(int iPlayerIndex, CBasePlayer *pBasePlayer);
	void ExecuteThink(void);
	virtual void StartTouch(CBaseEntity *pOther);
private:
	int	m_iPlayerPowerup[MAX_PLAYERS];
	CBaseEntity *m_pPlayer[MAX_PLAYERS];
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

	BaseClass::Spawn();

	SetModel("models/pickup/pickup.mdl");
	ResetSequence(LookupSequence("Idle"));
	
	SetThink(&CAR_Powerup::PowerupThink);
	SetNextThink(gpGlobals->curtime + 0.1f);

	SetThink(&CAR_Powerup::ExecuteThink);
	SetNextThink(gpGlobals->curtime);

	AddSolidFlags(FSOLID_NOT_SOLID | FSOLID_TRIGGER);
	VPhysicsInitNormal(SOLID_BBOX, GetSolidFlags(), false);
}

void CAR_Powerup::PowerupThink()
{
	StudioFrameAdvance();
	DispatchAnimEvents(this);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CAR_Powerup::ExecuteThink()
{
	// Loop around each player
	for (int i = 0; i < MAX_PLAYERS; i++) {
		// Check if player has powerup
		if (m_iPlayerPowerup[i] && m_pPlayer[i])
		{
			// Retrieve BasePlayer entity
			CBasePlayer *pBasePlayer = (CBasePlayer *)m_pPlayer[i];

			// See if +attack what pressed in this frame
			if (pBasePlayer->m_nButtons & IN_ATTACK) {

				ExecutePowerup(i, pBasePlayer);
			}
		}
	}
	SetNextThink(gpGlobals->curtime);
}

void CAR_Powerup::ExecutePowerup(int iPlayerIndex, CBasePlayer *pBasePlayer)
{
	DevMsg("PLAYER %d EXECUTED POWERUP %d\n", iPlayerIndex, m_iPlayerPowerup[iPlayerIndex]);
	
	// Retrieve the vehicle the player is in
	CBaseEntity *pVehicle = pBasePlayer->GetVehicleEntity();
	if (pVehicle) {
		QAngle angPushDir = pVehicle->GetAbsAngles();

		// Jump Powerup
		if (m_iPlayerPowerup[iPlayerIndex] == 1) {
			angPushDir.y += 90;
			angPushDir.x -= 15;
		}
		// Nitro Powerup
		else if (m_iPlayerPowerup[iPlayerIndex] == 2) {
			angPushDir.y += 90;
		}

		// Calculate vectors to push to
		Vector vecAbsDir;
		AngleVectors(angPushDir, &vecAbsDir);

		// Push vehicle towards that vector
		float m_flPushSpeed = 1000;
		pVehicle->ApplyAbsVelocityImpulse(m_flPushSpeed * vecAbsDir);
	}
	
	// Send message to HUD setting the powerup to -1
	// Set message recipient
	CSingleUserRecipientFilter filter((CBasePlayer *)m_pPlayer[iPlayerIndex]);
	filter.MakeReliable();

	// Create message 
	UserMessageBegin(filter, "Powerup");
	WRITE_BYTE(0);

	// Send message
	MessageEnd(); //send message

	// Finished executing the power up now remove the powerup and player from the store
	m_iPlayerPowerup[iPlayerIndex] = NULL;
	m_pPlayer[iPlayerIndex] = NULL;
}

void CAR_Powerup::StartTouch(CBaseEntity *pOther)
{
	if(stricmp(pOther->GetClassname(), "prop_vehicle_airboat") == 0) {
		DevMsg("POWERUP TOUCH: %s\n", pOther->GetClassname());
		
		CPropVehicleDriveable *pDrivable = dynamic_cast<CPropVehicleDriveable*>(pOther);
		if (pDrivable) {

			// Retrieve driver
			CBaseEntity *pDriver = pDrivable->GetDriver();

			// Retrieve player index
			int iPlayerIndex = pDriver->entindex() - 1;
			
			// Check if they don't already have a power up
			if (m_iPlayerPowerup[iPlayerIndex] == NULL) {

				// Store the driver, to be used later for launching the powerup
				m_pPlayer[iPlayerIndex] = pDriver;

				// Give them a random power up
				m_iPlayerPowerup[iPlayerIndex] = RandomInt(1, 2);

				// Set message recipient
				CSingleUserRecipientFilter filter((CBasePlayer *)pDriver);
				filter.MakeReliable();

				// Create message 
				UserMessageBegin(filter, "Powerup");
				WRITE_BYTE(m_iPlayerPowerup[iPlayerIndex]);

				// Send message
				MessageEnd(); //send message

				DevMsg("POWERUP GIVEN: %d\n", m_iPlayerPowerup[iPlayerIndex]);
			}

			/*CBaseEntity *pDriver = pDrivable->GetDriver();
			if (pDriver) {

				// Set message recipient
				CSingleUserRecipientFilter filter((CBasePlayer *)pDriver);
				filter.MakeReliable();  // reliable transmission

				UserMessageBegin(filter, "SayText"); // create message 
				WRITE_STRING(lapInfo); // Fill message
				MessageEnd(); //send message
			}*/

			/*CFourWheelVehiclePhysics *pPhysics = pDrivable->GetPhysics();
			IPhysicsVehicleController *pPhysicsVehicle = pPhysics->GetVehicle();
			vehicleparams_t &vehicleParams = pPhysicsVehicle->GetVehicleParamsForChange();

			// Increase max speed of vehicle
			vehicleParams.engine.maxSpeed *= 2;*/
			
			/*QAngle angPushDir = pOther->GetAbsAngles();
			angPushDir.y += 90;
			angPushDir.x -= 15;

			Vector vecAbsDir;
			AngleVectors(angPushDir, &vecAbsDir);

			float m_flPushSpeed = 1000;
			pOther->ApplyAbsVelocityImpulse(m_flPushSpeed * vecAbsDir);*/

			/*Vector vecAbsDir;
			QAngle angPushDir = pOther->GetAbsAngles();
			AngleVectors(angPushDir, &vecAbsDir);

			Vector vecAbsDir;
			QAngle cameraAngles(0, pOther->GetAbsAngles().y + 90, 0);
			VectorRotate(pOther->GetAbsOrigin(), EntityToWorldTransform(), vecAbsDir);
			VectorIRotate(vecAbsDir, EntityToWorldTransform(), m_vecPushDir);

			float m_flPushSpeed = 100;
			pOther->ApplyAbsVelocityImpulse(m_flPushSpeed * vecAbsDir);*/

			/*Vector	velocity;
			IPhysicsObject *pPhys = pOther->VPhysicsGetObject();
			pPhys->ApplyForceCenter(m_flPushSpeed * vecAbsDir * 100.0f * gpGlobals->frametime);*/

			//DevMsg("VELOCITY: %.2f %.2f %.2f", velocity.x, velocity.y, velocity.x);

			// Change max speed
			//vehicleParams.engine.maxSpeed *= 2;

			/*vehicleParams.engine.boostDelay = 15;		// seconds before you can use it again
			vehicleParams.engine.boostDuration = 5;		// seconds to boost for
			vehicleParams.engine.boostForce = 1.5;		// 1.5 car body mass * gravity * inches / second ^ 2
			vehicleParams.engine.boostMaxSpeed = 64;	// Max speed you can reach when boosting
			vehicleParams.engine.torqueBoost = 0;		// enable "sprint" mode of vehicle, not	force type booster
			
			m_bBoostOn = true;
			SetThink(&CAR_Powerup::BoostThink);
			SetNextThink(gpGlobals->curtime);*/

			//DevMsg("THROTTLE: %.2f\n", pPhysics->GetThrottle());
		}
	}

	BaseClass::StartTouch(pOther);
}

