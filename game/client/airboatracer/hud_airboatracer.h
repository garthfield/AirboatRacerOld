#include "hudelement.h"
#include <vgui_controls/Panel.h>

using namespace vgui;

class CHudAirboatRacer : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudAirboatRacer, Panel);

public:
	CHudAirboatRacer(const char *pElementName);
	virtual void OnThink();
	void Init(void);
	virtual void ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void MsgFunc_Powerup(bf_read &msg);
	virtual void MsgFunc_Lap(bf_read &msg);

protected:
	virtual void Paint();
	int m_nPowerup1;
	int m_nPowerup2;
	int m_nPowerup3;
	int m_iPowerupType;
	int m_iSpeed;
	char m_szLapInfo[256];

	CPanelAnimationVar(vgui::HFont, m_hNumberFont, "NumberFont", "HudNumbers");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");
	CPanelAnimationVar(Color, m_TextColor, "TextColor", "FgColor");

	CPanelAnimationVarAliasType(float, text1_xpos, "text1_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text1_ypos, "text1_ypos", "10", "proportional_float");
	CPanelAnimationVarAliasType(float, icon1_xpos, "icon1_xpos", "64", "proportional_float");
	CPanelAnimationVarAliasType(float, icon1_ypos, "icon1_ypos", "10", "proportional_float");

	CPanelAnimationVarAliasType(float, text2_xpos, "text2_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_ypos, "text2_ypos", "20", "proportional_float");
	CPanelAnimationVarAliasType(float, digit2_xpos, "digit2_xpos", "64", "proportional_float");
	CPanelAnimationVarAliasType(float, digit2_ypos, "digit2_ypos", "20", "proportional_float");

	CPanelAnimationVarAliasType(float, text3_xpos, "text3_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text3_ypos, "text3_ypos", "30", "proportional_float");
	CPanelAnimationVarAliasType(float, digit3_xpos, "digit3_xpos", "64", "proportional_float");
	CPanelAnimationVarAliasType(float, digit3_ypos, "digit3_ypos", "30", "proportional_float");

};