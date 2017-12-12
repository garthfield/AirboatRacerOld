#ifndef CAR_STARTLINE_H
#define CAR_STARTLINE_H
#pragma once

#include "cbase.h"

extern ConVar ar_laps;

class CAR_StartlineEntity {
public:
	void Spawn(void);
	void StartTouch(CBaseEntity *pOther);
	void SetPlayerCheckpoint(int iPlayerIndex, int iCheckpoint);
};

#endif // CAR_STARTLINE_H