#include "cbase.h"
#include "vehicle_base.h"

#define SF_CAMERA_PLAYER_POSITION		1
#define SF_CAMERA_PLAYER_TARGET			2
#define SF_CAMERA_PLAYER_TAKECONTROL	4
#define SF_CAMERA_PLAYER_INFINITE_WAIT	8
#define SF_CAMERA_PLAYER_SNAP_TO		16
#define SF_CAMERA_PLAYER_NOT_SOLID		32
#define SF_CAMERA_PLAYER_INTERRUPT		64

class CAR_VehicalCamera : public CBaseEntity
{
public:
	DECLARE_CLASS(CAR_VehicalCamera, CBaseEntity);

	void Spawn(void);
	bool KeyValue(const char *szKeyName, const char *szValue);
	void Enable(void);
	void Disable(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void FollowTarget(void);
	void Move(void);
	void CameraInit(void);
	void CameraDampenEyeAngles(void);

	// Always transmit to clients so they know where to move the view to
	virtual int UpdateTransmitState();

	DECLARE_DATADESC();

	// Input handlers
	void InputEnable(inputdata_t &inputdata);
	void InputDisable(inputdata_t &inputdata);

private:
	EHANDLE m_hPlayer;
	EHANDLE m_hTarget;

	// used for moving the camera along a path (rail rides)
	CBaseEntity *m_pPath;
	CBaseEntity *pVehicle;
	string_t m_sPath;
	float m_flWait;
	float m_flReturnTime;
	float m_flStopTime;
	float m_moveDistance;
	float m_targetSpeed;
	float m_initialSpeed;
	float m_acceleration;
	float m_deceleration;
	int	  m_state;
	Vector m_vecMoveDir;


	string_t m_iszTargetAttachment;
	int	  m_iAttachmentIndex;
	bool  m_bSnapToGoal;

	int   m_nPlayerButtons;
	int   m_nOldTakeDamage;

private:
	COutputEvent m_OnEndFollow;
};

LINK_ENTITY_TO_CLASS(ar_vehicle_camera, CAR_VehicalCamera);

BEGIN_DATADESC(CAR_VehicalCamera)

DEFINE_FIELD(m_hPlayer, FIELD_EHANDLE),
DEFINE_FIELD(m_hTarget, FIELD_EHANDLE),
DEFINE_FIELD(m_pPath, FIELD_CLASSPTR),
DEFINE_FIELD(m_sPath, FIELD_STRING),
DEFINE_FIELD(m_flWait, FIELD_FLOAT),
DEFINE_FIELD(m_flReturnTime, FIELD_TIME),
DEFINE_FIELD(m_flStopTime, FIELD_TIME),
DEFINE_FIELD(m_moveDistance, FIELD_FLOAT),
DEFINE_FIELD(m_targetSpeed, FIELD_FLOAT),
DEFINE_FIELD(m_initialSpeed, FIELD_FLOAT),
DEFINE_FIELD(m_acceleration, FIELD_FLOAT),
DEFINE_FIELD(m_deceleration, FIELD_FLOAT),
DEFINE_FIELD(m_state, FIELD_INTEGER),
DEFINE_FIELD(m_vecMoveDir, FIELD_VECTOR),
DEFINE_KEYFIELD(m_iszTargetAttachment, FIELD_STRING, "targetattachment"),
DEFINE_FIELD(m_iAttachmentIndex, FIELD_INTEGER),
DEFINE_FIELD(m_bSnapToGoal, FIELD_BOOLEAN),
DEFINE_FIELD(m_nPlayerButtons, FIELD_INTEGER),
DEFINE_FIELD(m_nOldTakeDamage, FIELD_INTEGER),

// Inputs
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),

// Function Pointers
DEFINE_FUNCTION(FollowTarget),
DEFINE_OUTPUT(m_OnEndFollow, "OnEndFollow"),

END_DATADESC()

void CAR_VehicalCamera::Spawn(void)
{
	DevMsg("CAR_VehicalCamera SPAWNED\n");
	BaseClass::Spawn();

	SetMoveType(MOVETYPE_NOCLIP);
	SetSolid(SOLID_NONE);							// Remove model & collisions
	SetRenderColorA(0);								// The engine won't draw this model if this is set to 0 and blending is on
	m_nRenderMode = kRenderTransTexture;

	m_state = USE_OFF;

	m_initialSpeed = m_flSpeed;

	if (m_acceleration == 0)
		m_acceleration = 500;

	if (m_deceleration == 0)
		m_deceleration = 500;

	DispatchUpdateTransmitState();
	CameraInit();
}

void CAR_VehicalCamera::CameraDampenEyeAngles(void) {
	if (pVehicle) {
		// Set y to 0 to lock left/right rotation
		QAngle cameraAngles(pVehicle->GetAbsAngles().x, pVehicle->GetAbsAngles().y + 90, 0);
		SetAbsAngles(cameraAngles);
		SetNextThink(gpGlobals->curtime);
	}
}

void CAR_VehicalCamera::CameraInit(void) {

	// Check we have a parent
	if (this->GetParent() != NULL) {
		const char *pszVehicleClass = this->GetParent()->GetClassname();

		// Check parent is a vehicle
		if (strcmp(pszVehicleClass, "prop_vehicle_airboat") == 0 || strcmp(pszVehicleClass, "prop_vehicle_jeep") == 0) {

			// Retrieve vehicle
			pVehicle = this->GetParent();
			CBaseServerVehicle *pServerVehicle = dynamic_cast<CBaseServerVehicle *>(pVehicle->GetServerVehicle());
			if (pServerVehicle) {
				DevMsg("CAR_VehicalCamera::Spawn retrieve vehicle\n");

				// Calculate origin & angles for the camera based on the vehicle's
				QAngle cameraAngles(0, pVehicle->GetAbsAngles().y + 90, 0);

				Vector vecForward;
				AngleVectors(cameraAngles, &vecForward);
				Vector cameraOrigin = pVehicle->GetAbsOrigin() - vecForward * 10;
				cameraOrigin.z += 62;

				// Set camera origin & angles
				SetAbsOrigin(cameraOrigin);
				SetAbsAngles(cameraAngles);

				// Check for driver and activate camrea
				if (pServerVehicle->GetPassenger()) {
					m_hPlayer = pServerVehicle->GetDriver();
					Enable();
				}

				SetThink(&CAR_VehicalCamera::CameraDampenEyeAngles);
				SetNextThink(gpGlobals->curtime);
			}
		}
	}
}

int CAR_VehicalCamera::UpdateTransmitState()
{
	// always tranmit if currently used by a monitor
	if (m_state == USE_ON)
	{
		return SetTransmitState(FL_EDICT_ALWAYS);
	}
	else
	{
		return SetTransmitState(FL_EDICT_DONTSEND);
	}
}

bool CAR_VehicalCamera::KeyValue(const char *szKeyName, const char *szValue)
{
	if (FStrEq(szKeyName, "wait"))
	{
		m_flWait = atof(szValue);
	}
	else if (FStrEq(szKeyName, "moveto"))
	{
		m_sPath = AllocPooledString(szValue);
	}
	else if (FStrEq(szKeyName, "acceleration"))
	{
		m_acceleration = atof(szValue);
	}
	else if (FStrEq(szKeyName, "deceleration"))
	{
		m_deceleration = atof(szValue);
	}
	else
		return BaseClass::KeyValue(szKeyName, szValue);

	return true;
}

void CAR_VehicalCamera::InputEnable(inputdata_t &inputdata)
{
	m_hPlayer = inputdata.pActivator;
	Enable();
}

void CAR_VehicalCamera::InputDisable(inputdata_t &inputdata)
{
	Disable();
}

void CAR_VehicalCamera::Enable(void)
{
	m_state = USE_ON;

	if (!m_hPlayer || !m_hPlayer->IsPlayer())
	{
		DevMsg("NO PLAYER TYRING TO RETRIEVE\n");
		m_hPlayer = UTIL_GetLocalPlayer();
	}

	if (!m_hPlayer)
	{
		DevMsg("NO PLAYER CANT RETRIEVE\n");
		DispatchUpdateTransmitState();
		return;
	}

	Assert(m_hPlayer->IsPlayer());
	CBasePlayer *pPlayer = NULL;

	if (m_hPlayer->IsPlayer())
	{
		pPlayer = ((CBasePlayer*)m_hPlayer.Get());
	}
	else
	{
		Warning("CTriggerCamera could not find a player!\n");
		return;
	}

	// if the player was already under control of a similar trigger, disable the previous trigger.
	{
		CBaseEntity *pPrevViewControl = pPlayer->GetViewEntity();
		if (pPrevViewControl && pPrevViewControl != pPlayer)
		{
			CAR_VehicalCamera *pOtherCamera = dynamic_cast<CAR_VehicalCamera *>(pPrevViewControl);
			if (pOtherCamera)
			{
				if (pOtherCamera == this)
				{
					// what the hell do you think you are doing?
					Warning("Viewcontrol %s was enabled twice in a row!\n", GetDebugName());
					return;
				}
				else
				{
					pOtherCamera->Disable();
				}
			}
		}
	}


	m_nPlayerButtons = pPlayer->m_nButtons;


	// Make the player invulnerable while under control of the camera.  This will prevent situations where the player dies while under camera control but cannot restart their game due to disabled player inputs.
	m_nOldTakeDamage = m_hPlayer->m_takedamage;
	m_hPlayer->m_takedamage = DAMAGE_NO;

	if (HasSpawnFlags(SF_CAMERA_PLAYER_NOT_SOLID))
	{
		m_hPlayer->AddSolidFlags(FSOLID_NOT_SOLID);
	}

	m_flReturnTime = gpGlobals->curtime + m_flWait;
	m_flSpeed = m_initialSpeed;
	m_targetSpeed = m_initialSpeed;

	// this pertains to view angles, not translation.
	if (HasSpawnFlags(SF_CAMERA_PLAYER_SNAP_TO))
	{
		m_bSnapToGoal = true;
	}

	if (HasSpawnFlags(SF_CAMERA_PLAYER_TARGET))
	{
		m_hTarget = m_hPlayer;
	}
	else
	{
		m_hTarget = GetNextTarget();
	}

	// If we don't have a target, ignore the attachment / etc
	if (m_hTarget)
	{
		m_iAttachmentIndex = 0;
		if (m_iszTargetAttachment != NULL_STRING)
		{
			if (!m_hTarget->GetBaseAnimating())
			{
				Warning("%s tried to target an attachment (%s) on target %s, which has no model.\n", GetClassname(), STRING(m_iszTargetAttachment), STRING(m_hTarget->GetEntityName()));
			}
			else
			{
				m_iAttachmentIndex = m_hTarget->GetBaseAnimating()->LookupAttachment(STRING(m_iszTargetAttachment));
				if (m_iAttachmentIndex <= 0)
				{
					Warning("%s could not find attachment %s on target %s.\n", GetClassname(), STRING(m_iszTargetAttachment), STRING(m_hTarget->GetEntityName()));
				}
			}
		}
	}

	if (HasSpawnFlags(SF_CAMERA_PLAYER_TAKECONTROL))
	{
		((CBasePlayer*)m_hPlayer.Get())->EnableControl(FALSE);
	}

	if (m_sPath != NULL_STRING)
	{
		m_pPath = gEntList.FindEntityByName(NULL, m_sPath, NULL, m_hPlayer);
	}
	else
	{
		m_pPath = NULL;
	}

	m_flStopTime = gpGlobals->curtime;
	if (m_pPath)
	{
		if (m_pPath->m_flSpeed != 0)
			m_targetSpeed = m_pPath->m_flSpeed;

		m_flStopTime += m_pPath->GetDelay();
	}


	// copy over player information. If we're interpolating from
	// the player position, do something more elaborate.
	if (HasSpawnFlags(SF_CAMERA_PLAYER_POSITION))
	{
		UTIL_SetOrigin(this, m_hPlayer->EyePosition());
		SetLocalAngles(QAngle(m_hPlayer->GetLocalAngles().x, m_hPlayer->GetLocalAngles().y, 0));
		SetAbsVelocity(m_hPlayer->GetAbsVelocity());
	}
	else
	{
		SetAbsVelocity(vec3_origin);
	}


	pPlayer->SetViewEntity(this);

	// Hide the player's viewmodel
	if (pPlayer->GetActiveWeapon())
	{
		pPlayer->GetActiveWeapon()->AddEffects(EF_NODRAW);
	}

	// Only track if we have a target
	if (m_hTarget)
	{
		// follow the player down
		SetThink(&CAR_VehicalCamera::FollowTarget);
		SetNextThink(gpGlobals->curtime);
	}

	m_moveDistance = 0;
	Move();

	DispatchUpdateTransmitState();
}

void CAR_VehicalCamera::Disable(void)
{
	if (m_hPlayer && m_hPlayer->IsAlive())
	{
		if (HasSpawnFlags(SF_CAMERA_PLAYER_NOT_SOLID))
		{
			m_hPlayer->RemoveSolidFlags(FSOLID_NOT_SOLID);
		}

		((CBasePlayer*)m_hPlayer.Get())->SetViewEntity(m_hPlayer);
		((CBasePlayer*)m_hPlayer.Get())->EnableControl(TRUE);

		// Restore the player's viewmodel
		if (((CBasePlayer*)m_hPlayer.Get())->GetActiveWeapon())
		{
			((CBasePlayer*)m_hPlayer.Get())->GetActiveWeapon()->RemoveEffects(EF_NODRAW);
		}
		//return the player to previous takedamage state
		m_hPlayer->m_takedamage = m_nOldTakeDamage;
	}

	m_state = USE_OFF;
	m_flReturnTime = gpGlobals->curtime;
	SetThink(NULL);

	m_OnEndFollow.FireOutput(this, this); // dvsents2: what is the best name for this output?
	SetLocalAngularVelocity(vec3_angle);

	DispatchUpdateTransmitState();
}

void CAR_VehicalCamera::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, m_state))
		return;

	// Toggle state
	if (m_state != USE_OFF)
	{
		Disable();
	}
	else
	{
		m_hPlayer = pActivator;
		Enable();
	}
}

void CAR_VehicalCamera::FollowTarget()
{
	if (m_hPlayer == NULL)
		return;

	if (m_hTarget == NULL)
	{
		Disable();
		return;
	}

	if (!HasSpawnFlags(SF_CAMERA_PLAYER_INFINITE_WAIT) && (!m_hTarget || m_flReturnTime < gpGlobals->curtime))
	{
		Disable();
		return;
	}

	QAngle vecGoal;
	if (m_iAttachmentIndex)
	{
		Vector vecOrigin;
		m_hTarget->GetBaseAnimating()->GetAttachment(m_iAttachmentIndex, vecOrigin);
		VectorAngles(vecOrigin - GetAbsOrigin(), vecGoal);
	}
	else
	{
		if (m_hTarget)
		{
			VectorAngles(m_hTarget->GetAbsOrigin() - GetAbsOrigin(), vecGoal);
		}
		else
		{
			// Use the viewcontroller's angles
			vecGoal = GetAbsAngles();
		}
	}

	// Should we just snap to the goal angles?
	if (m_bSnapToGoal)
	{
		SetAbsAngles(vecGoal);
		m_bSnapToGoal = false;
	}
	else
	{
		// UNDONE: Can't we just use UTIL_AngleDiff here?
		QAngle angles = GetLocalAngles();

		if (angles.y > 360)
			angles.y -= 360;

		if (angles.y < 0)
			angles.y += 360;

		SetLocalAngles(angles);

		float dx = vecGoal.x - GetLocalAngles().x;
		float dy = vecGoal.y - GetLocalAngles().y;

		if (dx < -180)
			dx += 360;
		if (dx > 180)
			dx = dx - 360;

		if (dy < -180)
			dy += 360;
		if (dy > 180)
			dy = dy - 360;

		QAngle vecAngVel;
		vecAngVel.Init(dx * 40 * gpGlobals->frametime, dy * 40 * gpGlobals->frametime, GetLocalAngularVelocity().z);
		SetLocalAngularVelocity(vecAngVel);
	}

	if (!HasSpawnFlags(SF_CAMERA_PLAYER_TAKECONTROL))
	{
		SetAbsVelocity(GetAbsVelocity() * 0.8);
		if (GetAbsVelocity().Length() < 10.0)
		{
			SetAbsVelocity(vec3_origin);
		}
	}

	SetNextThink(gpGlobals->curtime);

	Move();
}

void CAR_VehicalCamera::Move()
{
	if (HasSpawnFlags(SF_CAMERA_PLAYER_INTERRUPT))
	{
		if (m_hPlayer)
		{
			CBasePlayer *pPlayer = ToBasePlayer(m_hPlayer);

			if (pPlayer)
			{
				int buttonsChanged = m_nPlayerButtons ^ pPlayer->m_nButtons;

				if (buttonsChanged && pPlayer->m_nButtons)
				{
					Disable();
					return;
				}

				m_nPlayerButtons = pPlayer->m_nButtons;
			}
		}
	}

	// Not moving on a path, return
	if (!m_pPath)
		return;
	{
		// Subtract movement from the previous frame
		m_moveDistance -= m_flSpeed * gpGlobals->frametime;

		// Have we moved enough to reach the target?
		if (m_moveDistance <= 0)
		{
			variant_t emptyVariant;
			m_pPath->AcceptInput("InPass", this, this, emptyVariant, 0);
			// Time to go to the next target
			m_pPath = m_pPath->GetNextTarget();

			// Set up next corner
			if (!m_pPath)
			{
				SetAbsVelocity(vec3_origin);
			}
			else
			{
				if (m_pPath->m_flSpeed != 0)
					m_targetSpeed = m_pPath->m_flSpeed;

				m_vecMoveDir = m_pPath->GetLocalOrigin() - GetLocalOrigin();
				m_moveDistance = VectorNormalize(m_vecMoveDir);
				m_flStopTime = gpGlobals->curtime + m_pPath->GetDelay();
			}
		}

		if (m_flStopTime > gpGlobals->curtime)
			m_flSpeed = UTIL_Approach(0, m_flSpeed, m_deceleration * gpGlobals->frametime);
		else
			m_flSpeed = UTIL_Approach(m_targetSpeed, m_flSpeed, m_acceleration * gpGlobals->frametime);

		float fraction = 2 * gpGlobals->frametime;
		SetAbsVelocity(((m_vecMoveDir * m_flSpeed) * fraction) + (GetAbsVelocity() * (1 - fraction)));
	}

}
