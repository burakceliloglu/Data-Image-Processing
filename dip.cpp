#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma pack(1)
typedef unsigned char BYTE; // 1 byte of memory
typedef unsigned short WORD; // 2 bytes of memory
typedef unsigned int DWORD; // 4 bytes of memory
typedef int LONG; // 4 bytes of memory

typedef struct _BMPFH // takes 14 bytes of memory
{
    BYTE bftype1; // 1 byte and must be 'B'
    BYTE bftype2; // 1 byte and must be 'M'
    DWORD bfsize; // 4 bytes gives us the all size of bmp file (including headers, palette (if it has), data)
    WORD bfreserved1; // 2 bytes of memory could be set as 0
    WORD bfreserved2; // 2 bytes of memory could be set as 0
    DWORD bfOffbits; // 4 bytes of memory gives the position of data starts in the bmp file
} BMPFH;

typedef struct _BMPIH // 40 bytes for windows bitmap file
{
    DWORD bisize; // 4 bytes and it gives the size of info header
    LONG biw; // 4 bytes and it is the width of image
    LONG bih; // 4 bytes and it is the height of image
    WORD biplane; // 2 bytes which is not important for us
    WORD bibitcount; // 2 bytes it is about the type of bitmap file if it is 1 2 color, 4 (16 colors) ..
    DWORD biComp; // 4 bytes not important
    DWORD bisizeimage; // 4 bytes and it gives the size of data in the image
    LONG bix; // 4 bytes not important
    LONG biy; // 4 bytes not important
    DWORD biclused; // 4 bytes not important
    DWORD biclimp; // 4 bytes not important for us
} BMPIH;

typedef struct _PALET // in palette it describes colors (what is the color number)
{
    BYTE rgbblue; // blue component
    BYTE rgbgreen; // green component
    BYTE rgbred; // red component
    BYTE rgbreserved; // reserved byte the user can use this for their aims
} PALET;

typedef struct _IMAGE
{
    BMPFH bmpfh;
    BMPIH bmpih;
    PALET *palet;
    BYTE *data;
} IMAGE;

IMAGE *ImageRead(IMAGE *image, const char *filename)
{
    BMPFH bmpfh;
    BMPIH bmpih;
    FILE *fp;
    DWORD r, rowsize, size;
    fp = fopen(filename, "rb"); // tries to open the filename
    if (fp == NULL)
    {
        printf("File is not found..");
        exit(1);
    }
    fread(&bmpfh, sizeof(BMPFH), 1, fp); // reads bitmap file header
    fread(&bmpih, sizeof(BMPIH), 1, fp); // reads bitmap info header

    rowsize = ((bmpih.biw * bmpih.bibitcount + 31) / 32) * 4;
    size = rowsize * bmpih.bih;
    image->data = (BYTE *)malloc(size);
    if (image->data == NULL)
    {
        printf("Memory allocation error..");
        exit(1);
    }
    fread(image->data, size, 1, fp); // reads image data
    fclose(fp);

    image->bmpfh = bmpfh;
    image->bmpih = bmpih;
    return image;
}

void SaveGrayScaleImage(IMAGE *image, const char *filename)
{
    FILE *fp;
    DWORD i, j, size, rowsize, newSize;
    BYTE *grayData;
    PALET palette[256];
    BMPFH bmpfh = image->bmpfh;
    BMPIH bmpih = image->bmpih;

    bmpih.bibitcount = 8; // change bit count to 8
    bmpih.bisizeimage = ((bmpih.biw * bmpih.bibitcount + 31) / 32) * 4 * bmpih.bih;
    bmpfh.bfsize = sizeof(BMPFH) + sizeof(BMPIH) + 256 * sizeof(PALET) + bmpih.bisizeimage;
    bmpfh.bfOffbits = sizeof(BMPFH) + sizeof(BMPIH) + 256 * sizeof(PALET);

    for (i = 0; i < 256; i++) // create grayscale palette
    {
        palette[i].rgbblue = i;
        palette[i].rgbgreen = i;
        palette[i].rgbred = i;
        palette[i].rgbreserved = 0;
    }

    rowsize = ((image->bmpih.biw * 24 + 31) / 32) * 4;
    newSize = ((bmpih.biw * 8 + 31) / 32) * 4 * bmpih.bih;
    grayData = (BYTE *)malloc(newSize);

    if (grayData == NULL)
    {
        printf("Memory allocation error.");
        exit(1);
    }

    for (i = 0; i < bmpih.bih; i++)
    {
        for (j = 0; j < bmpih.biw; j++)
        {
            BYTE r = image->data[i * rowsize + j * 3 + 2];
            BYTE g = image->data[i * rowsize + j * 3 + 1];
            BYTE b = image->data[i * rowsize + j * 3];
            grayData[i * ((bmpih.biw + 3) & ~3) + j] = (BYTE)(0.299 * r + 0.587 * g + 0.114 * b);
        }
    }

    fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        printf("File could not be created..");
        exit(1);
    }

    fwrite(&bmpfh, sizeof(BMPFH), 1, fp);
    fwrite(&bmpih, sizeof(BMPIH), 1, fp);
    fwrite(palette, sizeof(PALET), 256, fp);
    fwrite(grayData, newSize, 1, fp);

    fclose(fp);
    free(grayData);
}

int main()
{
    IMAGE image;
    ImageRead(&image, "parrots.bmp");
    SaveGrayScaleImage(&image, "parrots_gray.bmp");
    free(image.data);
    return 0;
}
