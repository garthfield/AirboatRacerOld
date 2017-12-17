#include "hud.h"
#include "cbase.h"
#include "hud_airboatracer.h"
#include "c_prop_vehicle.h"
#include "iclientmode.h"
#include "hud_macros.h"
#include "vgui_controls/controls.h"
#include "vgui/ISurface.h"
#include "vgui/ILocalize.h"
#include <string>

#include "tier0/memdbgon.h"

using namespace vgui;

DECLARE_HUDELEMENT(CHudAirboatRacer);
DECLARE_HUD_MESSAGE(CHudAirboatRacer, Powerup);
DECLARE_HUD_MESSAGE(CHudAirboatRacer, Lap);

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

void CHudAirboatRacer::ApplySchemeSettings(IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);
}

void CHudAirboatRacer::Init(void) {
	HOOK_HUD_MESSAGE(CHudAirboatRacer, Powerup);
	HOOK_HUD_MESSAGE(CHudAirboatRacer, Lap);
}

void CHudAirboatRacer::MsgFunc_Powerup(bf_read &msg) {
	m_iPowerupType = msg.ReadByte();
	Msg("Received Powerup Type: %d\n", m_iPowerupType);
}

void CHudAirboatRacer::MsgFunc_Lap(bf_read &msg) {
	msg.ReadString(m_szLapInfo, sizeof(m_szLapInfo));
	Msg("Received Lap: %s\n", m_szLapInfo);
}

void CHudAirboatRacer::OnThink()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer || !pPlayer->IsInAVehicle())
	{
		return;
	}

	IClientVehicle *pVehicle = pPlayer->GetVehicle();
	C_PropVehicleDriveable *pDrivable = dynamic_cast<C_PropVehicleDriveable*>(pVehicle);
	m_iSpeed = pDrivable->GetSpeed();

	togglePrint();
	BaseClass::OnThink();
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
		surface()->DrawTexturedRect(icon1_xpos, icon1_ypos, icon1_xpos + 64, icon1_ypos + 64);
	}

	std::string sSpeed = std::to_string(m_iSpeed);
	const char *pszLabelPowerup = "POWERUP", *pszLabelLap = "LAP", *pszLabelSpeed = "SPEED", *pszValueSpeed = sSpeed.c_str();
	wchar_t sLabelPowerup[256], sLableLap[256], sLabelSpeed[256];
	wchar_t sValueLap[256], sValueSpeed[256];

	g_pVGuiLocalize->ConvertANSIToUnicode(pszLabelPowerup, sLabelPowerup, sizeof(sLabelPowerup));
	
	g_pVGuiLocalize->ConvertANSIToUnicode(pszLabelLap, sLableLap, sizeof(sLableLap));
	g_pVGuiLocalize->ConvertANSIToUnicode(m_szLapInfo, sValueLap, sizeof(sValueLap));

	g_pVGuiLocalize->ConvertANSIToUnicode(pszLabelSpeed, sLabelSpeed, sizeof(sLabelSpeed));
	g_pVGuiLocalize->ConvertANSIToUnicode(pszValueSpeed, sValueSpeed, sizeof(sValueSpeed));

	// Set text colour
	Color cColor = m_TextColor;
	surface()->DrawSetTextColor(cColor[0], cColor[1], cColor[2], cColor[3]);

	// Powerup Label
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextPos(text1_xpos, text1_ypos);
	surface()->DrawUnicodeString(sLabelPowerup);

	// Lap Label
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextPos(text2_xpos, text2_ypos);
	surface()->DrawUnicodeString(sLableLap);

	// Lap Number
	surface()->DrawSetTextFont(m_hNumberFont);
	surface()->DrawSetTextPos(digit2_xpos, digit2_ypos);
	surface()->DrawUnicodeString(sValueLap);

	// Speed Label
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextPos(text3_xpos, text3_ypos);
	surface()->DrawUnicodeString(sLabelSpeed);

	// Speed Number
	surface()->DrawSetTextFont(m_hNumberFont);
	surface()->DrawSetTextPos(digit3_xpos, digit3_ypos);
	surface()->DrawUnicodeString(sValueSpeed);
}

void CHudAirboatRacer::togglePrint()
{
	if (!show_powerup.GetBool())
		this->SetVisible(false);
	else
		this->SetVisible(true);
}
