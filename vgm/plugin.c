#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include <locale.h>


#include "plugin.h"

#include "../ao.h"

#include "chips/mamedef.h"
#include "stdbool.h"
#include "VGMPlay.h"
#include "VGMPlay_Intf.h"

#include <iconv.h>




#define SAMPLESIZE sizeof(WAVE_16BS)


#define SAMPLERATE 2048

UINT8 CmdList[0x100]; // used by VGMPlay.c and VGMPlay_AddFmts.c
bool ErrorHappened;   // used by VGMPlay.c and VGMPlay_AddFmts.c
extern VGM_HEADER VGMHead;
extern UINT32 SampleRate;
extern UINT32 VGMMaxLoopM;
extern UINT32 VGMMaxLoop;
extern GD3_TAG VGMTag;
extern UINT32 FadeTime;
extern bool EndPlay;

// Options Variables

extern UINT32 VGMPbRate;
extern UINT32 CMFMaxLoop;
UINT32 FadeTimeN;	// normal fade time
UINT32 FadeTimePL;	// in-playlist fade time
extern UINT32 FadeTime;
UINT32 PauseTimeJ;	// Pause Time for Jingles
UINT32 PauseTimeL;	// Pause Time for Looping Songs
extern UINT32 PauseTime;
static UINT8 Show95Cmds;

extern float VolumeLevel;
extern bool SurroundSound;
extern UINT8 HardStopOldVGMs;
extern bool FadeRAWLog;
static UINT8 LogToWave;
//extern bool FullBufFill;
extern bool PauseEmulate;
extern bool DoubleSSGVol;
static UINT16 ForceAudioBuf;

extern UINT8 ResampleMode;	// 00 - HQ both, 01 - LQ downsampling, 02 - LQ both
extern UINT8 CHIP_SAMPLING_MODE;
extern INT32 CHIP_SAMPLE_RATE;

extern UINT16 FMPort;
extern bool UseFM;
extern bool FMForce;
//extern bool FMAccurate;
extern bool FMBreakFade;
extern float FMVol;

extern CHIPS_OPTION ChipOpts[0x02];

extern UINT32 GetChipClock(VGM_HEADER* FileHead, UINT8 ChipID, UINT8* RetSubType);
extern const char* GetAccurateChipName(UINT8 ChipID, UINT8 SubType);


static int preferJapTag = 0;

WAVE_16BS *wave_16bs_buf;

static int buffered_len;




char * get_chip_str(UINT8 c_id, UINT8 sub_type, UINT32 clock)
{
	if (! clock)
		return 0;
	
	if (c_id == 0x00 && (clock & 0x80000000))
		clock &= ~0x40000000;
	if (clock & 0x80000000)
	{
		clock &= ~0x80000000;
		c_id |= 0x80;
	}
	
	
	
	return GetAccurateChipName(c_id, sub_type);
}

void fill_chip_tags()
{
	UINT8 cur;
	UINT32 clock;
	UINT8 type;
	
	char *tmp;
	
	
	for (cur = 0x00; cur < CHIP_COUNT; cur++)
	{
		clock = GetChipClock(&VGMHead, cur, &type);
		if (clock && GetChipClock(&VGMHead, 0x80 | cur, 0))
			clock |= 0x40000000;
		
		tmp = get_chip_str(cur, type, clock);
		
		if (tmp!=0)
		{
			strcat(tag_chips, tmp);
			strcat(tag_chips, " ");
		}
		
	}
	
}

wchar_t * select_tagje(wchar_t *e, wchar_t *j, int preferj)
{
	if (preferj)
	{
		if (!j)
		{
			if (!e)
				return 0;
			return e;
		}
		return j;
	}
	else
	{
		if (!e)
		{
			if (!j)
				return 0;
			return j;
		}
		return e;
	}
	return 0;
}

int wstr2cstr(char *cstr, wchar_t *wcstr, int max)
{
	int i,j,ws;
	char *w;
	
	ws = sizeof(wchar_t);
	
	if (!cstr || !wcstr)
		return 0 ;
	
	if (max < (wcslen(wcstr) * ws + 4))
		return 0;
	
	for (i=0;i<wcslen(wcstr);i++)
	{
		w = (char*) &wcstr[i];
		
		
		for (j=0;j<ws;j++)
			cstr[i+j] = w[j];
	}
	
	return 1;
}


void fill_tags_iconv()
{
	iconv_t ico;
	int ret;
	size_t srcs, dsts;
	
	char *out;
	
	
	char *track_name = (char*)
		select_tagje(VGMTag.strTrackNameE, VGMTag.strTrackNameJ, preferJapTag);
	char *author_name = (char *)
		select_tagje(VGMTag.strAuthorNameE, VGMTag.strAuthorNameJ, preferJapTag);
	char *game_name = (char *)
		select_tagje(VGMTag.strGameNameE, VGMTag.strGameNameJ, preferJapTag);
	
	ico = iconv_open("UTF-8", "WCHAR_T");
	
	if ((int) ico == -1)
		return;
	
	
	if (track_name!=0)
	{
				
		if (wstr2cstr(tag_track, track_name, 256) < 0)
			safe_strcpy(tag_track, "???", 256);
	}
	if (author_name!=0)
	{
		if (wstr2cstr(tag_author, author_name, 256) < 0)
			safe_strcpy(tag_author, "???", 256);
	}
	if (game_name!=0)
	{
		if (wstr2cstr(tag_game, game_name, 256) < 0)
			safe_strcpy(tag_game, "???", 256);
	}
	
}

void fill_tags()
{
	int tmp,wl;
	char *ch;
		
	wchar_t *track_name =
		select_tagje(VGMTag.strTrackNameE, VGMTag.strTrackNameJ, preferJapTag);
	wchar_t *author_name =
		select_tagje(VGMTag.strAuthorNameE, VGMTag.strAuthorNameJ, preferJapTag);
	wchar_t *game_name =
		select_tagje(VGMTag.strGameNameE, VGMTag.strGameNameJ, preferJapTag);
	wchar_t *sys_name =
		select_tagje(VGMTag.strSystemNameE, VGMTag.strSystemNameJ, preferJapTag);
		
	
	if (track_name!=0)
	{
		tmp = wcstombs(tag_track,track_name,256);
		if (tmp <=0)
			safe_strcpy(tag_track, "???", 256);
	}
	if (author_name!=0)
	{
		tmp = wcstombs(tag_author,author_name,256);
		if (tmp <=0)
			safe_strcpy(tag_author, "???", 256);
	}
	if (game_name!=0)
	{
		tmp = wcstombs(tag_game,game_name,256);
		if (tmp <=0)
			safe_strcpy(tag_game, "???", 256);
	}
	if (sys_name!=0)
	{
		tmp = wcstombs(tag_system,sys_name,256);
		if (tmp <=0)
			safe_strcpy(tag_system, "???", 256);
	}
	
		
}

int vgm_execute ( void (*update)(const void *, int ))
{
	/* create a buffer consisting of u8 ints, and use it to satisfy update.*/
	
	
	buffered_len = FillBuffer(wave_16bs_buf, SAMPLERATE);
	
	
	if (buffered_len) /* remember that arg 2 must be size in bytes */
		update((UINT8 * ) wave_16bs_buf, SAMPLESIZE * buffered_len);
	else
		update(0, 0);
	
	return 1;
	
}

int vgm_open ( char * fn)
{
	
	
	
	VGMPlay_Init();
	
	/* loop forever */
	VGMMaxLoop = 0x00;
	
	
	
	
    VGMPlay_Init2();
    
    
    clear_tags();
    
	if ( !OpenVGMFile(fn) )
	{
		play_stat = M_ERR;
		
		return 0;
	}
	
	/* prefer english for now. */
	
	setlocale(LC_CTYPE, "");
	
	fill_tags();
	
	fill_chip_tags();
	

	PlayVGM();
	
	
	wave_16bs_buf = (WAVE_16BS *) malloc (SAMPLESIZE * SAMPLERATE);
		
	
	return 1;
}

void vgm_close ( void )
{
	free(wave_16bs_buf);
	
	StopVGM();
    CloseVGMFile();
    VGMPlay_Deinit();
}

/*

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
		case 0x00: /* general /
			if (!strcmp("SampleRate",tmp->name)  && tmp->type==E_INT)
				SampleRate = tmp->dat[0].i;
				
			if (!strcmp("PlaybackRate",tmp->name)  && tmp->type==E_INT)
				VGMPbRate = tmp->dat[0].i;
				
			if (!strcmp("DoubleSSGVol",tmp->name)  && (tmp->type==E_INT || tmp->type==E_BOOL))
				DoubleSSGVol = tmp->dat[0].i;
				
			/*if (!strcmp("PreferJapTag",tmp->name)  && (tmp->type==E_INT || tmp->type==E_BOOL) )
				PreferJapTag = tmp->dat[0].i;/
				
			if (!strcmp("FadeTime",tmp->name)  && tmp->type==E_INT)
				FadeTimeN = tmp->dat[0].i;
				
			if (!strcmp("FadeTimePL",tmp->name)  && tmp->type==E_INT)
				FadeTimePL = tmp->dat[0].i;
				
			if (!strcmp("JinglePause",tmp->name)  && tmp->type==E_INT)
				PauseTimeJ = tmp->dat[0].i;
				
			if (!strcmp("HardStopOld",tmp->name)  && (tmp->type==E_INT || tmp->type==E_BOOL))
				HardStopOldVGMs = tmp->dat[0].i; /* check original /
				
			if (!strcmp("FadeRAWLogs",tmp->name)  && (tmp->type==E_INT || tmp->type==E_BOOL))
				FadeRAWLog = tmp->dat[0].i;
				
			if (!strcmp("Volume",tmp->name)  && tmp->type==E_FLOAT)
				VolumeLevel = tmp->dat[0].f;
				
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
						break;/
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
	
	
	if (CHIP_SAMPLE_RATE <= 0)
		CHIP_SAMPLE_RATE = SampleRate;
	
	return;
}
*/
