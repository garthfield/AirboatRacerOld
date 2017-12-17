#include "cbase.h"
#include "ar_player.h"
#include "ar_startline.h"
#include "in_buttons.h"
#include "vehicle_base.h"
#include "fmtstr.h"

LINK_ENTITY_TO_CLASS(player, CAR_Player);

static ConVar show_powerup("show_powerup", "1", 0, "toggles powerup");

CAR_Player::CAR_Player(void) {

}

CAR_Player::~CAR_Player(void)
{

}

void CAR_Player::Spawn(void)
{
	BaseClass::Spawn();
	CreateAirboat();

	// Send lap complete message via our player class
	CFmtStr str;
	hud_message msg;
	msg.type = "Lap";
	msg.valueString = str.sprintf("%d/%d", 1, ar_laps.GetInt());
	SendHudMessage(msg);
}

void CAR_Player::CreateAirboat(void)
{
	Vector vecForward;
	AngleVectors(EyeAngles(), &vecForward);
	CBaseEntity *pJeep = (CBaseEntity*)CreateEntityByName("prop_vehicle_airboat");
	if (pJeep)
	{
		Vector vecOrigin = GetAbsOrigin() + vecForward * 128;
		QAngle vecAngles(0, GetAbsAngles().y - 90, 0);
		pJeep->SetAbsOrigin(vecOrigin);
		pJeep->SetAbsAngles(vecAngles);
		pJeep->KeyValue("model", "models/airboat.mdl");
		pJeep->KeyValue("solid", "6");
		pJeep->KeyValue("targetname", "airboat");
		pJeep->KeyValue("vehiclescript", "scripts/vehicles/airboat.txt");
		DispatchSpawn(pJeep);
		pJeep->Activate();

		// Put driver inside vehicle immediately
		GetInVehicle(pJeep->GetServerVehicle(), VEHICLE_ROLE_DRIVER);

		// Create vehicle camera entity
		CBaseEntity *pJeepCamera = (CBaseEntity*)CreateEntityByName("ar_vehicle_camera");
		pJeepCamera->SetParent(pJeep);
		DispatchSpawn(pJeepCamera);

		// Reset animation to fix player angles and propeller animation
		CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>(pJeep);
		pAnimating->SetCycle(0);
		pAnimating->m_flAnimTime = gpGlobals->curtime;
		pAnimating->ResetSequence(0);
		pAnimating->ResetClientsideFrame();
		pAnimating->InvalidateBoneCache();
		CBaseServerVehicle *pServerVehicle = dynamic_cast<CBaseServerVehicle *>(pJeep->GetServerVehicle());
		pServerVehicle->GetDrivableVehicle()->SetVehicleEntryAnim(true);

		// Lock dirver inside vehicle
		CPropVehicleDriveable *driveable = dynamic_cast< CPropVehicleDriveable * >(pJeep);
		inputdata_t input;
		driveable->InputLock(input);
	}
}

void CAR_Player::PreThink(void)
{
	BaseClass::PreThink();
	
	// Check if player has powerup
	if (m_iPowerup)
	{
		// See if +attack what pressed in this frame
		if (m_nButtons & IN_ATTACK) {
			ExecutePowerup();
		}
	}
}

void CAR_Player::CreatePowerup()
{
	// Only allowed 1 pickup at a time
	if (m_iPowerup == NULL) {
		m_iPowerup = RandomInt(1, 2);
		DevMsg("CREATED POWER UP: %d", m_iPowerup);

		hud_message msg;
		msg.type = "Powerup";
		msg.valueByte = m_iPowerup;
		SendHudMessage(msg);
	}
}

void CAR_Player::ExecutePowerup()
{
	if (m_iPowerup == NULL)
		return;

	// Retrieve the vehicle the player is in
	CBaseEntity *pVehicle = GetVehicleEntity();
	if (pVehicle == NULL)
		return;

	DevMsg("PLAYER %d EXECUTED POWERUP %d\n", (entindex() - 1), m_iPowerup);

	QAngle angPushDir = pVehicle->GetAbsAngles();

	// Jump Powerup
	if (m_iPowerup == 1) {
		angPushDir.y += 90;
		angPushDir.x -= 15;
	}
	// Nitro Powerup
	else if (m_iPowerup == 2) {
		angPushDir.y += 90;
	}

	// Calculate vectors to push to
	Vector vecAbsDir;
	AngleVectors(angPushDir, &vecAbsDir);

	// Push vehicle towards that vector
	float m_flPushSpeed = 1000;
	pVehicle->ApplyAbsVelocityImpulse(m_flPushSpeed * vecAbsDir);

	// Send message to HUD setting the powerup to -1
	hud_message msg;
	msg.type = "Powerup";
	msg.valueByte = 0;
	SendHudMessage(msg);

	// Finished executing the power up now remove the powerup and player from the store
	m_iPowerup = NULL;
}

void CAR_Player::SendHudMessage(hud_message message)
{
	// Set message recipient
	CSingleUserRecipientFilter filter((CBasePlayer *)this);
	filter.MakeReliable();

	// Create message
	UserMessageBegin(filter, message.type);
	if (message.valueByte)
		WRITE_BYTE(message.valueByte);
	if (message.valueString)
		WRITE_STRING(message.valueString);

	// Send message
	MessageEnd(); //send message
}

/*

CFourWheelVehiclePhysics *pPhysics = pDrivable->GetPhysics();
IPhysicsVehicleController *pPhysicsVehicle = pPhysics->GetVehicle();
vehicleparams_t &vehicleParams = pPhysicsVehicle->GetVehicleParamsForChange();

// Increase max speed of vehicle
vehicleParams.engine.maxSpeed *= 2;

*/