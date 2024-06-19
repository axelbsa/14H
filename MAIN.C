#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <conio.h>
#include <dos.h>

#include "vbe.h"    /* vesa vbe interface */

/* --------------- Local Data -------------*/

/* (Abridged) header record of a PCX file */
typedef struct
{
    char Manufacturer; /* pcx file sig: 10 */
    char Version;      /* pc paintbrsh ver */
    char Encoding;     /* 1: rle */
    char BitsPerPixel; /* color resolution */
    int  Xmin,Ymin;    /* image origin */
    int  Xmax,Ymax;    /* image extent */
    char NotUsed1[53]; /* not used by this
                          program */
    char Planes;       /* number of color
                          planes */
    int  BytesPerLine; /* (per plane) */
    char NotUsed2[58]; /* by this program */

} pcx_t;

/* Error codes; these also index ErrorText
   array below */
enum
{
    PCX_OK,PCX_EOPEN,PCX_EVIDEO,
    PCX_EREAD,PCX_EFORMAT
};

static const char *ErrorText[]=
{
/* PCX_OK */     "Ok",
/* PCX_EOPEN */  "File not found",
/* PCX_EVIDEO */ "Unsupported video mode",
/* PCX_EREAD */  "File read error",
/* PCX_EFORMAT */
    "Invalid or unsupported format"
};

/* PCX image line buffer */
static char Line[1024*4];
/* DAC palette data */
static char Palette256[768];
/* PCX file header record */
static pcx_t Header;

/*------ local pcx file functions ---------*/

static int ReadHeader(FILE *f)

/* reads the header of the open file into the
   global Header structure; returns 0 on a
   file read error, or if the file is not a
   valid pcx file, or if the pcx file does
   not contain a 640x480x256-compatible
   image. */
{
    pcx_t *h=&Header;

    fread(h,sizeof(pcx_t),1,f);
    return !(ferror(f) ||
        h->Manufacturer!=10 ||
        h->Encoding!=1 ||
        h->Planes!=1 ||
        h->BitsPerPixel!=8 ||
        h->Ymax-h->Ymin>479 ||
        h->Xmax-h->Xmin>639);
}
static int ReadPalette(FILE *f)

/* reads the 256-color palette of the open
   file into the global buffer Palette256,
   and converts it to 6-bit DAC palette
   register data; returns 0 on a file read
   error or an invalid palette block. */
{
    int i;
    fseek(f,-769L,SEEK_END); /* to palette */
    if(fgetc(f)!=12) /* check sig byte */
        return 0;

    /* read the 256 triplets */

    fread(Palette256,768,1,f);
    if(ferror(f)) return 0;
    fseek(f,128L,SEEK_SET); /* bk to image */
    for(i=0;i<768;i++)
        Palette256[i]>>=2; /* to 6-bit rgb */
    return 1;
}
static void ReadLine(FILE *f)



/* reads and decompresses the next scanline
   from the current pcx file into the global
   Line buffer. */
{
    int    c,i=0;

    while(i<Header.BytesPerLine)
    {
        c=fgetc(f);
        if((c&0xc0)==0xc0) /* is rle hdr */
        {
            c&=~0xc0;   /* =repeat count */
            /* copy repeated pixel data */
            memset(Line+i,fgetc(f),c);
            i+=c;
        }
        else Line[i++]=c; /* is pixel data */
    }
     return;
}

/*----------- general functions -----------*/
int PcxShowFile(const char *file)

/* reads and displays a pcx file; returns 0
   on success, PCX_ error code on failure. */
{
    int    e=PCX_OK;
    FILE *f=fopen(file,"rb");
    if(!f) e=PCX_EOPEN;
    else if(!ReadHeader(f)) e=PCX_EFORMAT;
    else if(Header.Version==5 &&
            !ReadPalette(f)) e=PCX_EFORMAT;
    else if(!VbeSetMode(0x101)) e=PCX_EVIDEO;
    else
    {
        int y,ymax=min(Header.Ymax,479);
        int    n=min(Header.Xmax,640)-
               Header.Xmin+1;
        if(Header.Version==5)
            VbeSetPalette(Palette256,0,256);
        for(y=Header.Ymin;y<ymax;y++)
        {
            ReadLine(f);
            VbeWrite(Header.Xmin,y,n,Line);
        }
    }
    if(f) fclose(f);
    return e;
}

void PcxDeinit()
{
/* clears the image and resets the video to
   standard 80-column text mode */
    VbeSetMode(3);
}

void main(void)
{

/* displays the pcx file specified by the
   first argument to the program
   (usage: PCX <filename>) */
    int x,y;
    //int e=PcxShowFile(argv[1]);
    VbeSetMode(0x118);

    for (y = 0; y < 768; y++)
    {
        for (x = 0; x < 1024*4; x=x+4)
        {
            Line[x]   = 0xff;
            Line[x+1] = 0x0;
            Line[x+2] = 0x0;
            Line[x+3] = 0xff;
        }

        VbeWrite(x,y,1024*4, &Line);
    }
    getch();
    PcxDeinit();

    /*if(!e)*/
    /*{*/
        /*PcxDeinit();*/
    /*}*/
    /*else puts(ErrorText[e]);*/
}
/* End of File */
