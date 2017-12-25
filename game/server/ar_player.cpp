#include "cbase.h"
#include "ar_player.h"
#include "in_buttons.h"
#include "vehicle_base.h"
#include "globalstate.h"
#include "ar_game.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern CBaseEntity				*g_pLastSpawn;

LINK_ENTITY_TO_CLASS(player, CAR_Player);

void CC_SetIcon(void) {
	DevMsg("CC_SetIcon\n");

	CBasePlayer* pPlayer = UTIL_GetCommandClient();

	if (pPlayer) {
		DevMsg("CC_SetIcon BasePlayer %d\n", pPlayer->entindex());
		CAR_Player *pARPlayer = dynamic_cast<CAR_Player*>(pPlayer);
		if (pARPlayer) {
			DevMsg("CC_SetIcon pARPlayer\n");
			/*variant_t emptyInput;
			pARPlayer->m_pPositionSprite->AcceptInput("HideSprite", pPlayer, pPlayer, emptyInput, 0);
			pARPlayer->m_pPositionSprite->AcceptInput("ShowSprite", pPlayer, pPlayer, emptyInput, 0);
			pARPlayer->m_pPositionSprite->AcceptInput("Kill", pPlayer, pPlayer, emptyInput, 0);*/

			pARPlayer->CreateAirboatPositionSprite(7);
		}
	}
}
static ConCommand race_seticon("race_seticon", CC_SetIcon, "Set vehicle icon.");


void CC_ClientList(void) {

	CBasePlayer *pPlayer;

	Msg("MAX_PLAYERS METHOD\n");
	for (int i = 0; i < MAX_PLAYERS; i++) {
		pPlayer = UTIL_PlayerByIndex(i);
		if (!pPlayer)
			continue;

		Msg("i %d entindex %d\n", i, pPlayer->entindex());
	}

	Msg("maxClients METHOD\n");
	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer) {
			Msg("i %d entindex %d\n", i, pPlayer->entindex());
		}
	}

}
static ConCommand race_clientlist("race_clientlist", CC_ClientList, "List clients.");

void CC_StartPosition(void) {

	//DevMsg("TOTAL PLAYERS: %d\n", Game()->GetTotalPlayers());

	/*CBasePlayer* pPlayer = UTIL_GetCommandClient();
	if (pPlayer) {
		DevMsg("CC_StartPosition BasePlayer %d\n", pPlayer->entindex());
		CAR_Player *pARPlayer = dynamic_cast<CAR_Player*>(pPlayer);
		if (pARPlayer) {
			DevMsg("CC_StartPosition pARPlayer\n");
			pARPlayer->MoveStartPosition();
			HL2MPRules()->CleanUpMap();
			pARPlayer->Spawn();
		}
	}*/

}
static ConCommand race_startposition("race_startposition", CC_StartPosition, "Respawn.");


CAR_Player::CAR_Player(void) {
	ListenForGameEvent("player_disconnect");
	ListenForGameEvent("player_death");
}

CAR_Player::~CAR_Player(void)
{
	m_pAirboat = NULL;
}

void CAR_Player::Spawn(void)
{
	BaseClass::Spawn();

	DevMsg("CAR_Player::Spawn\n");

	// When players die we throw them out of the vehicle, we keep the airboat alive
	if (m_pAirboat == NULL) {
		CreateAirboat();
	}
	// Put driver back into airboat on respawn
	else {
		AirboatDriverEntry();
	}

	PrecacheModel("materials/sprites/positions/1.vmt");
	PrecacheModel("materials/sprites/positions/2.vmt");
	PrecacheModel("materials/sprites/positions/3.vmt");
	PrecacheModel("materials/sprites/positions/4.vmt");
	PrecacheModel("materials/sprites/positions/5.vmt");
	PrecacheModel("materials/sprites/positions/6.vmt");
	PrecacheModel("materials/sprites/positions/7.vmt");
	PrecacheModel("materials/sprites/positions/8.vmt");
	PrecacheModel("materials/sprites/positions/9.vmt");
	PrecacheModel("materials/sprites/positions/10.vmt");

	// Send initial lap message
	char msg[10];
	Q_snprintf(msg, sizeof(msg), "%d/%d", 1, ar_laps.GetInt());
	SendHudLapMsg(msg);

	// Send message to HUD clearing powerup
	SendHudPowerupMsg(0);
}

void CAR_Player::FireGameEvent(IGameEvent *event)
{
	// Player Disconnect - Remove Airboat & Rankings
	if (!Q_strncmp(event->GetName(), "player_disconnect", Q_strlen("player_disconnect"))) {
		DevMsg("PLAYER DISCONNECT\n");
		CBasePlayer *pPlayer = UTIL_PlayerByUserId(event->GetInt("userid"));
		if (pPlayer) {
			CBaseEntity *pVehicle = pPlayer->GetVehicleEntity();
			if (pVehicle) {
				pVehicle->Remove();
			}
		}
	}

	// Player Death - Take player out of airboat for death effect
	else if (!Q_strncmp(event->GetName(), "player_death", Q_strlen("player_death"))) {
		CBasePlayer *pPlayer = UTIL_PlayerByUserId(event->GetInt("userid"));
		if (pPlayer) {
			CBaseEntity *pVehicle = pPlayer->GetVehicleEntity();
			m_DeathOrigin = pVehicle->GetAbsOrigin();
			if (pVehicle) {
				LeaveVehicle();
			}
		}
	}

	// Player
}

void CAR_Player::MoveStartPosition()
{
	if (m_pAirboat) {
		LeaveVehicle();
		m_pAirboat->Remove();
		m_pAirboat = NULL;
	}

	//Spawn();
}

void CAR_Player::CreateAirboat(void)
{
	Vector vecForward;
	AngleVectors(EyeAngles(), &vecForward);
	m_pAirboat = (CBaseEntity*)CreateEntityByName("prop_vehicle_airboat");
	if (m_pAirboat)
	{
		Vector vecOrigin = GetAbsOrigin() + vecForward * 128;
		QAngle vecAngles(0, GetAbsAngles().y - 90, 0);
		m_pAirboat->SetAbsOrigin(vecOrigin);
		m_pAirboat->SetAbsAngles(vecAngles);
		m_pAirboat->KeyValue("model", "models/airboat.mdl");
		m_pAirboat->KeyValue("model", "models/airboat.mdl");
		m_pAirboat->KeyValue("solid", "6");
		m_pAirboat->KeyValue("targetname", "airboat");
		m_pAirboat->KeyValue("vehiclescript", "scripts/vehicles/airboat.txt");
		DispatchSpawn(m_pAirboat);
		m_pAirboat->Activate();

		// Create seat camera entity
		m_pVehicleCameras[0] = (CBaseEntity*)CreateEntityByName("ar_vehicle_camera");
		m_pVehicleCameras[0]->SetParent(m_pAirboat);
		m_pVehicleCameras[0]->AddSpawnFlags(AR_CAMERA_TYPE_SEAT);
		DispatchSpawn(m_pVehicleCameras[0]);

		// Create birdseye camera entity
		m_pVehicleCameras[1] = (CBaseEntity*)CreateEntityByName("ar_vehicle_camera");
		m_pVehicleCameras[1]->SetParent(m_pAirboat);
		m_pVehicleCameras[1]->AddSpawnFlags(AR_CAMERA_TYPE_BIRDSEYE);
		DispatchSpawn(m_pVehicleCameras[1]);

		// Create front camera entity
		m_pVehicleCameras[2] = (CBaseEntity*)CreateEntityByName("ar_vehicle_camera");
		m_pVehicleCameras[2]->SetParent(m_pAirboat);
		m_pVehicleCameras[2]->AddSpawnFlags(AR_CAMERA_TYPE_FRONT);
		DispatchSpawn(m_pVehicleCameras[2]);

		// Create back camera entity
		m_pVehicleCameras[3] = (CBaseEntity*)CreateEntityByName("ar_vehicle_camera");
		m_pVehicleCameras[3]->SetParent(m_pAirboat);
		m_pVehicleCameras[3]->AddSpawnFlags(AR_CAMERA_TYPE_BACK);
		DispatchSpawn(m_pVehicleCameras[3]);

		AirboatDriverEntry();
	}
}

void CAR_Player::AirboatDriverEntry(void) {
	if (m_pAirboat) {

		// Put driver inside vehicle immediately
		GetInVehicle(m_pAirboat->GetServerVehicle(), VEHICLE_ROLE_DRIVER);

		// Activate vehicle camera
		m_nCurrentVehicleCamera = 3;
		ChangeCamera();

		// Reset animation to fix player angles and propeller animation
		CBaseAnimating *pAnimating = dynamic_cast<CBaseAnimating *>(m_pAirboat);
		pAnimating->SetCycle(0);
		pAnimating->m_flAnimTime = gpGlobals->curtime;
		pAnimating->ResetSequence(0);
		pAnimating->ResetClientsideFrame();
		pAnimating->InvalidateBoneCache();
		CBaseServerVehicle *pServerVehicle = dynamic_cast<CBaseServerVehicle *>(m_pAirboat->GetServerVehicle());
		pServerVehicle->GetDrivableVehicle()->SetVehicleEntryAnim(true);

		// Lock dirver inside vehicle
		/*CPropVehicleDriveable *driveable = dynamic_cast< CPropVehicleDriveable * >(pJeep);
		inputdata_t input;
		driveable->InputLock(input);*/
	}
}

void CAR_Player::CreateAirboatPositionSprite(int icon)
{
	if (m_pAirboat) {

		CBaseEntity *pChild = m_pAirboat->FirstMoveChild();
		while (pChild) {
			CBaseEntity *pEntity = pChild;
			if (stricmp(pEntity->GetClassname(), "env_sprite") == 0) {
				pEntity->Remove();
			}
			pChild = pChild->NextMovePeer();
		}

		CBaseEntity *pPlayer = GetBaseEntity();
		DevMsg("CREATING SPRITE: %d FOR PLAYER %d AIRBOAT %d\n", icon, pPlayer->entindex(), m_pAirboat->entindex());

		char modelName[60];
		Q_snprintf(modelName, sizeof(modelName), "materials/sprites/positions/%d.vmt", icon);

		Vector origin = m_pAirboat->GetAbsOrigin();
		origin.z += 180;

		CBaseEntity *pSpriteEntity = CreateEntityByName("env_sprite");
		pSpriteEntity->KeyValue("model", modelName);
		pSpriteEntity->SetAbsOrigin(origin);
		pSpriteEntity->SetRenderMode(kRenderTransColor);
		pSpriteEntity->SetParent(m_pAirboat);
		DispatchSpawn(pSpriteEntity);
	}
}

void CAR_Player::PreThink(void)
{
	BaseClass::PreThink();
	
	// See if +attack was pressed in this frame
	if (m_afButtonReleased & IN_ATTACK && m_iPowerup) {
		DevMsg("EXECUTE POWER UP\n");
		ExecutePowerup();
	}
	
	// See if +attack2 was pressed
	if (m_afButtonReleased & IN_ATTACK2) {
		DevMsg("CHANGE CAMERA\n");
		ChangeCamera();
	}

	// Powerup Three Stopwatch
	if (m_StopwatchPowerupThree.IsRunning()) {
		DevMsg("POWERUP 3 TIMER: %.2f\n", m_StopwatchPowerupThree.GetRemaining());
		if (m_StopwatchPowerupThree.Expired()) {
			m_StopwatchPowerupThree.Stop();
			SetEngineMaxSpeed(0.0f);
		}
	}

}

void CAR_Player::CreatePowerup()
{
	// Only allowed 1 pickup at a time
	if (m_iPowerup == NULL) {
		m_iPowerup = RandomInt(1, 3);
		DevMsg("CREATED POWER UP: %d", m_iPowerup);
		SendHudPowerupMsg(m_iPowerup);
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
	if (m_iPowerup == 1 || m_iPowerup == 2) {
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
	}
	else if (m_iPowerup == 3) {

		// Can't run multiple max speeds at the same time
		if (m_StopwatchPowerupThree.IsRunning() == false) {
			CPropVehicleDriveable *pDrivable = dynamic_cast<CPropVehicleDriveable *>(pVehicle);
			CFourWheelVehiclePhysics *pPhysics = pDrivable->GetPhysics();
			IPhysicsVehicleController *pPhysicsVehicle = pPhysics->GetVehicle();
			vehicleparams_t &vehicleParams = pPhysicsVehicle->GetVehicleParamsForChange();

			// Increase max speed of vehicle
			m_fOriginalMaxSpeed = vehicleParams.engine.maxSpeed;
			vehicleParams.engine.maxSpeed *= 2;

			// Only let them use it for 10 seconds
			m_StopwatchPowerupThree.Start(10);
		}
	}

	// GEnable Gun
	/*variant_t input;
	input.SetBool(true);
	pVehicle->AcceptInput("EnableGun", pPlayer, pPlayer, input, 0);*/


	// Send message to HUD clearing powerup
	SendHudPowerupMsg(0);

	// Finished executing the power up now remove the powerup and player from the store
	m_iPowerup = NULL;
}

// Increase engine max speed with multiplier or 0 to reset to original max speed
void CAR_Player::SetEngineMaxSpeed(float multiplier) {

	CBaseEntity *pVehicle = GetVehicleEntity();
	if (pVehicle == NULL)
		return;

	CPropVehicleDriveable *pDrivable = dynamic_cast< CPropVehicleDriveable * >(pVehicle);
	CFourWheelVehiclePhysics *pPhysics = pDrivable->GetPhysics();
	IPhysicsVehicleController *pPhysicsVehicle = pPhysics->GetVehicle();
	vehicleparams_t &vehicleParams = pPhysicsVehicle->GetVehicleParamsForChange();

	// Store original max speed
	if (m_fOriginalMaxSpeed == NULL)
		m_fOriginalMaxSpeed = (float)vehicleParams.engine.maxSpeed;

	// Increase max speed of vehicle
	if (multiplier > 0)
		vehicleParams.engine.maxSpeed *= multiplier;
	else
		vehicleParams.engine.maxSpeed = m_fOriginalMaxSpeed;
}

void CAR_Player::ChangeCamera()
{
	if (m_pAirboat) {

		CBaseEntity *pPlayer = GetBaseEntity();
		
		// Change camera
		int iNextCamera = m_nCurrentVehicleCamera + 1;
		iNextCamera = iNextCamera > (AR_CAMERA_TOTAL - 1) ? 0 : iNextCamera;

		DevMsg("CAMERA CURRENT %d NEXT %d\n", m_nCurrentVehicleCamera, iNextCamera);

		variant_t emptyInput;
		m_pVehicleCameras[iNextCamera]->AcceptInput("Enable", pPlayer, pPlayer, emptyInput, 0);

		m_nCurrentVehicleCamera = iNextCamera;
	}
}

void CAR_Player::SendHudLapMsg(char *message)
{
	CSingleUserRecipientFilter filter((CBasePlayer *)this);
	filter.MakeReliable();
	UserMessageBegin(filter, "Lap");
		WRITE_STRING(message);
	MessageEnd();
}

void CAR_Player::SendHudPowerupMsg(int iPowerup)
{
	CSingleUserRecipientFilter filter((CBasePlayer *)this);
	filter.MakeReliable();
	UserMessageBegin(filter, "Powerup");
		WRITE_BYTE(iPowerup);
	MessageEnd();
}

void CAR_Player::SendHudLeaders(playerPosition *positions)
{
	DevMsg("CAR_Player::SendHudLeaders\n");
	CSingleUserRecipientFilter filter((CBasePlayer *)this);
	filter.MakeReliable();
	UserMessageBegin(filter, "Leaders");
	for (int i = 0; i < MAX_PLAYERS; i++) {
		WRITE_BYTE(positions[i].index);
		WRITE_BYTE(positions[i].position);
	}
	MessageEnd();
}

CBaseEntity* CAR_Player::EntSelectSpawnPoint(void) {
	DevMsg("CAR_Player::EntSelectSpawnPoint\n");
	
	DevMsg("CAR_Player::EntSelectSpawnPoint death orgin zero: %d\n", m_DeathOrigin.IsZero());

	if (m_DeathOrigin.IsValid()) {
		//DevMsg("CAR_Player::EntSelectSpawnPoint death orgin valid: %d\n", m_DeathOrigin.IsValid());
	}

	CBaseEntity *pSpot = NULL;
	while ((pSpot = gEntList.FindEntityByClassname(pSpot, "info_player_start")) != NULL) {
		if (g_pGameRules->IsSpawnPointValid(pSpot, this)) {
			g_pLastSpawn = pSpot;
			return pSpot;
		}
	}

	return gEntList.FindEntityByClassname(pSpot, "info_player_start");
}

/*

CFourWheelVehiclePhysics *pPhysics = pDrivable->GetPhysics();
IPhysicsVehicleController *pPhysicsVehicle = pPhysics->GetVehicle();
vehicleparams_t &vehicleParams = pPhysicsVehicle->GetVehicleParamsForChange();

// Increase max speed of vehicle
vehicleParams.engine.maxSpeed *= 2;

*/