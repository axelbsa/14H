#include <string.h>
#include <dos.h>
#include "vbe.h"    /* exports and data types */

/*------------------ local data -------------------*/

static VbeInfo_t _VbeInfo;
static ModeInfo_t _ModeInfo;

static int _Width;   /* scan line width in bytes */
static long _Window; /* memory window size in bytes */
static int _Segment; /* mem window segment */

/*--------------- exported functions ---------------*/

int VbeGetVbeInfo(VbeInfo_t far *p)

// fetches the vbe info block; returns 0 if no vbe.
{
    //union REGS r={0x4f00,0,0,0,0,FP_OFF(p)};
    //struct SREGS s={FP_SEG(p)};

    union REGS r;
    struct SREGS s;

    r.x.ax = 0x4F00;  // Get VBE controller information
    r.x.di = FP_OFF(p);
    s.es = FP_SEG(p);

    if(!p) return 0;

    _fmemset(p,0,sizeof(VbeInfo_t));
    _fmemcpy(p->VbeSignature,"VBE2",4);
    int86x(0x10,&r,&r,&s);

    if(_fmemcmp(p->VbeSignature,"VESA",4)) return 0;
    return r.x.ax==0x4f;
}

int VbeGetModeInfo(int mode, ModeInfo_t far *p)

/* fetches the mode info block for the specified mode
   number; returns 0 if mode unsupported by vbe. */
{
    //union REGS r={0x4f01,0,mode,0,0,FP_OFF(p)};
    //struct SREGS s={FP_SEG(p)};
    union REGS r;
    struct SREGS s;

    r.x.ax = 0x4F01;  // Get mode information
    r.x.cx = mode;
    r.x.di = FP_OFF(p);
    s.es = FP_SEG(p);

    if(!p) return 0;

    _fmemset(p,0,sizeof(ModeInfo_t));
    int86x(0x10,&r,&r,&s);

    return r.x.ax==0x4f;
}

int VbeSetMode(int mode)

/* initializes the requested video mode; returns 0 if
   there's no vbe or the mode is unavailable. */
{
    //union REGS r={0x4f02,mode};
    union REGS r;
    r.x.ax = 0x4F02;  // Set VBE mode
    r.x.bx = mode;  // Bit 14: linear frame buffer

    if(!VbeGetVbeInfo(&_VbeInfo)) return 0;

    if(mode>=0x100)    /* svga mode */
    {
        if(!VbeGetModeInfo(mode,&_ModeInfo) ||
            !(_ModeInfo.ModeAttributes&1)) return 0;

        _Width=_ModeInfo.BytesPerScanLine;
        _Window=1024L*_ModeInfo.WinSize;    /* convert to bytes */
        _Segment=_ModeInfo.WinASegment;
    }
    int86(0x10,&r,&r);

    return r.x.ax==0x4f;
}

void VbeSetPalette(const char far *p, int start, int n)

/* loads the dac palette registers; uses bios on vbe
   versions before 2.0 */
{
    if(_VbeInfo.VbeVersion<0x200)    /* use bios */
    {
        //union REGS r={0x1012,start,n,FP_OFF(p)};
        //struct SREGS s={FP_SEG(p)};
        union REGS r;
        struct SREGS s;

        r.x.ax = 0x1012;
        r.x.bx = start;
        r.x.cx = n;
        r.x.dx = FP_OFF(p);
        s.es = FP_SEG(p);

        int86x(0x10,&r,&r,&s);
    }
    else    /* use vbe */
    {
        /*union REGS r={0x4f09,0,n,start,0,FP_OFF(p)};*/
        /*struct SREGS s={FP_SEG(p)};*/
        union REGS r;
        struct SREGS s;

        r.x.ax = 0x4f09;
        r.x.cx = n;
        r.x.dx = start;
        r.x.di = FP_OFF(p);
        s.es = FP_SEG(p);

        int86x(0x10,&r,&r,&s);
    }
}
void VbeSetWindow(int window,int position)

/* repositions the indicates memory window to the new
   position (in WinGranularity units). */
{
    /*union REGS r={0x4f05,window,0,position};*/
    union REGS r;

    r.x.ax = 0x4f05;
    r.x.bx = window;
    r.x.dx = position;

        int86(0x10,&r,&r);
}
void VbeWrite(int x, int y, int bytes, const char far *buffer)

    /* copies the contents of the buffer (<64k) to display,
    starting at pixel (x,y). */

{
    long absolute = x + (long)y * _Width;      /* absolute offset */
    long position = absolute / _Window;      /* of window */
    long offset= absolute % _Window;        /* of window */
    char far *vram = MK_FP(_Segment, 0);    /* to window */

    VbeSetWindow(0, (int)position);

    if( (offset + bytes) > _Window)    /* data overruns window */
    {
        int n = (int)(_Window-offset);  /* bytes left */

        _fmemcpy(vram + offset, buffer, n);  /* display 1st part */
        VbeSetWindow(0, (int)++position); /* move window */
        _fmemcpy(vram, buffer + n, bytes - n); /* display rest */
    }
    else
    {
        _fmemcpy(vram + offset, buffer, bytes);/* no overrun */
    }
}
/* End of File */
