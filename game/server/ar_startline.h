#ifndef CAR_STARTLINE_H
#define CAR_STARTLINE_H
#pragma once

#define MAX_CHECKPOINTS 1000 // Total accumulative checkpoints a race can have

#include "cbase.h"
#include "triggers.h"
#include "simtimer.h"

extern ConVar ar_laps;

struct playerPoints {
	int index;
	int points;
};

struct playerPosition {
	int index;
	int position;
};

class CAR_StartlineEntity : public CTriggerMultiple
{
public:
	DECLARE_CLASS(CAR_StartlineEntity, CTriggerMultiple);

	void Spawn(void);
	void StartTouch(CBaseEntity *pOther);
	void SetPlayerCheckpoint(int iPlayerIndex, int iCheckpoint);
	void Think(void);
	void FinishPlayer(CBaseEntity *pPlayer);
	int GetTotalFinished(void);
	void ResetPositions(void);

private:
	void UpdatePlayerPositions(void);
	int GetCheckpointFinished(int iCheckpoint);
	int GetPlayerLaps(int iPlayerIndex);

	int	m_iPlayerCheckpoint[MAX_PLAYERS];				// Stores each player's current checkpoint
	int m_iPlayerCheckpointPosition[MAX_PLAYERS][MAX_CHECKPOINTS];	// Store finish position of each checkpoint
	bool m_bPlayerFinished[MAX_PLAYERS];				// Store each player as they finish only
	int m_iLastCheckpoint;
	int m_iRaceState;
	
	CSimpleStopwatch m_StopwatchWarmup;
	CSimpleStopwatch m_StopwatchFinish;


	DECLARE_DATADESC();
};

#endif // CAR_STARTLINE_H