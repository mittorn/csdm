//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Triangle rendering, if any

#include "hud.h"
#include "cl_util.h"

// Triangle rendering apis are in gEngfuncs.pTriAPI

#include "const.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"

#define DLLEXPORT __declspec( dllexport )

extern "C"
{
	void DLLEXPORT HUD_DrawNormalTriangles( void );
	void DLLEXPORT HUD_DrawTransparentTriangles( void );
};

//#define TEST_IT
#if defined( TEST_IT )

/*
=================
Draw_Triangles

Example routine.  Draws a sprite offset from the player origin.
=================
*/
void Draw_Triangles( void )
{
	cl_entity_t *player;
	vec3_t org;

	// Load it up with some bogus data
	player = gEngfuncs.GetLocalPlayer();
	if ( !player )
		return;

	org = player->origin;

	org.x += 50;
	org.y += 50;

	if (gHUD.m_hsprCursor == 0)
	{
		char sz[256];
		sprintf( sz, "sprites/cursor.spr" );
		gHUD.m_hsprCursor = SPR_Load( sz );
	}

	if ( !gEngfuncs.pTriAPI->SpriteTexture( (struct model_s *)gEngfuncs.GetSpritePointer( gHUD.m_hsprCursor ), 0 ))
	{
		return;
	}
	
	// Create a triangle, sigh
	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
	gEngfuncs.pTriAPI->CullFace( TRI_NONE );
	gEngfuncs.pTriAPI->Begin( TRI_QUADS );
	// Overload p->color with index into tracer palette, p->packedColor with brightness
	gEngfuncs.pTriAPI->Color4f( 1.0, 1.0, 1.0, 1.0 );
	// UNDONE: This gouraud shading causes tracers to disappear on some cards (permedia2)
	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 0, 0 );
	gEngfuncs.pTriAPI->Vertex3f( org.x, org.y, org.z );

	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 0, 1 );
	gEngfuncs.pTriAPI->Vertex3f( org.x, org.y + 50, org.z );

	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 1, 1 );
	gEngfuncs.pTriAPI->Vertex3f( org.x + 50, org.y + 50, org.z );

	gEngfuncs.pTriAPI->Brightness( 1 );
	gEngfuncs.pTriAPI->TexCoord2f( 1, 0 );
	gEngfuncs.pTriAPI->Vertex3f( org.x + 50, org.y, org.z );

	gEngfuncs.pTriAPI->End();
	gEngfuncs.pTriAPI->RenderMode( kRenderNormal );
}

#endif

/*
=================
HUD_DrawNormalTriangles

Non-transparent triangles-- add them here
=================
*/
void DLLEXPORT HUD_DrawNormalTriangles( void )
{
	gHUD.m_Spectator.DrawOverview();
	
#if defined( TEST_IT )
//	Draw_Triangles();
#endif
}

/*
=================
HUD_DrawTransparentTriangles

Render any triangles with transparent rendermode needs here
=================
*/
void DLLEXPORT HUD_DrawTransparentTriangles( void )
{
#if defined( TEST_IT )
//	Draw_Triangles();
#endif
}


#include <windows.h>
#include <gl/gl.h>
#include "gl/glext.h"
#include "tex_header.h"
int ScopeTex = -1;
void DrawScope()
{
	if( ScopeTex < 0 )
		ScopeTex = glLoadTgaFromPak( "/gfx/textures/scope.tga" );

	Vector Point[4];
	Point[0] = Vector( ( ScreenWidth - ScreenHeight ) / 2, ScreenHeight, 0 );
	Point[1] = Vector( ( ScreenWidth - ScreenHeight ) / 2, 0, 0 );
	Point[2] = Vector( ScreenWidth - ( ScreenWidth - ScreenHeight ) / 2, 0, 0 );
	Point[3] = Vector( ScreenWidth - ( ScreenWidth - ScreenHeight ) / 2, ScreenHeight, 0 );
	glColor4f(0.0f,0.0f,0.0f,1.0f);
	glBindTexture(GL_TEXTURE_2D, ScopeTex);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBegin (GL_QUADS);

          glTexCoord2f(0, 0);
          glVertex3fv(Point[0]);

          glTexCoord2f(1, 0);
          glVertex3fv (Point[1]);

          glTexCoord2f(1, 1);
          glVertex3fv (Point[2]);

          glTexCoord2f(0, 1);
          glVertex3fv (Point[3]);

	glEnd ();


	glColor4f(0.0f,0.0f,0.0f,1.0f);
	glBegin (GL_QUADS);
			
          glVertex3f ( 0, ScreenHeight, 0 );
          glVertex3f ( 0, 0, 0 );
          glVertex3f ( ( ScreenWidth - ScreenHeight ) / 2, 0, 0 );
          glVertex3f ( ( ScreenWidth - ScreenHeight ) / 2, ScreenHeight, 0 );

	glEnd ();

	glColor4f(0.0f,0.0f,0.0f,1.0f);
	glBegin (GL_QUADS);

          glVertex3f ( ScreenWidth - ( ScreenWidth - ScreenHeight ) / 2, ScreenHeight, 0 );
          glVertex3f ( ScreenWidth - ( ScreenWidth - ScreenHeight ) / 2, 0, 0 );
          glVertex3f ( ScreenWidth, 0, 0 );
          glVertex3f ( ScreenWidth, ScreenHeight, 0 );

	glEnd ();

	Vector Center = Vector( ScreenWidth / 2, ScreenHeight / 2, 0 );

	glColor4f(0.0f,0.0f,0.0f,1.0f);
	glBegin (GL_QUADS);
			
          glVertex3f ( 0, Center.y + 1, 0 );
          glVertex3f ( 0, Center.y, 0 );
          glVertex3f ( ScreenWidth, Center.y, 0 );
          glVertex3f ( ScreenWidth, Center.y + 1, 0 );

	glEnd ();

	glColor4f(0.0f,0.0f,0.0f,1.0f);
	glBegin (GL_QUADS);
			
          glVertex3f ( Center.x, ScreenHeight, 0 );
          glVertex3f ( Center.x, 0, 0 );
          glVertex3f ( Center.x + 1, 0, 0 );
          glVertex3f ( Center.x + 1, ScreenHeight, 0 );

	glEnd ();
}