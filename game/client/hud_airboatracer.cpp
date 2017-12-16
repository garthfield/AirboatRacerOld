#include "hud.h"
#include "cbase.h"
#include "hud_airboatracer.h"
#include "iclientmode.h"
#include "hud_macros.h"
#include "vgui_controls/controls.h"
#include "vgui/ISurface.h"

#include "tier0/memdbgon.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudAirboatRacer);
DECLARE_HUD_MESSAGE(CHudAirboatRacer, Powerup);

static ConVar show_powerup("show_powerup", "1", 0, "toggles powerup");

CHudAirboatRacer::CHudAirboatRacer(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudAirboatRacer")
{
	Msg("HUD AIRBOAT RACER");
	Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetVisible(false);
	SetAlpha(255);

	m_nPowerup1 = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nPowerup1, "sprites/hud/jumpicon", true, true);

	m_nPowerup2 = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_nPowerup2, "sprites/hud/nitroicon", true, true);

	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudAirboatRacer::Init(void) {
	HOOK_HUD_MESSAGE(CHudAirboatRacer, Powerup);
}

void CHudAirboatRacer::MsgFunc_Powerup(bf_read &msg) {
	m_iPowerupType = msg.ReadByte();
	Msg("Received Powerup Type: %d\n", m_iPowerupType);
}

void CHudAirboatRacer::Paint()
{
	SetPaintBorderEnabled(false);

	if (m_iPowerupType) {
		if (m_iPowerupType == 1) {
			surface()->DrawSetTexture(m_nPowerup1);
		}
		else if (m_iPowerupType == 2) {
			surface()->DrawSetTexture(m_nPowerup2);
		}
		surface()->DrawTexturedRect(0, 0, 64, 64);
	}
}

void CHudAirboatRacer::togglePrint()
{
	if (!show_powerup.GetBool())
		this->SetVisible(false);
	else
		this->SetVisible(true);
}

void CHudAirboatRacer::OnThink()
{
	togglePrint();
	BaseClass::OnThink();
}