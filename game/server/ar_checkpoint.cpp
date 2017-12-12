#include "cbase.h"
#include "triggers.h"
#include "ar_startline.h"

class CAR_CheckpointEntity : public CTriggerMultiple
{
public:
	DECLARE_CLASS(CAR_CheckpointEntity, CTriggerMultiple);
	DECLARE_DATADESC();

	void Spawn();
	void StartTouch(CBaseEntity *pOther);

private:
	int m_iIndex; // Checkpoint number
};

LINK_ENTITY_TO_CLASS(r_checkpoint, CAR_CheckpointEntity);

BEGIN_DATADESC(CAR_CheckpointEntity)
DEFINE_KEYFIELD(m_iIndex, FIELD_INTEGER, "index"),
END_DATADESC()

void CAR_CheckpointEntity::Spawn()
{
	DevMsg("Spawned Checkpoint\n");
	BaseClass::Spawn();
}

void CAR_CheckpointEntity::StartTouch(CBaseEntity *pOther)
{
	if (stricmp(pOther->GetClassname(), "player") == 0) {
		DevMsg("StartTouch entity:%s\n", pOther->GetClassname());
		
		CBaseEntity *pStartlineEntity = NULL;
		CAR_StartlineEntity *pStartpoint = NULL;
		if ((pStartlineEntity = gEntList.FindEntityByClassname(NULL, "r_startline")) != NULL) {
			pStartpoint = dynamic_cast<CAR_StartlineEntity *>(pStartlineEntity);
			pStartpoint->SetPlayerCheckpoint(pOther->entindex()-1, m_iIndex);
		}
	}
}