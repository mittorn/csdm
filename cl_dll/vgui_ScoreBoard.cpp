#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"
#include "vgui_ScoreBoard.h"

#include <iostream>
#include <algorithm>
using namespace std;

hud_player_info_t	 g_PlayerInfoList[MAX_PLAYERS+1];	   // player info from the engine
extra_player_info_t  g_PlayerExtraInfo[MAX_PLAYERS+1];   // additional player info sent directly to the client dll
team_info_t			 g_TeamInfo[MAX_TEAMS+1];
int					 g_IsSpectator[MAX_PLAYERS+1];

struct s_ScrBrdPlayerInfo
{
	char Name[32];
	int Frags;
	int Deaths;
	int Ping;
	int Team;
	int spectator;
}ScrBrdPlayerInfo[32];

int CountPlayers = 0;

bool cmp( s_ScrBrdPlayerInfo A, s_ScrBrdPlayerInfo B )
{
	if( A.Frags > B.Frags )
		return true;
	else if( A.Frags == B.Frags && A.Deaths < B.Deaths )
		return true;
	return false;
}

CScoreBoard :: CScoreBoard() : Panel(XRES(100), YRES(30), XRES(440), YRES(390))
{
	setBgColor( 0, 0, 0, 100 );
}

//if(idx == gEngfuncs.GetLocalPlayer()->index)

void CScoreBoard::paintTeamplay()
{
	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Basic_Text"  );
	Font *pTitleFont = pSchemes->getFont( hTitleScheme );
	
	drawSetColor(255, 255, 0, 200);
	drawFilledRect(0, 0, getWide(), pTitleFont->getTall() + YRES(4));
	
    drawSetTextFont(pTitleFont);
    drawSetTextColor(250, 250, 250, 40);
    drawSetTextPos(XRES(10), YRES(2));
    drawPrintText("SCORES:", strlen( "SCORES:" ) );

	drawSetTextFont(pTitleFont);
    drawSetTextColor(250, 250, 250, 40);
    drawSetTextPos(XRES(10), YRES(20));
    drawPrintText("Teams", strlen( "Teams" ) );

	drawSetTextFont(pTitleFont);
    drawSetTextColor(250, 250, 250, 40);
    drawSetTextPos(XRES(280), YRES(20));
    drawPrintText("Frags", strlen( "Frags" ) );

	drawSetTextFont(pTitleFont);
    drawSetTextColor(250, 250, 250, 40);
    drawSetTextPos(XRES(330), YRES(20));
    drawPrintText("Deaths", strlen( "Deaths" ) );

	drawSetTextFont(pTitleFont);
    drawSetTextColor(250, 250, 250, 40);
    drawSetTextPos(XRES(380), YRES(20));
    drawPrintText("Ping", strlen( "Ping" ) );

	drawSetColor(0, 0, 0, 70);
    drawOutlinedRect(0, 0, getWide(), getTall());

	drawSetColor(255, 0, 0, 200);
	drawFilledRect( XRES( 10 ), YRES( 40 ), XRES( 430 ), pTitleFont->getTall() + YRES(44) );

	drawSetTextFont(pTitleFont);
    drawSetTextColor(250, 250, 250, 40);
    drawSetTextPos(XRES(12), YRES(42));
    drawPrintText("Terrorists", strlen( "Terrorists" ) );

	drawSetColor(0, 0, 255, 200);
	drawFilledRect( XRES( 10 ), YRES( 200 ), XRES( 430 ), pTitleFont->getTall() + YRES(204) );

	drawSetTextFont(pTitleFont);
    drawSetTextColor(250, 250, 250, 40);
    drawSetTextPos(XRES(12), YRES(202));
    drawPrintText("Counter-Terrorists", strlen( "Counter-Terrorists" ) );

	int TY = YRES(46) + pTitleFont->getTall();
	int CTY = YRES(206) + pTitleFont->getTall();

	
	for( int i = 0; i < CountPlayers; i++ )
	{
		if( ScrBrdPlayerInfo[i].spectator )
			continue;

		char Name[32];
		char Frags[32];
		char Deaths[32];
		char Ping[32];
		int Team;
		strcpy( Name, ScrBrdPlayerInfo[i].Name );
		sprintf( Frags, "%d", ScrBrdPlayerInfo[i].Frags );
		sprintf( Deaths, "%d", ScrBrdPlayerInfo[i].Deaths );
		sprintf( Ping, "%d", ScrBrdPlayerInfo[i].Ping );
		Team = ScrBrdPlayerInfo[i].Team;

		int X = XRES(12);
		int Y;
		if( Team == 1 )
			Y = TY;
		else
			Y = CTY;

		drawSetTextFont(pTitleFont);
		drawSetTextColor(250, 250, 250, 40);
		drawSetTextPos(X, Y);
		drawPrintText(Name, strlen( Name ) );

		drawSetTextFont(pTitleFont);
		drawSetTextColor(250, 250, 250, 40);
		drawSetTextPos(XRES(290), Y);
		drawPrintText(Frags, strlen( Frags ) );

		drawSetTextFont(pTitleFont);
		drawSetTextColor(250, 250, 250, 40);
		drawSetTextPos(XRES(340), Y);
		drawPrintText(Deaths, strlen( Deaths ) );

		drawSetTextFont(pTitleFont);
		drawSetTextColor(250, 250, 250, 40);
		drawSetTextPos(XRES(390), Y);
		drawPrintText(Ping, strlen( Ping ) );

		if( Team == 1 )
			TY += YRES(2) + pTitleFont->getTall();
		else
			CTY += YRES(2) + pTitleFont->getTall();
	}
}


void CScoreBoard::paint()
{
	gViewPort->GetAllPlayersInfo();
	CountPlayers = 0;
	for ( int i = 1; i < MAX_PLAYERS; i++ )
	{
		if( !g_PlayerInfoList[i].name )
			continue;
		if( !strlen( g_PlayerInfoList[i].name ) )
			continue;
		strcpy( ScrBrdPlayerInfo[i - 1].Name, g_PlayerInfoList[i].name );
		ScrBrdPlayerInfo[i - 1].Frags = g_PlayerExtraInfo[i].frags;
		ScrBrdPlayerInfo[i - 1].Deaths = g_PlayerExtraInfo[i].deaths;
		ScrBrdPlayerInfo[i - 1].Ping = g_PlayerInfoList[i].ping;
		ScrBrdPlayerInfo[i - 1].Team = g_PlayerExtraInfo[i].teamnumber;
		ScrBrdPlayerInfo[i - 1].spectator = g_IsSpectator[i];
		CountPlayers++;
	}

	sort( ScrBrdPlayerInfo, ScrBrdPlayerInfo + CountPlayers, cmp );

	if( gHUD.m_GameMode != 1 )
	{
		paintTeamplay();
		return;
	}

	CSchemeManager *pSchemes = gViewPort->GetSchemeManager();
	SchemeHandle_t hTitleScheme = pSchemes->getSchemeHandle( "Basic_Text"  );
	Font *pTitleFont = pSchemes->getFont( hTitleScheme );
	
	drawSetColor(255, 255, 0, 200);
	drawFilledRect(0, 0, getWide(), pTitleFont->getTall() + YRES(4));
	
    drawSetTextFont(pTitleFont);
    drawSetTextColor(250, 250, 250, 40);
    drawSetTextPos(XRES(10), YRES(2));
    drawPrintText("SCORES:", strlen( "SCORES:" ) );

	drawSetTextFont(pTitleFont);
    drawSetTextColor(250, 250, 250, 40);
    drawSetTextPos(XRES(10), YRES(20));
    drawPrintText("Teams", strlen( "Teams" ) );

	drawSetTextFont(pTitleFont);
    drawSetTextColor(250, 250, 250, 40);
    drawSetTextPos(XRES(280), YRES(20));
    drawPrintText("Frags", strlen( "Frags" ) );

	drawSetTextFont(pTitleFont);
    drawSetTextColor(250, 250, 250, 40);
    drawSetTextPos(XRES(330), YRES(20));
    drawPrintText("Deaths", strlen( "Deaths" ) );

	drawSetTextFont(pTitleFont);
    drawSetTextColor(250, 250, 250, 40);
    drawSetTextPos(XRES(380), YRES(20));
    drawPrintText("Ping", strlen( "Ping" ) );

	drawSetColor(0, 0, 0, 70);
    drawOutlinedRect(0, 0, getWide(), getTall());

	int Y = YRES(36) + pTitleFont->getTall();

	
	for( int i = 0; i < CountPlayers; i++ )
	{
		if( ScrBrdPlayerInfo[i].spectator )
			continue;

		char Name[32];
		char Frags[32];
		char Deaths[32];
		char Ping[32];
		int Team;
		strcpy( Name, ScrBrdPlayerInfo[i].Name );
		sprintf( Frags, "%d", ScrBrdPlayerInfo[i].Frags );
		sprintf( Deaths, "%d", ScrBrdPlayerInfo[i].Deaths );
		sprintf( Ping, "%d", ScrBrdPlayerInfo[i].Ping );
		Team = ScrBrdPlayerInfo[i].Team;

		int X = XRES(12);

		drawSetTextFont(pTitleFont);
		drawSetTextColor(250, 250, 250, 40);
		drawSetTextPos(X, Y);
		drawPrintText(Name, strlen( Name ) );

		drawSetTextFont(pTitleFont);
		drawSetTextColor(250, 250, 250, 40);
		drawSetTextPos(XRES(290), Y);
		drawPrintText(Frags, strlen( Frags ) );

		drawSetTextFont(pTitleFont);
		drawSetTextColor(250, 250, 250, 40);
		drawSetTextPos(XRES(340), Y);
		drawPrintText(Deaths, strlen( Deaths ) );

		drawSetTextFont(pTitleFont);
		drawSetTextColor(250, 250, 250, 40);
		drawSetTextPos(XRES(390), Y);
		drawPrintText(Ping, strlen( Ping ) );

		Y += YRES(2) + pTitleFont->getTall();
	}
}