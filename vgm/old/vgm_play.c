// VGMPlay.c: C Source File of the Main Executable
//

// Line Size:	96 Chars
// Tab Size:	4 Spaces

/*3456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456
0000000001111111111222222222233333333334444444444555555555566666666667777777777888888888899999*/

// Mixer Muting ON:
//		Mixer's FM Volume is set to 0 or Mute	-> absolutely muted
//		(sometimes it can take some time to get the Mixer Control under Windows)
// Mixer Muting OFF:
//		FM Volume is set to 0 through commands	-> very very low volume level ~0.4%
//		(faster way)
//#define MIXER_MUTING

// These defines enable additional features.
//	ADDITIONAL_FORMATS enables CMF and DRO support.
//	CONSOLE_MODE switches between VGMPlay and in_vgm mode.
//	in_vgm mode can also be used for custom players.
//
//#define ADDITIONAL_FORMATS
//#define CONSOLE_MODE
//#define VGM_LITTLE_ENDIAN	// enable optimizations for Little Endian systems
//#define VGM_BIG_ENDIAN	// enable optimizations for Big Endian systems

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "stdbool.h"
#include <math.h>	// for pow()


#include <limits.h>		// for PATH_MAX
#include <pthread.h>	// for pthread functions


#include <time.h>		// for clock_gettime()

#include <unistd.h>		// for usleep()

#define MAX_PATH	PATH_MAX
#define	Sleep(msec)	usleep(msec * 1000)



#include <zlib.h>

#include "chips/mamedef.h"

// integer types for fast integer calculation
// the bit number is unused (it's an orientation)
#define FUINT8	unsigned int
#define FUINT16	unsigned int

#include "VGMPlay.h"
#include "VGMPlay_Intf.h"

#include "chips/ChipIncl.h"

unsigned char OpenPortTalk(void);
void ClosePortTalk(void);

#include "ChipMapper.h"

typedef void (*strm_func)(UINT8 ChipID, stream_sample_t **outputs, int samples);

typedef struct chip_audio_attributes CAUD_ATTR;
struct chip_audio_attributes
{
	UINT32 SmpRate;
	UINT16 Volume;
	UINT8 ChipType;
	UINT8 ChipID;		// 0 - 1st chip, 1 - 2nd chip, etc.
	// Resampler Type:
	//	00 - Old
	//	01 - Upsampling
	//	02 - Copy
	//	03 - Downsampling
	UINT8 Resampler;
	strm_func StreamUpdate;
	UINT32 SmpP;		// Current Sample (Playback Rate)
	UINT32 SmpLast;		// Sample Number Last
	UINT32 SmpNext;		// Sample Number Next
	WAVE_32BS LSmpl;	// Last Sample
	WAVE_32BS NSmpl;	// Next Sample
	CAUD_ATTR* Paired;
};

typedef struct chip_audio_struct
{
	CAUD_ATTR SN76496;
	CAUD_ATTR YM2413;
	CAUD_ATTR YM2612;
	CAUD_ATTR YM2151;
	CAUD_ATTR SegaPCM;
	CAUD_ATTR RF5C68;
	CAUD_ATTR YM2203;
	CAUD_ATTR YM2608;
	CAUD_ATTR YM2610;
	CAUD_ATTR YM3812;
	CAUD_ATTR YM3526;
	CAUD_ATTR Y8950;
	CAUD_ATTR YMF262;
	CAUD_ATTR YMF278B;
	CAUD_ATTR YMF271;
	CAUD_ATTR YMZ280B;
	CAUD_ATTR RF5C164;
	CAUD_ATTR PWM;
	CAUD_ATTR AY8910;
	CAUD_ATTR GameBoy;
	CAUD_ATTR NES;
	CAUD_ATTR MultiPCM;
	CAUD_ATTR UPD7759;
	CAUD_ATTR OKIM6258;
	CAUD_ATTR OKIM6295;
	CAUD_ATTR K051649;
	CAUD_ATTR K054539;
	CAUD_ATTR HuC6280;
	CAUD_ATTR C140;
	CAUD_ATTR K053260;
	CAUD_ATTR Pokey;
	CAUD_ATTR QSound;
	CAUD_ATTR SCSP;
	CAUD_ATTR WSwan;
	CAUD_ATTR VSU;
	CAUD_ATTR SAA1099;
	CAUD_ATTR ES5503;
	CAUD_ATTR ES5506;
	CAUD_ATTR X1_010;
	CAUD_ATTR C352;
	CAUD_ATTR GA20;
//	CAUD_ATTR OKIM6376;
} CHIP_AUDIO;

typedef struct chip_aud_list CA_LIST;
struct chip_aud_list
{
	CAUD_ATTR* CAud;
	CHIP_OPTS* COpts;
	CA_LIST* next;
};

typedef struct daccontrol_data
{
	int Enable;
	UINT8 Bank;
} DACCTRL_DATA;

typedef struct pcmbank_table
{
	UINT8 ComprType;
	UINT8 CmpSubType;
	UINT8 BitDec;
	UINT8 BitCmp;
	UINT16 EntryCount;
	void* Entries;
} PCMBANK_TBL;


// Function Prototypes (prototypes in comments are defined in VGMPlay_Intf.h)
//void VGMPlay_Init(void);
//void VGMPlay_Init2(void);
//void VGMPlay_Deinit(void);
//char* FindFile(const char* FileName)

INLINE UINT16 ReadLE16(const UINT8* Data);
INLINE UINT16 ReadBE16(const UINT8* Data);
INLINE UINT32 ReadLE24(const UINT8* Data);
INLINE UINT32 ReadLE32(const UINT8* Data);
INLINE int gzgetLE16(gzFile hFile, UINT16* RetValue);
INLINE int gzgetLE32(gzFile hFile, UINT32* RetValue);
static UINT32 gcd(UINT32 x, UINT32 y);
//void PlayVGM(void);
//void StopVGM(void);
//void RestartVGM(void);
//void PauseVGM(bool Pause);
//void SeekVGM(bool Relative, INT32 PlayBkSamples);
//void RefreshMuting(void);
//void RefreshPanning(void);
//void RefreshPlaybackOptions(void);

//UINT32 GetGZFileLength(const char* FileName);
//UINT32 GetGZFileLengthW(const wchar_t* FileName);
static UINT32 GetGZFileLength_Internal(FILE* hFile);
//bool OpenVGMFile(const char* FileName);
static int OpenVGMFile_Internal(gzFile hFile, UINT32 FileSize);
static void ReadVGMHeader(gzFile hFile, VGM_HEADER* RetVGMHead);
static UINT8 ReadGD3Tag(gzFile hFile, UINT32 GD3Offset, GD3_TAG* RetGD3Tag);
static void ReadChipExtraData32(UINT32 StartOffset, VGMX_CHP_EXTRA32* ChpExtra);
static void ReadChipExtraData16(UINT32 StartOffset, VGMX_CHP_EXTRA16* ChpExtra);
//void CloseVGMFile(void);
//void FreeGD3Tag(GD3_TAG* TagData);
static wchar_t* MakeEmptyWStr(void);
static wchar_t* ReadWStrFromFile(gzFile hFile, UINT32* FilePos, UINT32 EOFPos);
//UINT32 GetVGMFileInfo(const char* FileName, VGM_HEADER* RetVGMHead, GD3_TAG* RetGD3Tag);
static UINT32 GetVGMFileInfo_Internal(gzFile hFile, UINT32 FileSize,
									  VGM_HEADER* RetVGMHead, GD3_TAG* RetGD3Tag);
INLINE UINT32 MulDivRound(UINT64 Number, UINT64 Numerator, UINT64 Denominator);
//UINT32 CalcSampleMSec(UINT64 Value, UINT8 Mode);
//UINT32 CalcSampleMSecExt(UINT64 Value, UINT8 Mode, VGM_HEADER* FileHead);
//const char* GetChipName(UINT8 ChipID);
//const char* GetAccurateChipName(UINT8 ChipID, UINT8 SubType);
//UINT32 GetChipClock(VGM_HEADER* FileHead, UINT8 ChipID, UINT8* RetSubType);
static UINT16 GetChipVolume(VGM_HEADER* FileHead, UINT8 ChipID, UINT8 ChipNum, UINT8 ChipCnt);

static void RestartPlaying(void);
static void Chips_GeneralActions(UINT8 Mode);

INLINE INT32 SampleVGM2Pbk_I(INT32 SampleVal);	// inline functions
INLINE INT32 SamplePbk2VGM_I(INT32 SampleVal);
//INT32 SampleVGM2Playback(INT32 SampleVal);		// non-inline functions
//INT32 SamplePlayback2VGM(INT32 SampleVal);
static UINT8 StartThread(void);
static UINT8 StopThread(void);
#if defined(WIN32) && defined(MIXER_MUTING)
static int GetMixerControl(void);
#endif
static int SetMuteControl(int mute);

static void InterpretFile(UINT32 SampleCount);
static void AddPCMData(UINT8 Type, UINT32 DataSize, const UINT8* Data);
//INLINE FUINT16 ReadBits(UINT8* Data, UINT32* Pos, FUINT8* BitPos, FUINT8 BitsToRead);
static int DecompressDataBlk(VGM_PCM_DATA* Bank, UINT32 DataSize, const UINT8* Data);
static UINT8 GetDACFromPCMBank(void);
static UINT8* GetPointerFromPCMBank(UINT8 Type, UINT32 DataPos);
static void ReadPCMTable(UINT32 DataSize, const UINT8* Data);
static void InterpretVGM(UINT32 SampleCount);
#ifdef ADDITIONAL_FORMATS
extern void InterpretOther(UINT32 SampleCount);
#endif

static void GeneralChipLists(void);
static void SetupResampler(CAUD_ATTR* CAA);
static void ChangeChipSampleRate(void* DataPtr, UINT32 NewSmplRate);

INLINE INT16 Limit2Short(INT32 Value);
static void null_update(UINT8 ChipID, stream_sample_t **outputs, int samples);
static void dual_opl2_stereo(UINT8 ChipID, stream_sample_t **outputs, int samples);
static void ResampleChipStream(CA_LIST* CLst, WAVE_32BS* RetSample, UINT32 Length);
static INT32 RecalcFadeVolume(void);
//UINT32 FillBuffer(WAVE_16BS* Buffer, UINT32 BufferSize)

#ifdef WIN32
DWORD WINAPI PlayingThread(void* Arg);
#else
UINT64 TimeSpec2Int64(const struct timespec* ts);
void* PlayingThread(void* Arg);
#endif


// Options Variables
UINT32 SampleRate;	// Note: also used by some sound cores to determinate the chip sample rate

UINT32 VGMMaxLoop;
UINT32 VGMPbRate;	// in Hz, ignored if this value or VGM's lngRate Header value is 0
#ifdef ADDITIONAL_FORMATS
extern UINT32 CMFMaxLoop;
#endif
UINT32 FadeTime;
UINT32 PauseTime;	// current Pause Time

float VolumeLevel;
int SurroundSound;
UINT8 HardStopOldVGMs;
int FadeRAWLog;
int PauseEmulate;

int DoubleSSGVol;

UINT8 ResampleMode;	// 00 - HQ both, 01 - LQ downsampling, 02 - LQ both
UINT8 CHIP_SAMPLING_MODE;
INT32 CHIP_SAMPLE_RATE;

UINT16 FMPort;
int FMForce;
int FMBreakFade;
float FMVol;

CHIPS_OPTION ChipOpts[0x02];

UINT8 OPL_MODE;
UINT8 OPL_CHIPS;

stream_sample_t* DUMMYBUF[0x02] = {NULL, NULL};

char* AppPaths[8];

int AutoStopSkip;

UINT8 FileMode;
VGM_HEADER VGMHead;
VGM_HDR_EXTRA VGMHeadX;
VGM_EXTRA VGMH_Extra;
UINT32 VGMDataLen;
UINT8* VGMData;
GD3_TAG VGMTag;

#define PCM_BANK_COUNT	0x40
VGM_PCM_BANK PCMBank[PCM_BANK_COUNT];
PCMBANK_TBL PCMTbl;
UINT8 DacCtrlUsed;
UINT8 DacCtrlUsg[0xFF];
DACCTRL_DATA DacCtrl[0xFF];


pthread_t hPlayThread;
#endif
int PlayThreadOpen;
volatile int PauseThread;
static volatile int CloseThread;
int ThreadPauseEnable;
volatile int ThreadPauseConfrm;
int ThreadNoWait;	// don't reset the timer

CHIP_AUDIO ChipAudio[0x02];
CAUD_ATTR CA_Paired[0x02][0x03];
float MasterVol;

CA_LIST ChipListBuffer[0x200];
CA_LIST* ChipListAll;	// all chips needed for playback (in general)
CA_LIST* ChipListPause;	// all chips needed for EmulateWhilePaused
//CA_LIST* ChipListOpt;	// ChipListAll minus muted chips
CA_LIST* CurChipList;	// pointer to Pause or All [Opt]

#define SMPL_BUFSIZE	0x100
static INT32* StreamBufs[0x02];

#ifdef MIXER_MUTING


int hmixer;
UINT16 mixer_vol;

#else
float VolumeBak;
#endif

UINT32 VGMPos;
INT32 VGMSmplPos;
INT32 VGMSmplPlayed;
INT32 VGMSampleRate;
static UINT32 VGMPbRateMul;
static UINT32 VGMPbRateDiv;
static UINT32 VGMSmplRateMul;
static UINT32 VGMSmplRateDiv;
static UINT32 PauseSmpls;
bool VGMEnd;
bool EndPlay;
bool PausePlay;
bool FadePlay;
int ForceVGMExec;
UINT8 PlayingMode;
int UseFM;
UINT32 PlayingTime;
UINT32 FadeStart;
UINT32 VGMMaxLoopM;
UINT32 VGMCurLoop;
float VolumeLevelM;
float FinalVol;
int ResetPBTimer;


UINT32 FadeTimeN = FadeTime;
UINT32 PauseTimeJ = PauseTime;
UINT32 PauseTimeL = 0;
UINT32 Show95Cmds = 0x00;
UINT32 LogToWave = 0x00;
UINT32 ForceAudioBuf = 0x00;
UINT32 PreferJapTag = 0;

static int Interpreting;


UINT8 IsVGMInit;
UINT16 Last95Drum;	// for optvgm debugging
UINT16 Last95Max;	// for optvgm debugging
UINT32 Last95Freq;	// for optvgm debugging



static void ReadOptions(char *fn)
{
	const UINT8 CHN_COUNT[CHIP_COUNT] =
	{	0x04, 0x09, 0x06, 0x08, 0x10, 0x08, 0x03, 0x00,
		0x00, 0x09, 0x09, 0x09, 0x12, 0x00, 0x0C, 0x08,
		0x08, 0x00, 0x03, 0x04, 0x05, 0x1C, 0x00, 0x00,
		0x04, 0x05, 0x08, 0x08, 0x18, 0x04, 0x04, 0x10,
		0x20, 0x04, 0x06, 0x06, 0x20, 0x20, 0x10, 0x20,
		0x04
	};
	const UINT8 CHN_MASK_CNT[CHIP_COUNT] =
	{	0x04, 0x0E, 0x07, 0x08, 0x10, 0x08, 0x03, 0x06,
		0x06, 0x0E, 0x0E, 0x0E, 0x17, 0x18, 0x0C, 0x08,
		0x08, 0x00, 0x03, 0x04, 0x05, 0x1C, 0x00, 0x00,
		0x04, 0x05, 0x08, 0x08, 0x18, 0x04, 0x04, 0x10,
		0x20, 0x04, 0x06, 0x06, 0x20, 0x20, 0x10, 0x20,
		0x04
	};
	const char* FNList[3];
	char* FileName;
	FILE* hFile;
	char TempStr[0x40];
	UINT32 StrLen;
	UINT32 TempLng;
	char* LStr;
	char* RStr;
	UINT8 IniSection;
	UINT8 CurChip;
	CHIP_OPTS* TempCOpt;
	CHIP_OPTS* TempCOpt2;
	UINT8 CurChn;
	char* TempPnt;
	bool TempFlag;
	
	struct cfg_entry *o, *tmp;
	
	int tsection;
	
	o = read_conf(fn);
	
	if (!o)
		return;
	
	tmp = o;
	
	// most defaults are set by VGMPlay_Init()
	FadeTimeN = FadeTime;
	PauseTimeJ = PauseTime;
	PauseTimeL = 0;
	Show95Cmds = 0x00;
	LogToWave = 0x00;
	ForceAudioBuf = 0x00;
	PreferJapTag = false;
	
	
	
	while (tmp)
	{
		
		if (!strcmp("VGMGeneral", tmp->section))
			tsection = 0x00;
			
		else
		{
			tsection = 0xFF;
			for (CurChip = 0x00; CurChip < CHIP_COUNT; CurChip ++)
			{
				if (!strcmp(GetChipName(CurChip), tmp->section))
				{
					tsection = 0x80 | CurChip;
					break;
				}
			}
			
			if (tsection == 0xFF)
			{
				tmp = tmp->next;
				continue;
			}
				
		}
		switch (tsection)
		{
		case 0x00: /* general */
			if (!strcmp("SampleRate",tmp->name)  && tmp->type==E_INT)
				SampleRate = tmp->dat[0].i;
				
			if (!strcmp("PlaybackRate",tmp->name)  && tmp->type==E_INT)
				VGMPbRate = tmp->dat[0].i;
				
			if (!strcmp("DoubleSSGVol",tmp->name)  && (tmp->type==E_INT || tmp->type==E_BOOL))
				DoubleSSGVol = tmp->dat[0].i;
				
			/*if (!strcmp("PreferJapTag",tmp->name)  && (tmp->type==E_INT || tmp->type==E_BOOL) )
				PreferJapTag = tmp->dat[0].i;*/
				
			if (!strcmp("FadeTime",tmp->name)  && tmp->type==E_INT)
				FadeTimeN = tmp->dat[0].i;
				
			if (!strcmp("FadeTimePL",tmp->name)  && tmp->type==E_INT)
				FadeTimePL = tmp->dat[0].i;
				
			if (!strcmp("JinglePause",tmp->name)  && tmp->type==E_INT)
				PauseTimeJ = tmp->dat[0].i;
				
			if (!strcmp("HardStopOld",tmp->name)  && (tmp->type==E_INT || tmp->type==E_BOOL))
				HardStopOldVGMs = tmp->dat[0].i; /* check original */
				
			if (!strcmp("FadeRAWLogs",tmp->name)  && (tmp->type==E_INT || tmp->type==E_BOOL))
				FadeRAWLog = tmp->dat[0].i;
				
			if (!strcmp("Volume",tmp->name)  && tmp->type==E_FLOAT)
				VolumeLevel = tmp->dat[0].f; /* do floats parse now? */
				
			if (!strcmp("LogSound",tmp->name)  && tmp->type==E_INT)
				LogToWave = tmp->dat[0].i;
				
			if (!strcmp("MaxLoops",tmp->name)  && tmp->type==E_INT)
				VGMMaxLoop = tmp->dat[0].i;
				
			if (!strcmp("MaxLoopsCMF",tmp->name)  && tmp->type==E_INT)
				CMFMaxLoop = tmp->dat[0].i;
				
			if (!strcmp("ResamplingMode",tmp->name)  && tmp->type==E_INT)
				ResampleMode = (UINT8) tmp->dat[0].i;
				
			if (!strcmp("ChipSmplMode",tmp->name)  && tmp->type==E_INT)
				CHIP_SAMPLING_MODE = (UINT8) tmp->dat[0].i;
				
			if (!strcmp("ChipSmplRate",tmp->name)  && tmp->type==E_INT)
				CHIP_SAMPLE_RATE = tmp->dat[0].i;
				
			if (!strcmp("AudioBuffers",tmp->name)  && tmp->type==E_INT)
			{
				ForceAudioBuf = tmp->dat[0].i;
				
				if (ForceAudioBuf < 0x04)
					ForceAudioBuf= 0x00;
			}
			
			if (!strcmp("SurroundSound",tmp->name)  && (tmp->type==E_INT || tmp->type==E_BOOL))
				SurroundSound = tmp->dat[0].i;
			
			if (!strcmp("EmulatePause",tmp->name)  && (tmp->type==E_INT || tmp->type==E_BOOL))
				PauseEmulate = tmp->dat[0].i;
			
			if (!strcmp("ShowStreamCmds",tmp->name)  && tmp->type==E_INT)
				Show95Cmds = tmp->dat[0].i;
			
			if (!strcmp("FMPort",tmp->name)  && tmp->type==E_INT)
				FMPort = (UINT16)tmp->dat[0].i;
			
			if (!strcmp("FMForce",tmp->name)  && (tmp->type==E_INT || tmp->type==E_BOOL))
				FMForce = tmp->dat[0].i;
			
			if (!strcmp("FMVolume",tmp->name)  && (tmp->type==E_FLOAT))
				FMVol = tmp->dat[0].f;
			
			if (!strcmp("FMSoftStop",tmp->name)  && (tmp->type==E_INT || tmp->type==E_BOOL))
				FMBreakFade = tmp->dat[0].i;
			
			break;
		
				
				
				
			case 0x80:	// SN76496
			case 0x81:	// YM2413
			case 0x82:	// YM2612
			case 0x83:	// YM2151
			case 0x84:	// SegaPCM
			case 0x85:	// RF5C68
			case 0x86:	// YM2203
			case 0x87:	// YM2608
			case 0x88:	// YM2610
			case 0x89:	// YM3812
			case 0x8A:	// YM3526
			case 0x8B:	// Y8950
			case 0x8C:	// YMF262
			case 0x8D:	// YMF278B
			case 0x8E:	// YMF271
			case 0x8F:	// YMZ280B
			case 0x90:	// RF5C164
			case 0x91:	// PWM
			case 0x92:	// AY8910
			case 0x93:	// GameBoy
			case 0x94:	// NES
			case 0x95:	// MultiPCM
			case 0x96:	// UPD7759
			case 0x97:	// OKIM6258
			case 0x98:	// OKIM6295
			case 0x99:	// K051649
			case 0x9A:	// K054539
			case 0x9B:	// HuC6280
			case 0x9C:	// C140
			case 0x9D:	// K053260
			case 0x9E:	// Pokey
			case 0x9F:	// QSound
			case 0xA0:	// SCSP
			case 0xA1:	// WonderSwan
			case 0xA2:	// VSU
			case 0xA3:	// SAA1099
			case 0xA4:	// ES5503
			case 0xA5:	// ES5506
			case 0xA6:	// X1_010
			case 0xA7:	// C352
			case 0xA8:	// GA20
				CurChip = IniSection & 0x7F;
				TempCOpt = (CHIP_OPTS*)&ChipOpts[0x00] + CurChip;
				
				if (! stricmp_u(LStr, "Disabled"))
				{
					TempCOpt->Disabled = GetBoolFromStr(RStr);
				}
				else if (! stricmp_u(LStr, "EmulatorType"))
				{
					TempCOpt->EmuCore = (UINT8)strtol(RStr, NULL, 0);
				}
				else if (! stricmp_u(LStr, "MuteMask"))
				{
					if (! CHN_COUNT[CurChip])
						break;	// must use MuteMaskFM and MuteMask???
					TempCOpt->ChnMute1 = strtoul(RStr, NULL, 0);
					if (CHN_MASK_CNT[CurChip] < 0x20)
						TempCOpt->ChnMute1 &= (1 << CHN_MASK_CNT[CurChip]) - 1;
				}
				else if (! strnicmp_u(LStr, "MuteCh", 0x06))
				{
					if (! CHN_COUNT[CurChip])
						break;	// must use MuteFM and Mute???
					CurChn = (UINT8)strtol(LStr + 0x06, &TempPnt, 0);
					if (TempPnt == NULL || *TempPnt)
						break;
					if (CurChn >= CHN_COUNT[CurChip])
						break;
					TempFlag = GetBoolFromStr(RStr);
					TempCOpt->ChnMute1 &= ~(0x01 << CurChn);
					TempCOpt->ChnMute1 |= TempFlag << CurChn;
				}
				else
				{
					switch(CurChip)
					{
					//case 0x00:	// SN76496
					case 0x02:	// YM2612
						if (! stricmp_u(LStr, "MuteDAC"))
						{
							CurChn = 0x06;
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->ChnMute1 &= ~(0x01 << CurChn);
							TempCOpt->ChnMute1 |= TempFlag << CurChn;
						}
						else if (! stricmp_u(LStr, "DACHighpass"))
						{
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->SpecialFlags &= ~(0x01 << 0);
							TempCOpt->SpecialFlags |= TempFlag << 0;
						}
						else if (! stricmp_u(LStr, "SSG-EG"))
						{
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->SpecialFlags &= ~(0x01 << 1);
							TempCOpt->SpecialFlags |= TempFlag << 1;
						}
						else if (! stricmp_u(LStr, "PseudoStereo"))
						{
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->SpecialFlags &= ~(0x01 << 2);
							TempCOpt->SpecialFlags |= TempFlag << 2;
						}
						break;
					//case 0x03:	// YM2151
					//case 0x04:	// SegaPCM
					//case 0x05:	// RF5C68
					case 0x06:	// YM2203
						if (! stricmp_u(LStr, "DisableAY"))
						{
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->SpecialFlags &= ~(0x01 << 0);
							TempCOpt->SpecialFlags |= TempFlag << 0;
						}
						break;
					case 0x07:	// YM2608
					case 0x08:	// YM2610
						if (! stricmp_u(LStr, "DisableAY"))
						{
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->SpecialFlags &= ~(0x01 << 0);
							TempCOpt->SpecialFlags |= TempFlag << 0;
						}
						else if (! stricmp_u(LStr, "MuteMask_FM"))
						{
							TempCOpt->ChnMute1 = strtoul(RStr, NULL, 0);
							TempCOpt->ChnMute1 &= (1 << CHN_MASK_CNT[CurChip]) - 1;
						}
						else if (! stricmp_u(LStr, "MuteMask_PCM"))
						{
							TempCOpt->ChnMute2 = strtoul(RStr, NULL, 0);
							TempCOpt->ChnMute2 &= (1 << (CHN_MASK_CNT[CurChip] + 1)) - 1;
						}
						else if (! strnicmp_u(LStr, "MuteFMCh", 0x08))
						{
							CurChn = (UINT8)strtol(LStr + 0x08, &TempPnt, 0);
							if (TempPnt == NULL || *TempPnt)
								break;
							if (CurChn >= CHN_COUNT[CurChip])
								break;
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->ChnMute1 &= ~(0x01 << CurChn);
							TempCOpt->ChnMute1 |= TempFlag << CurChn;
						}
						else if (! strnicmp_u(LStr, "MutePCMCh", 0x08))
						{
							CurChn = (UINT8)strtol(LStr + 0x08, &TempPnt, 0);
							if (TempPnt == NULL || *TempPnt)
								break;
							if (CurChn >= CHN_COUNT[CurChip])
								break;
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->ChnMute2 &= ~(0x01 << CurChn);
							TempCOpt->ChnMute2 |= TempFlag << CurChn;
						}
						else if (! stricmp_u(LStr, "MuteDT"))
						{
							CurChn = 0x06;
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->ChnMute2 &= ~(0x01 << CurChn);
							TempCOpt->ChnMute2 |= TempFlag << CurChn;
						}
						break;
					case 0x01:	// YM2413
					case 0x09:	// YM3812
					case 0x0A:	// YM3526
					case 0x0B:	// Y8950
					case 0x0C:	// YMF262
						CurChn = 0xFF;
						if (! stricmp_u(LStr, "MuteBD"))
							CurChn = 0x00;
						else if (! stricmp_u(LStr, "MuteSD"))
							CurChn = 0x01;
						else if (! stricmp_u(LStr, "MuteTOM"))
							CurChn = 0x02;
						else if (! stricmp_u(LStr, "MuteTC"))
							CurChn = 0x03;
						else if (! stricmp_u(LStr, "MuteHH"))
							CurChn = 0x04;
						else if (CurChip == 0x0B && ! stricmp_u(LStr, "MuteDT"))
							CurChn = 0x05;
						if (CurChn != 0xFF)
						{
							if (CurChip < 0x0C)
								CurChn += 9;
							else
								CurChn += 18;
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->ChnMute1 &= ~(0x01 << CurChn);
							TempCOpt->ChnMute1 |= TempFlag << CurChn;
						}
						break;
					case 0x0D:	// YMF278B
						if (! stricmp_u(LStr, "MuteMask_FM"))
						{
							TempCOpt->ChnMute1 = strtoul(RStr, NULL, 0);
							TempCOpt->ChnMute1 &= (1 << CHN_MASK_CNT[CurChip - 0x01]) - 1;
						}
						else if (! stricmp_u(LStr, "MuteMask_WT"))
						{
							TempCOpt->ChnMute2 = strtoul(RStr, NULL, 0);
							TempCOpt->ChnMute2 &= (1 << CHN_MASK_CNT[CurChip]) - 1;
						}
						else if (! strnicmp_u(LStr, "MuteFMCh", 0x08))
						{
							CurChn = (UINT8)strtol(LStr + 0x08, &TempPnt, 0);
							if (TempPnt == NULL || *TempPnt)
								break;
							if (CurChn >= CHN_COUNT[CurChip - 0x01])
								break;
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->ChnMute1 &= ~(0x01 << CurChn);
							TempCOpt->ChnMute1 |= TempFlag << CurChn;
						}
						else if (! strnicmp_u(LStr, "MuteFM", 0x06))
						{
							CurChn = 0xFF;
							if (! stricmp_u(LStr + 6, "BD"))
								CurChn = 0x00;
							else if (! stricmp_u(LStr + 6, "SD"))
								CurChn = 0x01;
							else if (! stricmp_u(LStr + 6, "TOM"))
								CurChn = 0x02;
							else if (! stricmp_u(LStr + 6, "TC"))
								CurChn = 0x03;
							else if (! stricmp_u(LStr + 6, "HH"))
								CurChn = 0x04;
							if (CurChn != 0xFF)
							{
								CurChn += 18;
								TempFlag = GetBoolFromStr(RStr);
								TempCOpt->ChnMute1 &= ~(0x01 << CurChn);
								TempCOpt->ChnMute1 |= TempFlag << CurChn;
							}
						}
						else if (! strnicmp_u(LStr, "MuteWTCh", 0x08))
						{
							CurChn = (UINT8)strtol(LStr + 0x08, &TempPnt, 0);
							if (TempPnt == NULL || *TempPnt)
								break;
							if (CurChn >= CHN_MASK_CNT[CurChip])
								break;
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->ChnMute2 &= ~(0x01 << CurChn);
							TempCOpt->ChnMute2 |= TempFlag << CurChn;
						}
						break;
					//case 0x0E:	// YMF271
					//case 0x0F:	// YMZ280B
						/*if (! stricmp_u(LStr, "DisableFix"))
						{
							DISABLE_YMZ_FIX = GetBoolFromStr(RStr);
						}
						break;*/
					//case 0x10:	// RF5C164
					//case 0x11:	// PWM
					//case 0x12:	// AY8910
					case 0x13:	// GameBoy
						if (! stricmp_u(LStr, "BoostWaveChn"))
						{
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->SpecialFlags &= ~(0x01 << 0);
							TempCOpt->SpecialFlags |= TempFlag << 0;
						}
						else if (! stricmp_u(LStr, "LowerNoiseChn"))
						{
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->SpecialFlags &= ~(0x01 << 1);
							TempCOpt->SpecialFlags |= TempFlag << 1;
						}
						else if (! stricmp_u(LStr, "Inaccurate"))
						{
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->SpecialFlags &= ~(0x01 << 2);
							TempCOpt->SpecialFlags |= TempFlag << 2;
						}
						break;
					case 0x14:	// NES
						if (! stricmp_u(LStr, "SharedOpts"))
						{
							// 2 bits
							TempLng = (UINT32)strtol(RStr, NULL, 0) & 0x03;
							TempCOpt->SpecialFlags &= ~(0x03 << 0) & 0x7FFF;
							TempCOpt->SpecialFlags |= TempLng << 0;
						}
						else if (! stricmp_u(LStr, "APUOpts"))
						{
							// 2 bits
							TempLng = (UINT32)strtol(RStr, NULL, 0) & 0x03;
							TempCOpt->SpecialFlags &= ~(0x03 << 2) & 0x7FFF;
							TempCOpt->SpecialFlags |= TempLng << 2;
						}
						else if (! stricmp_u(LStr, "DMCOpts"))
						{
							// 8 bits (6 bits used)
							TempLng = (UINT32)strtol(RStr, NULL, 0) & 0xFF;
							TempCOpt->SpecialFlags &= ~(0xFF << 4) & 0x7FFF;
							TempCOpt->SpecialFlags |= TempLng << 4;
						}
						else if (! stricmp_u(LStr, "FDSOpts"))
						{
							// 1 bit
							TempLng = (UINT32)strtol(RStr, NULL, 0) & 0x01;
							TempCOpt->SpecialFlags &= ~(0x01 << 12) & 0x7FFF;
							TempCOpt->SpecialFlags |= TempLng << 12;
						}
						break;
					case 0x17:	// OKIM6258
						if (! stricmp_u(LStr, "Enable10Bit"))
						{
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->SpecialFlags &= ~(0x01 << 0);
							TempCOpt->SpecialFlags |= TempFlag << 0;
						}
						else if (! stricmp_u(LStr, "RemoveDCOfs"))
						{
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->SpecialFlags &= ~(0x01 << 1);
							TempCOpt->SpecialFlags |= TempFlag << 1;
						}
						break;
					case 0x20:	// SCSP
						if (! stricmp_u(LStr, "BypassDSP"))
						{
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->SpecialFlags &= ~(0x01 << 0);
							TempCOpt->SpecialFlags |= TempFlag << 0;
						}
						break;
					case 0x27:	// C352
						if (! stricmp_u(LStr, "DisableRear"))
						{
							TempFlag = GetBoolFromStr(RStr);
							TempCOpt->SpecialFlags &= ~(0x01 << 0);
							TempCOpt->SpecialFlags |= TempFlag << 0;
						}
						break;
					}
				}
				break;
			case 0xFF:	// Dummy Section
				break;
			}
		}
	}
	
	TempCOpt = (CHIP_OPTS*)&ChipOpts[0x00];
	TempCOpt2 = (CHIP_OPTS*)&ChipOpts[0x01];
	for (CurChip = 0x00; CurChip < CHIP_COUNT; CurChip ++, TempCOpt ++, TempCOpt2 ++)
	{
		TempCOpt2->Disabled = TempCOpt->Disabled;
		TempCOpt2->EmuCore = TempCOpt->EmuCore;
		TempCOpt2->SpecialFlags = TempCOpt->SpecialFlags;
		TempCOpt2->ChnMute1 = TempCOpt->ChnMute1;
		TempCOpt2->ChnMute2 = TempCOpt->ChnMute2;
		TempCOpt2->ChnMute3 = TempCOpt->ChnMute3;
	}
	
	fclose(hFile);
	
#ifdef WIN32
	WinNT_Check();
#endif
	if (CHIP_SAMPLE_RATE <= 0)
		CHIP_SAMPLE_RATE = SampleRate;
	
	return;
}
