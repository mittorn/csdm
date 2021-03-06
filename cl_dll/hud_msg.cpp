/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
//  hud_msg.cpp
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h"
#include "pmtrace.h"

#define MAX_CLIENTS 32
extern void EV_HLDM_WaterSplash( float x, float y, float z, float ScaleSplash1, float ScaleSplash2 );
extern void EV_HLDM_SmokeGrenade( float x, float y, float z );
extern void EV_HLDM_NewExplode( float x, float y, float z, float ScaleExplode1 );

int CHud :: MsgFunc_WaterSplash( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
    float X, Y, Z, ScaleSplash1, ScaleSplash2;
    
    X = READ_COORD();
    Y = READ_COORD();
    Z = READ_COORD();
	ScaleSplash1 = READ_COORD();
	ScaleSplash2 = READ_COORD();
    
	EV_HLDM_WaterSplash( X, Y, Z, ScaleSplash1, ScaleSplash2 );
    return 1;
}

int CHud :: MsgFunc_Smoke( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
    float X, Y, Z;
    
    X = READ_COORD();
    Y = READ_COORD();
    Z = READ_COORD();
    
	EV_HLDM_SmokeGrenade( X, Y, Z );
    return 1;
}

int CHud :: MsgFunc_NewExplode( const char *pszName, int iSize, void *pbuf )
{
    BEGIN_READ( pbuf, iSize );
    float X, Y, Z, ScaleExplode1;
    
    X = READ_COORD();
    Y = READ_COORD();
    Z = READ_COORD();
    ScaleExplode1 = READ_COORD();
    
    EV_HLDM_NewExplode( X, Y, Z, ScaleExplode1 );
    return 1;
}

/// USER-DEFINED SERVER MESSAGE HANDLERS

int CHud :: MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf )
{
	ASSERT( iSize == 0 );

	// clear all hud data
	HUDLIST *pList = m_pHudList;

	while ( pList )
	{
		if ( pList->p )
			pList->p->Reset();
		pList = pList->pNext;
	}

	// reset sensitivity
	m_flMouseSensitivity = 0;

	// reset concussion effect
	m_iConcussionEffect = 0;

	return 1;
}

void CAM_ToFirstPerson(void);

void CHud :: MsgFunc_ViewMode( const char *pszName, int iSize, void *pbuf )
{
	CAM_ToFirstPerson();
}

void CHud :: MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf )
{
	// prepare all hud data
	HUDLIST *pList = m_pHudList;

	while (pList)
	{
		if ( pList->p )
			pList->p->InitHUDData();
		pList = pList->pNext;
	}
}


int CHud :: MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_GameMode = READ_BYTE();

	return 1;
}


int CHud :: MsgFunc_Damage(const char *pszName, int iSize, void *pbuf )
{
	int		armor, blood;
	Vector	from;
	int		i;
	float	count;
	
	BEGIN_READ( pbuf, iSize );
	armor = READ_BYTE();
	blood = READ_BYTE();

	for (i=0 ; i<3 ; i++)
		from[i] = READ_COORD();

	count = (blood * 0.5) + (armor * 0.5);

	if (count < 10)
		count = 10;

	// TODO: kick viewangles,  show damage visually

	return 1;
}

int CHud :: MsgFunc_Concuss( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_iConcussionEffect = READ_BYTE();
	if (m_iConcussionEffect)
		this->m_StatusIcons.EnableIcon("dmg_concuss",255,160,0);
	else
		this->m_StatusIcons.DisableIcon("dmg_concuss");
	return 1;
}