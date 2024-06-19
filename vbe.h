


//int VbeGetVbeInfo(VbeInfo_t far *p);
//int VbeGetModeInfo(int mode, ModeInfo_t far *p);
int VbeSetMode(int mode);
void VbeSetPalette(const char far *p, int start, int n);
void VbeSetWindow(int window, int position);
void VbeWrite(int x, int y, int bytes, const char far *buffer);

typedef struct
{
    char VbeSignature[4];    /* "VESA" */
    int  VbeVersion;         /* bcd */
    char far *OemStringPtr;  /* asciiz */
    long Capabilities;       /* of video,
                                bitmapped */
    int  far *VideoModePtr;  /* array of
                                mode
                                numbers */
    int  TotalMemory; /* video memory/64kb */
    
    /* vbe 2.0+ */

    int  OemSoftwareRev;         /* bcd */
    char far *OemVendorNamePtr;  /* asciiz */
    char far *OemProductNamePtr; /* asciiz */
    char far *OemProductRevPtr;  /* asciiz */
    char Reserved[222];     /* used by vbe */
    char OemData[256];      /* used by vbe */
    
} VbeInfo_t;

typedef struct {
    unsigned short ModeAttributes;
    unsigned char WinAAttributes;
    unsigned char WinBAttributes;
    unsigned short WinGranularity;
    unsigned short WinSize;
    unsigned short WinASegment;
    unsigned short WinBSegment;
    unsigned long WinFuncPtr;
    unsigned short BytesPerScanLine;
    unsigned short XResolution;
    unsigned short YResolution;
    unsigned char XCharSize;
    unsigned char YCharSize;
    unsigned char NumberOfPlanes;
    unsigned char BitsPerPixel;
    unsigned char NumberOfBanks;
    unsigned char MemoryModel;
    unsigned char BankSize;
    unsigned char NumberOfImagePages;
    unsigned char ReservedPage;
    unsigned char RedMaskSize;
    unsigned char RedMaskPos;
    unsigned char GreenMaskSize;
    unsigned char GreenMaskPos;
    unsigned char BlueMaskSize;
    unsigned char BlueMaskPos;
    unsigned char ReservedMaskSize;
    unsigned char ReservedMaskPos;
    unsigned char DirectColorModeInfo;
    unsigned long PhysBasePtr;
    unsigned long Reserved1;
    unsigned short Reserved2;
    unsigned char Reserved3[206];
} ModeInfo_t;

/*  Not shown: ModeInfo_t structure, VBE
    function prototypes.  These are available
    on the code disk and via ftp see p. 3
    for details -mb */
/* End of File */
