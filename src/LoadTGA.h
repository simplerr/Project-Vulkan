#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
	TGA loader by Ingemar Ragnemalm
*/

typedef struct TextureData			// Create A Structure for .tga loading.
{
	unsigned char	*imageData;		// Image Data (Up To 32 Bits)
	int		bpp;					// Image Color Depth In Bits Per Pixel.
	int		width;					// Image Width
	int		height;					// Image Height
	int		w;						// Image Width "raw"
	int		h;						// Image Height "raw"
	int		texID;					// Texture ID Used To Select A Texture
	float	texWidth, texHeight;
} TextureData, *TextureDataPtr;					// Structure Name

//bool LoadTGATexture(char *filename, TextureData *texture);
//void LoadTGATextureSimple(char *filename, int *tex);
void LoadTGASetMipmapping(bool active);
bool LoadTGATextureData(char *filename, TextureData *texture);

// Constants for SaveTGA
#define	TGA_ERROR_FILE_OPEN				-5
#define TGA_ERROR_READING_FILE			-4
#define TGA_ERROR_INDEXED_COLOR			-3
#define TGA_ERROR_MEMORY				-2
#define TGA_ERROR_COMPRESSED_FILE		-1
#define TGA_OK							 0

// Save functions
int SaveDataToTGA(char			*filename,
	short int		width,
	short int		height,
	unsigned char	pixelDepth,
	unsigned char	*imageData);
void SaveTGA(TextureData *tex, char *filename);
void SaveFramebufferToTGA(char *filename, int x, int y, int w, int h);
