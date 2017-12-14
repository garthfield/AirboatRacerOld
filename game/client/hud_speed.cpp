#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "view.h"

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "c_prop_vehicle.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Speed panel
//-----------------------------------------------------------------------------
class CHudSpeed : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE(CHudSpeed, CHudNumericDisplay);

public:
	CHudSpeed(const char *pElementName);
	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual void OnThink();

private:
	int		m_iSpeed;
};

DECLARE_HUDELEMENT(CHudSpeed);

CHudSpeed::CHudSpeed(const char *pElementName) : CHudElement(pElementName), CHudNumericDisplay(NULL, "HudSpeed")
{
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudSpeed::Init()
{
	Reset();
}

void CHudSpeed::Reset()
{
	m_iSpeed = 0;
	SetLabelText(L"SPEED");
	SetDisplayValue(m_iSpeed);
}

void CHudSpeed::VidInit()
{
	Reset();
}

void CHudSpeed::OnThink()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer || !pPlayer->IsInAVehicle())
	{
		return;
	}

	IClientVehicle *pVehicle = pPlayer->GetVehicle();
	C_PropVehicleDriveable *pDrivable = dynamic_cast<C_PropVehicleDriveable*>(pVehicle);
	SetDisplayValue(pDrivable->GetSpeed());
}
