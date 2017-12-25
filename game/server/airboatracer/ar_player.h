#ifndef CAR_PLAYER_H
#define CAR_PLAYER_H
#pragma once

#include "ar_definitions.h"
#include "ar_startline.h"
#include "hl2mp_player.h"
#include "simtimer.h"
#include "GameEventListener.h"

class CAR_Player;

struct raceLeaders {
	int index;
	int position;
};

class CAR_Player : public CHL2MP_Player, public CGameEventListener
{

public:
	DECLARE_CLASS(CAR_Player, CHL2MP_Player);

	CAR_Player();
	~CAR_Player(void);

	static CAR_Player *CreatePlayer(const char *className, edict_t *ed)
	{
		CAR_Player::s_PlayerEdict = ed;
		return (CAR_Player*)CreateEntityByName(className);
	}

	virtual void Spawn(void);
	virtual void FireGameEvent(IGameEvent *event);
	void MoveStartPosition(void);
	virtual void CreateAirboat(void);
	void AirboatDriverEntry(void);
	void CreateAirboatPositionSprite(int icon);
	virtual void PreThink(void);
	void CreatePowerup();
	void ExecutePowerup(void);
	void SetEngineMaxSpeed(float multiplier);
	void ChangeCamera(void);
	void SendHudLapMsg(char *msg);
	void SendHudPowerupMsg(int iPowerup);
	void SendHudLeaders(playerPosition positions[33]);
	virtual CBaseEntity* EntSelectSpawnPoint(void);
	CBaseEntity *m_pAirboat;
	CBaseEntity *m_pVehicleCameras[AR_CAMERA_TOTAL];

private:
	int m_iPowerup;
	float m_fOriginalMaxSpeed;
	Vector m_DeathOrigin;
	CSimpleStopwatch m_StopwatchPowerupThree;
	int m_nCurrentVehicleCamera;

};

inline CAR_Player *ToARPlayer(CBaseEntity *pEntity)
{
	if (!pEntity || !pEntity->IsPlayer())
		return NULL;

	return dynamic_cast<CAR_Player*>(pEntity);
}

#endif //CAR_PLAYER_H
