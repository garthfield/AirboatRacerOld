#include "cbase.h"
#include "ar_startline.h"
#include "ar_player.h"
#include "vehicle_base.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(r_startline, CAR_StartlineEntity);

BEGIN_DATADESC(CAR_StartlineEntity)
	DEFINE_KEYFIELD(m_iLastCheckpoint, FIELD_INTEGER, "lastCheck"),
END_DATADESC()

ConVar ar_laps("race_laps", "3", FCVAR_NOTIFY, "Set the number of laps each race is", true, 1, false, 0);

void CAR_StartlineEntity::Spawn()
{
	Msg("Spawned Startline: Last Checkpoint: %d\n", m_iLastCheckpoint);
	BaseClass::Spawn();

	ResetPositions();

	SetThink(&CAR_StartlineEntity::Think);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CAR_StartlineEntity::Think()
{
	if (m_StopwatchFinish.IsRunning()) {
		DevMsg("STOPWATCH: %.2f\n", m_StopwatchFinish.GetRemaining());
		if (m_StopwatchFinish.Expired()) {
			m_StopwatchFinish.Stop();
			HL2MPRules()->GoToIntermission();
		}
	}
	
	SetNextThink(gpGlobals->curtime + 0.1f);
}

void CAR_StartlineEntity::StartTouch(CBaseEntity *pOther)
{
	if (stricmp(pOther->GetClassname(), "player") == 0) {

		int iPlayerSaveIndex = pOther->entindex() - 1;

		int iPenultimateCheckpoint = ((GetPlayerLaps(pOther->entindex()) * (m_iLastCheckpoint + 1)) + m_iLastCheckpoint);
		DevMsg("Startline player %d current check %d penultimate checkpoint %d\n", pOther->entindex(), m_iPlayerCheckpoint[iPlayerSaveIndex], iPenultimateCheckpoint);

		if (m_iPlayerCheckpoint[iPlayerSaveIndex] == iPenultimateCheckpoint) {

			// Startline is considered a checkpoint
			// Needed because of how the points are calculated
			SetPlayerCheckpoint(pOther->entindex(), (m_iLastCheckpoint+1));

			// Check if player finished race
			if (GetPlayerLaps(pOther->entindex()) == ar_laps.GetInt()) {

				// Process finished player
				FinishPlayer(pOther);

				// Start finish race timer when the winner crossed the line
				if (GetTotalFinished() == 1) {
					m_StopwatchFinish.Start(10);
				}

				//CBasePlayer *pBasePlayer = dynamic_cast<CBasePlayer*>(pOther);
				UTIL_ClientPrintAll(HUD_PRINTNOTIFY, "%s1 WINS\n", ((CBasePlayer*)pOther)->GetPlayerName());
			}
			// If player didn't win update lap information
			else {
				CAR_Player *pARPlayer = (CAR_Player*)(pOther);
				if (pARPlayer) {
					char msg[10];
					Q_snprintf(msg, sizeof(msg), "%d/%d", (GetPlayerLaps(pOther->entindex()) + 1), ar_laps.GetInt());
					pARPlayer->SendHudLapMsg(msg);
				}
			}

		}
	}
}

int CAR_StartlineEntity::GetPlayerLaps(int iPlayerIndex) {
	int iPlayerSaveIndex = iPlayerIndex - 1;
	float fLaps = m_iPlayerCheckpoint[iPlayerSaveIndex] / (m_iLastCheckpoint + 1);
	return floor(fLaps);
}

void CAR_StartlineEntity::SetPlayerCheckpoint(int iPlayerIndex, int iCheckpoint) {
	int iPlayerSaveIndex = iPlayerIndex - 1;

	int iCheckpointReached = GetPlayerLaps(iPlayerIndex) * (m_iLastCheckpoint + 1) + iCheckpoint;

	if ((m_iPlayerCheckpoint[iPlayerSaveIndex] + 1) == iCheckpointReached) {
		m_iPlayerCheckpoint[iPlayerSaveIndex] = iCheckpointReached;
		m_iPlayerCheckpointPosition[iPlayerSaveIndex][iCheckpointReached] = GetCheckpointFinished(iCheckpointReached) + 1;
		UpdatePlayerPositions();
	}
}

void CAR_StartlineEntity::UpdatePlayerPositions()
{
	// Structure array used to store player's total points
	playerPoints iPlayerPoints[MAX_PLAYERS];
	playerPosition iPlayerPosition[MAX_PLAYERS];
	
	for (int i = 0; i < MAX_PLAYERS; i++) {
		
		int pointsCheckpoint = 0, pointsCheckpointFinal = 0;

		// Give a point for each checkpoint reached
		pointsCheckpoint = m_iPlayerCheckpoint[i];
		
		// Then add additional points onto the their last checkpoint depending on their position
		if (m_iPlayerCheckpointPosition[i][m_iPlayerCheckpoint[i]]) {
			pointsCheckpointFinal = ((MAX_PLAYERS + 1) - m_iPlayerCheckpointPosition[i][m_iPlayerCheckpoint[i]]);
		}

		// Now add players points to our array structure
		iPlayerPoints[i].index = i+1;
		iPlayerPoints[i].points = pointsCheckpoint + pointsCheckpointFinal;

		if (iPlayerPoints[i].points > 0)
			DevMsg("player %d points checkpoints %d points checkpoint final %d\n", iPlayerPoints[i].index, pointsCheckpoint, pointsCheckpointFinal);
	}
	
	// Bubble sort
	playerPoints swap;
	for (int i = 0; i < (MAX_PLAYERS-1); i++) {
		for (int b = 0; b < (MAX_PLAYERS - 1); b++) {
			if (iPlayerPoints[i].points < iPlayerPoints[i + 1].points) {
				swap = iPlayerPoints[i];
				iPlayerPoints[i] = iPlayerPoints[i + 1];
				iPlayerPoints[i + 1] = swap;
			}
		}
	}

	// Calculate positions
	for (int i = 0; i < MAX_PLAYERS; i++) {
		iPlayerPosition[i].index = iPlayerPoints[i].index;
		iPlayerPosition[i].position = (iPlayerPoints[i].points > 0 ? (i + 1) : 0);

		DevMsg("i %d client %d points %d pos %d\n", i, iPlayerPosition[i].index, iPlayerPoints[i].points, iPlayerPosition[i].position);

		// Update airboat sprite position if player now has a position
		if (iPlayerPosition[i].position != 0) {
			CBasePlayer *pPlayer = UTIL_PlayerByIndex(iPlayerPosition[i].index);
			if (pPlayer && pPlayer->IsConnected()) {
				CAR_Player *pARPlayer = (CAR_Player*)(pPlayer);
				pARPlayer->CreateAirboatPositionSprite(iPlayerPosition[i].position);
			}
		}
	}

	// Send positions to HUD
	CBasePlayer *pPlayer = NULL;
	for (int i = 0; i < MAX_PLAYERS; i++) {
		pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer) {
			if (pPlayer->IsConnected()) {
				CAR_Player *pARPlayer = (CAR_Player*)(pPlayer);
				pARPlayer->SendHudLeaders(iPlayerPosition);
			}
		}
	}

}

int CAR_StartlineEntity::GetCheckpointFinished(int iCheckpoint)
{
	int iCount = 0;
	for (int i = 0; i<MAX_PLAYERS; i++) {
		if (m_iPlayerCheckpointPosition[i][iCheckpoint] != 0)
			iCount++;
	}
	return iCount;
}

void CAR_StartlineEntity::FinishPlayer(CBaseEntity *pPlayer)
{
	int iPlayerSaveIndex = pPlayer->entindex()-1;
	m_bPlayerFinished[iPlayerSaveIndex] = true;

	CAR_Player *pARPlayer = (CAR_Player*)(pPlayer);
	CBaseEntity *pVehicle = pARPlayer->GetVehicleEntity();
	if (pVehicle) {

		// Change camera back to original vehicle camera
		CBaseEntity *pViewEntity = pARPlayer->GetViewEntity();
		variant_t emptyInput;
		pViewEntity->AcceptInput("Disable", pPlayer, pPlayer, emptyInput, 0);

		// Exit vehicle
		pVehicle->AcceptInput("ExitVehicle", pPlayer, pPlayer, emptyInput, 0);
	}

	// Move player to spectator mode
	pARPlayer->ChangeTeam(TEAM_SPECTATOR);
	pARPlayer->SetObserverMode(OBS_MODE_ROAMING);
	pARPlayer->ForceObserverMode(OBS_MODE_ROAMING);
}

int CAR_StartlineEntity::GetTotalFinished()
{
	int iFinished = 0;
	for (int i = 0; i<MAX_PLAYERS; i++) {
		if (m_bPlayerFinished[i])
			iFinished++;
	}
	return iFinished;
}

void CAR_StartlineEntity::ResetPositions()
{
	for (int i = 0; i < MAX_PLAYERS; i++) {
		
		m_bPlayerFinished[i] = false;

		for (int checkpoint = 0; checkpoint < MAX_CHECKPOINTS; checkpoint++) {
			m_iPlayerCheckpointPosition[i][checkpoint] = 0;
		}
	}
}
