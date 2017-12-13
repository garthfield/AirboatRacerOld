#include "cbase.h"
#include "ar_player.h"
#include "vehicle_jeep.h"

LINK_ENTITY_TO_CLASS(player, CAR_Player);

void CAR_Player::Spawn(void)
{
	BaseClass::Spawn();
	
	CreateAirboat();

	/*CBasePlayer *pPlayer = UTIL_PlayerByUserId(GetUserID());
	if (!pPlayer)
		return;

	// Cheat to create a jeep in front of the player
	Vector vecForward;
	AngleVectors(pPlayer->EyeAngles(), &vecForward);
	CBaseEntity *pJeep = (CBaseEntity*)CreateEntityByName("prop_vehicle_airboat");
	if (pJeep)
	{
		Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 128;
		QAngle vecAngles(0, pPlayer->GetAbsAngles().y - 90, 0);
		pJeep->SetAbsOrigin(vecOrigin);
		pJeep->SetAbsAngles(vecAngles);
		pJeep->KeyValue("model", "models/airboat.mdl");
		pJeep->KeyValue("solid", "6");
		pJeep->KeyValue("targetname", "airboat");
		pJeep->KeyValue("vehiclescript", "scripts/vehicles/airboat.txt");
		DispatchSpawn(pJeep);
		pJeep->Activate();

		CBaseEntity *pJeepCamera = (CBaseEntity*)CreateEntityByName("point_viewcontrol");
		if (pJeepCamera) {
			Vector cameraOrigin = pPlayer->GetAbsOrigin() + vecForward * 116;
			cameraOrigin.z += 62;
			QAngle cameraAngles(0, pPlayer->GetAbsAngles().y, 0);
			pJeepCamera->SetAbsOrigin(cameraOrigin);
			pJeepCamera->SetAbsAngles(cameraAngles);
			pJeepCamera->SetParent(pJeep);
			DispatchSpawn(pJeepCamera);

			variant_t emptyVariant;
			pJeepCamera->AcceptInput("Enable", pPlayer, pPlayer, emptyVariant, 0);
			pPlayer->SetViewEntity(pJeepCamera);
		}

		// Put driver inside vehicle immediately
		pPlayer->GetInVehicle(pJeep->GetServerVehicle(), VEHICLE_ROLE_DRIVER);

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

	}*/
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
	}
}


