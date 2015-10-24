#include <windows.h>
#include <gl/gl.h>
#include "gl/glext.h"
#include "hud.h"
#include "tex_header.h"
#include <iostream>
using namespace std;

GLuint baseindex = 8192;

GLuint glLoadTgaFromPak( char* name, int index )
{
	// Длина файла и буффер под него:
	int len;

	byte *buff = gEngfuncs.COM_LoadFile( name, 5, &len ); // грузим файл
	
	int width, height; // размеры картинки
	TGA_HEADER hdr; // Заголовок файла

	if ( !buff ) // Если буффер=0 то файл не найден
	{
		gEngfuncs.Con_Printf( "%s - file not found!\n", name );
		return 0;
	}

	// копируем заголовок в отдельную переменную
	memcpy( &hdr, &buff[0], 18);
	
	width=hdr.width;
	height=hdr.height;
	
	// предполагаем что у нас 24 битная картинка
	int type = GL_RGB;
	int bpp;

	// проверяем это
	switch( hdr.bits )
	{
	case 24:
		bpp=3;
		break;
	case 32:
		type=GL_RGBA;
		bpp=4;
		break;
	default:
		gEngfuncs.Con_Printf( "glLoadTgaFromPak - not 32/24 bpp texture!\n" ); // материмся что картинка  - гамно
		return 0;
	}
	int imgsize = width * height * bpp;
	//byte temp;

	// flip image
	if (!(hdr.descriptor & 0x20))  // bottom up, need to swap scanlines
	{
		unsigned char *temp = (unsigned char *)malloc(hdr.width * bpp);

		for (int i = 0; i < hdr.height/2; i++)
		{
			memcpy(temp, &buff[18] + i*hdr.width*bpp, hdr.width*bpp);
			memcpy(&buff[18] + i*hdr.width*bpp,&buff[18] + (hdr.height-i-1)*hdr.width*bpp, hdr.width*bpp);
			memcpy(&buff[18] + (hdr.height-i-1)*hdr.width*bpp, temp, hdr.width*bpp);
		}
		free(temp);
	}

	// преобразуем BGR в RGB
	for( GLuint i = 18; i < GLuint( imgsize ); i += bpp )                                          
		swap( buff[i], buff[i+2] );

	GLuint ImgIndex;

	if( index )
		ImgIndex = index;
	else
		ImgIndex = baseindex;

	glBindTexture( GL_TEXTURE_2D, ImgIndex );	// делаем её текушей
	
	// Настраиваем мип маппинг
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR ); 
	glHint( GL_GENERATE_MIPMAP_HINT_SGIS, GL_NICEST );
	glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	
	// Загружаем в память
	glTexImage2D( GL_TEXTURE_2D, 0, type, width, height, 0, type, GL_UNSIGNED_BYTE, &buff[18] );

	// Закрываем файл
	gEngfuncs.COM_FreeFile( buff );

	if( !index )
		baseindex++;

	return ImgIndex;
	
}