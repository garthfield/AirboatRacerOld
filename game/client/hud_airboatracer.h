#include "hudelement.h"
#include <vgui_controls/Panel.h>

using namespace vgui;

class CHudAirboatRacer : public CHudElement, public Panel
{
	DECLARE_CLASS_SIMPLE(CHudAirboatRacer, Panel);

public:
	CHudAirboatRacer(const char *pElementName);
	void togglePrint();
	virtual void OnThink();
	void Init(void);
	virtual void MsgFunc_Powerup(bf_read &msg);

protected:
	virtual void Paint();
	int m_nPowerup1;
	int m_nPowerup2;
	int m_iPowerupType;
};