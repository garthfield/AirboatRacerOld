#ifndef CAR_PLAYER_H
#define CAR_PLAYER_H
#pragma once

class CAR_Player;

#include "hl2mp_player.h"

class CAR_Player : public CHL2MP_Player
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
	virtual void CreateAirboat(void);
	virtual void PreThink(void);
	void CreatePowerup();
	void ExecutePowerup(void);

private:
	int m_iPowerup;

};

#endif //CAR_PLAYER_H
