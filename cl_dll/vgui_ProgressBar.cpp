#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_ProgressBar.h"

CProgressBar::CProgressBar() : Panel(XRES(140), YRES(236), XRES(360), YRES(8))
{
    setPaintBackgroundEnabled(false);
}

void CProgressBar::paint()
{
	if( gHUD.m_flTime >= TimeEnd )
	{
		KillPrgBar();
		return;
	}

    drawSetColor(40, 40, 200, 100);
    drawFilledRect(0, 0, ( getWide() / TimeLong ) * ( gHUD.m_flTime - TimeStart ), getTall());

	drawSetColor(0, 0, 0, 70);
    drawOutlinedRect(0, 0, getWide(), getTall());
}

void CProgressBar::NewPrgBar( float flTimeLong )
{
	TimeStart = gHUD.m_flTime;
	TimeLong = flTimeLong;
	TimeEnd = TimeStart + TimeLong; 
	setVisible( true );
}

void CProgressBar::KillPrgBar( void )
{
	setVisible( false );
}