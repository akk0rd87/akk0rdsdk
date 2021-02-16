/*
 * This source code is public domain.
 *
 * Authors: Olivier Lapicque <olivierl@jps.net>,
 *          Adam Goode       <adam@evdebs.org> (Endian and char fixes for PPC)
 *          Marco Trillo     <toad@arsystel.com> (Endian fixes for SaveIT, XM->IT Sample Converter)
 *
*/

#include "stdafx.h"
#include "sndfile.h"
#include "it_defs.h"

#ifdef _MSC_VER
#pragma warning(disable:4244)
#endif

static const
BYTE autovibit2xm[8] =
{ 0, 3, 1, 4, 2, 0, 0, 0 };

//////////////////////////////////////////////////////////
// Impulse Tracker IT file support

BOOL CSoundFile::ITInstrToMPT(const void *p, INSTRUMENTHEADER *penv, UINT trkvers)
//--------------------------------------------------------------------------------
{
	if (trkvers < 0x0200)
	{
		const ITOLDINSTRUMENT *pis = (const ITOLDINSTRUMENT *)p;
		memcpy(penv->name, pis->name, 26);
		memcpy(penv->filename, pis->filename, 12);
		penv->nFadeOut = bswapLE16(pis->fadeout) << 6;
		penv->nGlobalVol = 64;
		for (UINT j=0; j<NOTE_MAX; j++)
		{
			UINT note = pis->keyboard[j*2];
			UINT ins = pis->keyboard[j*2+1];
			if (ins < MAX_SAMPLES) penv->Keyboard[j] = ins;
			if (note < 128) penv->NoteMap[j] = note+1;
			else if (note >= 0xFE) penv->NoteMap[j] = note;
		}
		if (pis->flags & 0x01) penv->dwFlags |= ENV_VOLUME;
		if (pis->flags & 0x02) penv->dwFlags |= ENV_VOLLOOP;
		if (pis->flags & 0x04) penv->dwFlags |= ENV_VOLSUSTAIN;
		penv->nVolLoopStart = pis->vls;
		penv->nVolLoopEnd = pis->vle;
		penv->nVolSustainBegin = pis->sls;
		penv->nVolSustainEnd = pis->sle;
		penv->nVolEnv = 25;
		for (UINT ev=0; ev<25; ev++)
		{
			if ((penv->VolPoints[ev] = pis->nodes[ev*2]) == 0xFF)
			{
				penv->nVolEnv = ev;
				break;
			}
			penv->VolEnv[ev] = pis->nodes[ev*2+1];
		}
		penv->nNNA = pis->nna;
		penv->nDCT = pis->dnc;
		penv->nPan = 0x80;
	} else
	{
		const ITINSTRUMENT *pis = (const ITINSTRUMENT *)p;
		memcpy(penv->name, pis->name, 26);
		memcpy(penv->filename, pis->filename, 12);
		penv->nMidiProgram = pis->mpr;
		penv->nMidiChannel = pis->mch;
		penv->wMidiBank = bswapLE16(pis->mbank);
		penv->nFadeOut = bswapLE16(pis->fadeout) << 5;
		penv->nGlobalVol = pis->gbv >> 1;
		if (penv->nGlobalVol > 64) penv->nGlobalVol = 64;
		for (UINT j=0; j<NOTE_MAX; j++)
		{
			UINT note = pis->keyboard[j*2];
			UINT ins = pis->keyboard[j*2+1];
			if (ins < MAX_SAMPLES) penv->Keyboard[j] = ins;
			if (note < 128) penv->NoteMap[j] = note+1;
			else if (note >= 0xFE) penv->NoteMap[j] = note;
		}
		// Volume Envelope
		if (pis->volenv.flags & 1) penv->dwFlags |= ENV_VOLUME;
		if (pis->volenv.flags & 2) penv->dwFlags |= ENV_VOLLOOP;
		if (pis->volenv.flags & 4) penv->dwFlags |= ENV_VOLSUSTAIN;
		if (pis->volenv.flags & 8) penv->dwFlags |= ENV_VOLCARRY;
		penv->nVolEnv = pis->volenv.num;
		if (penv->nVolEnv > 25) penv->nVolEnv = 25;

		penv->nVolLoopStart = pis->volenv.lpb;
		penv->nVolLoopEnd = pis->volenv.lpe;
		penv->nVolSustainBegin = pis->volenv.slb;
		penv->nVolSustainEnd = pis->volenv.sle;
		// Panning Envelope
		if (pis->panenv.flags & 1) penv->dwFlags |= ENV_PANNING;
		if (pis->panenv.flags & 2) penv->dwFlags |= ENV_PANLOOP;
		if (pis->panenv.flags & 4) penv->dwFlags |= ENV_PANSUSTAIN;
		if (pis->panenv.flags & 8) penv->dwFlags |= ENV_PANCARRY;
		penv->nPanEnv = pis->panenv.num;
		if (penv->nPanEnv > 25) penv->nPanEnv = 25;
		penv->nPanLoopStart = pis->panenv.lpb;
		penv->nPanLoopEnd = pis->panenv.lpe;
		penv->nPanSustainBegin = pis->panenv.slb;
		penv->nPanSustainEnd = pis->panenv.sle;
		// Pitch Envelope
		if (pis->pitchenv.flags & 1) penv->dwFlags |= ENV_PITCH;
		if (pis->pitchenv.flags & 2) penv->dwFlags |= ENV_PITCHLOOP;
		if (pis->pitchenv.flags & 4) penv->dwFlags |= ENV_PITCHSUSTAIN;
		if (pis->pitchenv.flags & 8) penv->dwFlags |= ENV_PITCHCARRY;
		if (pis->pitchenv.flags & 0x80) penv->dwFlags |= ENV_FILTER;
		penv->nPitchEnv = pis->pitchenv.num;
		if (penv->nPitchEnv > 25) penv->nPitchEnv = 25;
		penv->nPitchLoopStart = pis->pitchenv.lpb;
		penv->nPitchLoopEnd = pis->pitchenv.lpe;
		penv->nPitchSustainBegin = pis->pitchenv.slb;
		penv->nPitchSustainEnd = pis->pitchenv.sle;
		// Envelopes Data
		for (UINT ev=0; ev<25; ev++)
		{
			penv->VolEnv[ev] = pis->volenv.data[ev*3];
			penv->VolPoints[ev] = (pis->volenv.data[ev*3+2] << 8) | (pis->volenv.data[ev*3+1]);
			penv->PanEnv[ev] = pis->panenv.data[ev*3] + 32;
			penv->PanPoints[ev] = (pis->panenv.data[ev*3+2] << 8) | (pis->panenv.data[ev*3+1]);
			penv->PitchEnv[ev] = pis->pitchenv.data[ev*3] + 32;
			penv->PitchPoints[ev] = (pis->pitchenv.data[ev*3+2] << 8) | (pis->pitchenv.data[ev*3+1]);
		}
		penv->nNNA = pis->nna;
		penv->nDCT = pis->dct;
		penv->nDNA = pis->dca;
		penv->nPPS = pis->pps;
		penv->nPPC = pis->ppc;
		penv->nIFC = pis->ifc;
		penv->nIFR = pis->ifr;
		penv->nVolSwing = pis->rv;
		penv->nPanSwing = pis->rp;
		penv->nPan = (pis->dfp & 0x7F) << 2;
		if (penv->nPan > 256) penv->nPan = 128;
		if (pis->dfp < 0x80) penv->dwFlags |= ENV_SETPANNING;
	}
	if ((penv->nVolLoopStart >= 25) || (penv->nVolLoopEnd >= 25)) penv->dwFlags &= ~ENV_VOLLOOP;
	if ((penv->nVolSustainBegin >= 25) || (penv->nVolSustainEnd >= 25)) penv->dwFlags &= ~ENV_VOLSUSTAIN;
	return TRUE;
}


BOOL CSoundFile::ReadIT(const BYTE *lpStream, DWORD dwMemLength)
//--------------------------------------------------------------
{
	DWORD dwMemPos = sizeof(ITFILEHEADER);
	DWORD inspos[MAX_INSTRUMENTS];
	DWORD smppos[MAX_SAMPLES];
	DWORD patpos[MAX_PATTERNS];
	BYTE chnmask[64], channels_used[64];
	MODCOMMAND lastvalue[64];
	UINT j;

	if ((!lpStream) || (dwMemLength < sizeof(ITFILEHEADER))) return FALSE;
	ITFILEHEADER pifh = *(ITFILEHEADER *)lpStream;

	pifh.id = bswapLE32(pifh.id);
	pifh.reserved1 = bswapLE16(pifh.reserved1);
	pifh.ordnum = bswapLE16(pifh.ordnum);
	pifh.insnum = bswapLE16(pifh.insnum);
	pifh.smpnum = bswapLE16(pifh.smpnum);
	pifh.patnum = bswapLE16(pifh.patnum);
	pifh.cwtv = bswapLE16(pifh.cwtv);
	pifh.cmwt = bswapLE16(pifh.cmwt);
	pifh.flags = bswapLE16(pifh.flags);
	pifh.special = bswapLE16(pifh.special);
	pifh.msglength = bswapLE16(pifh.msglength);
	pifh.msgoffset = bswapLE32(pifh.msgoffset);
	pifh.reserved2 = bswapLE32(pifh.reserved2);

	if ((pifh.id != 0x4D504D49) || (pifh.insnum >= MAX_INSTRUMENTS)
	 || (!pifh.smpnum) || (pifh.smpnum >= MAX_INSTRUMENTS) || (!pifh.ordnum)) return FALSE;
	if (dwMemPos + pifh.ordnum + pifh.insnum*4
	 + pifh.smpnum*4 + pifh.patnum*4 > dwMemLength) return FALSE;
	m_nType = MOD_TYPE_IT;
	if (pifh.flags & 0x08) m_dwSongFlags |= SONG_LINEARSLIDES;
	if (pifh.flags & 0x10) m_dwSongFlags |= SONG_ITOLDEFFECTS;
	if (pifh.flags & 0x20) m_dwSongFlags |= SONG_ITCOMPATMODE;
	if (pifh.flags & 0x80) m_dwSongFlags |= SONG_EMBEDMIDICFG;
	if (pifh.flags & 0x1000) m_dwSongFlags |= SONG_EXFILTERRANGE;
	memcpy(m_szNames[0], pifh.songname, 26);
	m_szNames[0][26] = 0;
	// Global Volume
	if (pifh.globalvol)
	{
		m_nDefaultGlobalVolume = pifh.globalvol << 1;
		if (!m_nDefaultGlobalVolume) m_nDefaultGlobalVolume = 256;
		if (m_nDefaultGlobalVolume > 256) m_nDefaultGlobalVolume = 256;
	}
	if (pifh.speed) m_nDefaultSpeed = pifh.speed;
	if (pifh.tempo) m_nDefaultTempo = pifh.tempo;
	m_nSongPreAmp = pifh.mv & 0x7F;
	// Reading Channels Pan Positions
	for (int ipan=0; ipan<64; ipan++) if (pifh.chnpan[ipan] != 0xFF)
	{
		ChnSettings[ipan].nVolume = pifh.chnvol[ipan];
		ChnSettings[ipan].nPan = 128;
		if (pifh.chnpan[ipan] & 0x80) ChnSettings[ipan].dwFlags |= CHN_MUTE;
		UINT n = pifh.chnpan[ipan] & 0x7F;
		if (n <= 64) ChnSettings[ipan].nPan = n << 2;
		if (n == 100) ChnSettings[ipan].dwFlags |= CHN_SURROUND;
	}
	if (m_nChannels < 4) m_nChannels = 4;
	// Reading Song Message
	if ((pifh.special & 0x01) && (pifh.msglength) && (pifh.msglength <= dwMemLength) && (pifh.msgoffset < dwMemLength - pifh.msglength))
	{
		m_lpszSongComments = new char[pifh.msglength+1];
		if (m_lpszSongComments)
		{
			memcpy(m_lpszSongComments, lpStream+pifh.msgoffset, pifh.msglength);
			m_lpszSongComments[pifh.msglength] = 0;
		}
	}
	// Reading orders
	UINT nordsize = pifh.ordnum;
	if (nordsize > MAX_ORDERS) nordsize = MAX_ORDERS;
	memcpy(Order, lpStream+dwMemPos, nordsize);
	dwMemPos += pifh.ordnum;
	// Reading Instrument Offsets
	memset(inspos, 0, sizeof(inspos));
	UINT inspossize = pifh.insnum;
	if (inspossize > MAX_INSTRUMENTS) inspossize = MAX_INSTRUMENTS;
	inspossize <<= 2;
	memcpy(inspos, lpStream+dwMemPos, inspossize);
	for (j=0; j < (inspossize>>2); j++)
	{
	       inspos[j] = bswapLE32(inspos[j]);
	}
	dwMemPos += pifh.insnum * 4;
	// Reading Samples Offsets
	memset(smppos, 0, sizeof(smppos));
	UINT smppossize = pifh.smpnum;
	if (smppossize > MAX_SAMPLES) smppossize = MAX_SAMPLES;
	smppossize <<= 2;
	memcpy(smppos, lpStream+dwMemPos, smppossize);
	for (j=0; j < (smppossize>>2); j++)
	{
	       smppos[j] = bswapLE32(smppos[j]);
	}
	dwMemPos += pifh.smpnum * 4;
	// Reading Patterns Offsets
	memset(patpos, 0, sizeof(patpos));
	UINT patpossize = pifh.patnum;
	if (patpossize > MAX_PATTERNS) patpossize = MAX_PATTERNS;
	patpossize <<= 2;
	memcpy(patpos, lpStream+dwMemPos, patpossize);
	for (j=0; j < (patpossize>>2); j++)
	{
	       patpos[j] = bswapLE32(patpos[j]);
	}
	dwMemPos += pifh.patnum * 4;
	// Reading IT Extra Info
	if (dwMemPos + 2 < dwMemLength)
	{
		UINT nflt = bswapLE16(*((WORD *)(lpStream + dwMemPos)));
		dwMemPos += 2;
		if (dwMemPos + nflt * 8 < dwMemLength) dwMemPos += nflt * 8;
	}
	// Reading Midi Output & Macros
	if (m_dwSongFlags & SONG_EMBEDMIDICFG)
	{
		if (dwMemPos + sizeof(MODMIDICFG) < dwMemLength)
		{
			memcpy(&m_MidiCfg, lpStream+dwMemPos, sizeof(MODMIDICFG));
			dwMemPos += sizeof(MODMIDICFG);
		}
	}
	// Read pattern names: "PNAM"
	if ((dwMemPos + 8 < dwMemLength) && (bswapLE32(*((DWORD *)(lpStream+dwMemPos))) == 0x4d414e50))
	{
		UINT len = bswapLE32(*((DWORD *)(lpStream+dwMemPos+4)));
		dwMemPos += 8;
		if ((dwMemPos + len <= dwMemLength) && (len <= MAX_PATTERNS*MAX_PATTERNNAME) && (len >= MAX_PATTERNNAME))
		{
			m_lpszPatternNames = new char[len];
			if (m_lpszPatternNames)
			{
				m_nPatternNames = len / MAX_PATTERNNAME;
				memcpy(m_lpszPatternNames, lpStream+dwMemPos, len);
			}
			dwMemPos += len;
		}
	}
	// 4-channels minimum
	m_nChannels = 4;
	// Read channel names: "CNAM"
	if ((dwMemPos + 8 < dwMemLength) && (bswapLE32(*((DWORD *)(lpStream+dwMemPos))) == 0x4d414e43))
	{
		UINT len = bswapLE32(*((DWORD *)(lpStream+dwMemPos+4)));
		dwMemPos += 8;
		if ((dwMemPos + len <= dwMemLength) && (len <= 64*MAX_CHANNELNAME))
		{
			UINT n = len / MAX_CHANNELNAME;
			if (n > m_nChannels) m_nChannels = n;
			for (UINT i=0; i<n; i++)
			{
				memcpy(ChnSettings[i].szName, (lpStream+dwMemPos+i*MAX_CHANNELNAME), MAX_CHANNELNAME);
				ChnSettings[i].szName[MAX_CHANNELNAME-1] = 0;
			}
			dwMemPos += len;
		}
	}
	// Read mix plugins information
	if (dwMemPos + 8 < dwMemLength)
	{
		dwMemPos += LoadMixPlugins(lpStream+dwMemPos, dwMemLength-dwMemPos);
	}
	// Checking for unused channels
	UINT npatterns = pifh.patnum;
	if (npatterns > MAX_PATTERNS) npatterns = MAX_PATTERNS;
	for (UINT patchk=0; patchk<npatterns; patchk++)
	{
		memset(chnmask, 0, sizeof(chnmask));
		if ((!patpos[patchk]) || ((DWORD)patpos[patchk] >= dwMemLength - 4)) continue;
		UINT len = bswapLE16(*((WORD *)(lpStream+patpos[patchk])));
		UINT rows = bswapLE16(*((WORD *)(lpStream+patpos[patchk]+2)));
		if ((rows < 4) || (rows > 256)) continue;
		if (8+len > dwMemLength || patpos[patchk] > dwMemLength - (8+len)) continue;
		UINT i = 0;
		const BYTE *p = lpStream+patpos[patchk]+8;
		UINT nrow = 0;
		while (nrow<rows)
		{
			if (i >= len) break;
			BYTE b = p[i++];
			if (!b)
			{
				nrow++;
				continue;
			}
			UINT ch = b & 0x7F;
			if (ch) ch = (ch - 1) & 0x3F;
			if (b & 0x80)
			{
				if (i >= len) break;
				chnmask[ch] = p[i++];
			}
			// Channel used
			if (chnmask[ch] & 0x0F)
			{
				if ((ch >= m_nChannels) && (ch < 64)) m_nChannels = ch+1;
			}
			// Note
			if (chnmask[ch] & 1) i++;
			// Instrument
			if (chnmask[ch] & 2) i++;
			// Volume
			if (chnmask[ch] & 4) i++;
			// Effect
			if (chnmask[ch] & 8) i += 2;
			if (i >= len) break;
		}
	}
	// Reading Instruments
	m_nInstruments = 0;
	if (pifh.flags & 0x04) m_nInstruments = pifh.insnum;
	if (m_nInstruments >= MAX_INSTRUMENTS) m_nInstruments = MAX_INSTRUMENTS-1;
	for (UINT nins=0; nins<m_nInstruments; nins++)
	{
		if ((inspos[nins] > 0) && dwMemLength > sizeof(ITOLDINSTRUMENT) &&
			(inspos[nins] < dwMemLength - sizeof(ITOLDINSTRUMENT)))
		{
			INSTRUMENTHEADER *penv = new INSTRUMENTHEADER;
			if (!penv) continue;
			Headers[nins+1] = penv;
			memset(penv, 0, sizeof(INSTRUMENTHEADER));
			ITInstrToMPT(lpStream + inspos[nins], penv, pifh.cmwt);
		}
	}
	// Reading Samples
	m_nSamples = pifh.smpnum;
	if (m_nSamples >= MAX_SAMPLES) m_nSamples = MAX_SAMPLES-1;
	for (UINT nsmp=0; nsmp<pifh.smpnum; nsmp++) if ((smppos[nsmp]) && (smppos[nsmp] <= dwMemLength - sizeof(ITSAMPLESTRUCT)))
	{
		ITSAMPLESTRUCT pis = *(ITSAMPLESTRUCT *)(lpStream+smppos[nsmp]);
		pis.id = bswapLE32(pis.id);
		pis.length = bswapLE32(pis.length);
		pis.loopbegin = bswapLE32(pis.loopbegin);
		pis.loopend = bswapLE32(pis.loopend);
		pis.C5Speed = bswapLE32(pis.C5Speed);
		pis.susloopbegin = bswapLE32(pis.susloopbegin);
		pis.susloopend = bswapLE32(pis.susloopend);
		pis.samplepointer = bswapLE32(pis.samplepointer);

		if (pis.id == 0x53504D49)
		{
			MODINSTRUMENT *pins = &Ins[nsmp+1];
			memcpy(pins->name, pis.filename, 12);
			pins->uFlags = 0;
			pins->nLength = 0;
			pins->nLoopStart = pis.loopbegin;
			pins->nLoopEnd = pis.loopend;
			pins->nSustainStart = pis.susloopbegin;
			pins->nSustainEnd = pis.susloopend;
			pins->nC4Speed = pis.C5Speed;
			if (!pins->nC4Speed) pins->nC4Speed = 8363;
			if (pis.C5Speed < 256) pins->nC4Speed = 256;
			pins->nVolume = pis.vol << 2;
			if (pins->nVolume > 256) pins->nVolume = 256;
			pins->nGlobalVol = pis.gvl;
			if (pins->nGlobalVol > 64) pins->nGlobalVol = 64;
			if (pis.flags & 0x10) pins->uFlags |= CHN_LOOP;
			if (pis.flags & 0x20) pins->uFlags |= CHN_SUSTAINLOOP;
			if (pis.flags & 0x40) pins->uFlags |= CHN_PINGPONGLOOP;
			if (pis.flags & 0x80) pins->uFlags |= CHN_PINGPONGSUSTAIN;
			pins->nPan = (pis.dfp & 0x7F) << 2;
			if (pins->nPan > 256) pins->nPan = 256;
			if (pis.dfp & 0x80) pins->uFlags |= CHN_PANNING;
			pins->nVibType = autovibit2xm[pis.vit & 7];
			pins->nVibRate = pis.vis;
			pins->nVibDepth = pis.vid & 0x7F;
			pins->nVibSweep = (pis.vir + 3) / 4;
			if ((pis.samplepointer) && (pis.samplepointer < dwMemLength) && (pis.length))
			{
				pins->nLength = pis.length;
				if (pins->nLength > MAX_SAMPLE_LENGTH) pins->nLength = MAX_SAMPLE_LENGTH;
				UINT flags = (pis.cvt & 1) ? RS_PCM8S : RS_PCM8U;
				if (pis.flags & 2)
				{
					flags += 5;
					if (pis.flags & 4) flags |= RSF_STEREO;
					pins->uFlags |= CHN_16BIT;
					// IT 2.14 16-bit packed sample ?
					if (pis.flags & 8) flags = ((pifh.cmwt >= 0x215) && (pis.cvt & 4)) ? RS_IT21516 : RS_IT21416;
				} else
				{
					if (pis.flags & 4) flags |= RSF_STEREO;
					if (pis.cvt == 0xFF) flags = RS_ADPCM4; else
					// IT 2.14 8-bit packed sample ?
					if (pis.flags & 8)	flags =	((pifh.cmwt >= 0x215) && (pis.cvt & 4)) ? RS_IT2158 : RS_IT2148;
				}
				ReadSample(&Ins[nsmp+1], flags, (LPSTR)(lpStream+pis.samplepointer), dwMemLength - pis.samplepointer);
			}
		}
		memcpy(m_szNames[nsmp+1], pis.name, 26);
	}
	// Reading Patterns
	for (UINT npat=0; npat<npatterns; npat++)
	{
		if ((!patpos[npat]) || ((DWORD)patpos[npat] >= dwMemLength - 4))
		{
			PatternSize[npat] = 64;
			Patterns[npat] = AllocatePattern(64, m_nChannels);
			continue;
		}

		UINT len = bswapLE16(*((WORD *)(lpStream+patpos[npat])));
		UINT rows = bswapLE16(*((WORD *)(lpStream+patpos[npat]+2)));
		if ((rows < 4) || (rows > 256)) continue;
		if (8+len > dwMemLength || patpos[npat] > dwMemLength - (8+len)) continue;
		PatternSize[npat] = rows;
		if ((Patterns[npat] = AllocatePattern(rows, m_nChannels)) == NULL) continue;
		memset(lastvalue, 0, sizeof(lastvalue));
		memset(chnmask, 0, sizeof(chnmask));
		MODCOMMAND *m = Patterns[npat];
		UINT i = 0;
		const BYTE *p = lpStream+patpos[npat]+8;
		UINT nrow = 0;
		while (nrow<rows)
		{
			if (i >= len) break;
			BYTE b = p[i++];
			if (!b)
			{
				nrow++;
				m+=m_nChannels;
				continue;
			}
			UINT ch = b & 0x7F;
			if (ch) ch = (ch - 1) & 0x3F;
			if (b & 0x80)
			{
				if (i >= len) break;
				chnmask[ch] = p[i++];
			}
			if ((chnmask[ch] & 0x10) && (ch < m_nChannels))
			{
				m[ch].note = lastvalue[ch].note;
			}
			if ((chnmask[ch] & 0x20) && (ch < m_nChannels))
			{
				m[ch].instr = lastvalue[ch].instr;
			}
			if ((chnmask[ch] & 0x40) && (ch < m_nChannels))
			{
				m[ch].volcmd = lastvalue[ch].volcmd;
				m[ch].vol = lastvalue[ch].vol;
			}
			if ((chnmask[ch] & 0x80) && (ch < m_nChannels))
			{
				m[ch].command = lastvalue[ch].command;
				m[ch].param = lastvalue[ch].param;
			}
			if (chnmask[ch] & 1)	// Note
			{
				if (i >= len) break;
				UINT note = p[i++];
				if (ch < m_nChannels)
				{
					if (note < 0x80) note++;
					m[ch].note = note;
					lastvalue[ch].note = note;
					channels_used[ch] = TRUE;
				}
			}
			if (chnmask[ch] & 2)
			{
				if (i >= len) break;
				UINT instr = p[i++];
				if (ch < m_nChannels)
				{
					m[ch].instr = instr;
					lastvalue[ch].instr = instr;
				}
			}
			if (chnmask[ch] & 4)
			{
				if (i >= len) break;
				UINT vol = p[i++];
				if (ch < m_nChannels)
				{
					// 0-64: Set Volume
					if (vol <= 64) { m[ch].volcmd = VOLCMD_VOLUME; m[ch].vol = vol; } else
					// 128-192: Set Panning
					if ((vol >= 128) && (vol <= 192)) { m[ch].volcmd = VOLCMD_PANNING; m[ch].vol = vol - 128; } else
					// 65-74: Fine Volume Up
					if (vol < 75) { m[ch].volcmd = VOLCMD_FINEVOLUP; m[ch].vol = vol - 65; } else
					// 75-84: Fine Volume Down
					if (vol < 85) { m[ch].volcmd = VOLCMD_FINEVOLDOWN; m[ch].vol = vol - 75; } else
					// 85-94: Volume Slide Up
					if (vol < 95) { m[ch].volcmd = VOLCMD_VOLSLIDEUP; m[ch].vol = vol - 85; } else
					// 95-104: Volume Slide Down
					if (vol < 105) { m[ch].volcmd = VOLCMD_VOLSLIDEDOWN; m[ch].vol = vol - 95; } else
					// 105-114: Pitch Slide Up
					if (vol < 115) { m[ch].volcmd = VOLCMD_PORTADOWN; m[ch].vol = vol - 105; } else
					// 115-124: Pitch Slide Down
					if (vol < 125) { m[ch].volcmd = VOLCMD_PORTAUP; m[ch].vol = vol - 115; } else
					// 193-202: Portamento To
					if ((vol >= 193) && (vol <= 202)) { m[ch].volcmd = VOLCMD_TONEPORTAMENTO; m[ch].vol = vol - 193; } else
					// 203-212: Vibrato
					if ((vol >= 203) && (vol <= 212)) { m[ch].volcmd = VOLCMD_VIBRATOSPEED; m[ch].vol = vol - 203; }
					lastvalue[ch].volcmd = m[ch].volcmd;
					lastvalue[ch].vol = m[ch].vol;
				}
			}
			// Reading command/param
			if (chnmask[ch] & 8)
			{
				if (i > len - 2) break;
				UINT cmd = p[i++];
				UINT param = p[i++];
				if (ch < m_nChannels)
				{
					if (cmd)
					{
						m[ch].command = cmd;
						m[ch].param = param;
						S3MConvert(&m[ch], TRUE);
						lastvalue[ch].command = m[ch].command;
						lastvalue[ch].param = m[ch].param;
					}
				}
			}
		}
	}
	for (UINT ncu=0; ncu<MAX_BASECHANNELS; ncu++)
	{
		if (ncu>=m_nChannels)
		{
			ChnSettings[ncu].nVolume = 64;
			ChnSettings[ncu].dwFlags &= ~CHN_MUTE;
		}
	}
	m_nMinPeriod = 8;
	m_nMaxPeriod = 0xF000;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
// IT 2.14 compression

DWORD ITReadBits(DWORD &bitbuf, UINT &bitnum, LPBYTE &ibuf, CHAR n)
//-----------------------------------------------------------------
{
	DWORD retval = 0;
	UINT i = n;

	// explicit if read 0 bits, then return 0
	if (i == 0)
		return(0);

	if (n > 0)
	{
		do
		{
			if (!bitnum)
			{
				bitbuf = *ibuf++;
				bitnum = 8;
			}
			retval >>= 1;
			retval |= bitbuf << 31;
			bitbuf >>= 1;
			bitnum--;
			i--;
		} while (i);
		i = n;
	}
	return (retval >> (32-i));
}

#define IT215_SUPPORT
void ITUnpack8Bit(signed char *pSample, DWORD dwLen, LPBYTE lpMemFile, DWORD dwMemLength, BOOL b215)
//-------------------------------------------------------------------------------------------
{
	signed char *pDst = pSample;
	LPBYTE pSrc = lpMemFile;
	DWORD wHdr = 0;
	DWORD wCount = 0;
	DWORD bitbuf = 0;
	UINT bitnum = 0;
	BYTE bLeft = 0, bTemp = 0, bTemp2 = 0;

	while (dwLen)
	{
		if (!wCount)
		{
			wCount = 0x8000;
			wHdr = bswapLE16(*((LPWORD)pSrc));
			pSrc += 2;
			bLeft = 9;
			bTemp = bTemp2 = 0;
			bitbuf = bitnum = 0;
		}
		DWORD d = wCount;
		if (d > dwLen) d = dwLen;
		// Unpacking
		DWORD dwPos = 0;
		do
		{
			WORD wBits = (WORD)ITReadBits(bitbuf, bitnum, pSrc, bLeft);
			if (bLeft < 7)
			{
				DWORD i = 1 << (bLeft-1);
				DWORD j = wBits & 0xFFFF;
				if (i != j) goto UnpackByte;
				wBits = (WORD)(ITReadBits(bitbuf, bitnum, pSrc, 3) + 1) & 0xFF;
				bLeft = ((BYTE)wBits < bLeft) ? (BYTE)wBits : (BYTE)((wBits+1) & 0xFF);
				goto Next;
			}
			if (bLeft < 9)
			{
				WORD i = (0xFF >> (9 - bLeft)) + 4;
				WORD j = i - 8;
				if ((wBits <= j) || (wBits > i)) goto UnpackByte;
				wBits -= j;
				bLeft = ((BYTE)(wBits & 0xFF) < bLeft) ? (BYTE)(wBits & 0xFF) : (BYTE)((wBits+1) & 0xFF);
				goto Next;
			}
			if (bLeft >= 10) goto SkipByte;
			if (wBits >= 256)
			{
				bLeft = (BYTE)(wBits + 1) & 0xFF;
				goto Next;
			}
		UnpackByte:
			if (bLeft < 8)
			{
				BYTE shift = 8 - bLeft;
				signed char c = (signed char)(wBits << shift);
				c >>= shift;
				wBits = (WORD)c;
			}
			wBits += bTemp;
			bTemp = (BYTE)wBits;
			bTemp2 += bTemp;
#ifdef IT215_SUPPORT
			pDst[dwPos] = (b215) ? bTemp2 : bTemp;
#else
			pDst[dwPos] = bTemp;
#endif
		SkipByte:
			dwPos++;
		Next:
			if (pSrc >= lpMemFile+dwMemLength+1) return;
		} while (dwPos < d);
		// Move On
		wCount -= d;
		dwLen -= d;
		pDst += d;
	}
}


void ITUnpack16Bit(signed char *pSample, DWORD dwLen, LPBYTE lpMemFile, DWORD dwMemLength, BOOL b215)
//--------------------------------------------------------------------------------------------
{
	signed short *pDst = (signed short *)pSample;
	LPBYTE pSrc = lpMemFile;
	DWORD wHdr = 0;
	DWORD wCount = 0;
	DWORD bitbuf = 0;
	UINT bitnum = 0;
	BYTE bLeft = 0;
	signed short wTemp = 0, wTemp2 = 0;

	while (dwLen)
	{
		if (!wCount)
		{
			wCount = 0x4000;
			wHdr = bswapLE16(*((LPWORD)pSrc));
			pSrc += 2;
			bLeft = 17;
			wTemp = wTemp2 = 0;
			bitbuf = bitnum = 0;
		}
		DWORD d = wCount;
		if (d > dwLen) d = dwLen;
		// Unpacking
		DWORD dwPos = 0;
		do
		{
			DWORD dwBits = ITReadBits(bitbuf, bitnum, pSrc, bLeft);
			if (bLeft < 7)
			{
				DWORD i = 1 << (bLeft-1);
				DWORD j = dwBits;
				if (i != j) goto UnpackByte;
				dwBits = ITReadBits(bitbuf, bitnum, pSrc, 4) + 1;
				bLeft = ((BYTE)(dwBits & 0xFF) < bLeft) ? (BYTE)(dwBits & 0xFF) : (BYTE)((dwBits+1) & 0xFF);
				goto Next;
			}
			if (bLeft < 17)
			{
				DWORD i = (0xFFFF >> (17 - bLeft)) + 8;
				DWORD j = (i - 16) & 0xFFFF;
				if ((dwBits <= j) || (dwBits > (i & 0xFFFF))) goto UnpackByte;
				dwBits -= j;
				bLeft = ((BYTE)(dwBits & 0xFF) < bLeft) ? (BYTE)(dwBits & 0xFF) : (BYTE)((dwBits+1) & 0xFF);
				goto Next;
			}
			if (bLeft >= 18) goto SkipByte;
			if (dwBits >= 0x10000)
			{
				bLeft = (BYTE)(dwBits + 1) & 0xFF;
				goto Next;
			}
		UnpackByte:
			if (bLeft < 16)
			{
				BYTE shift = 16 - bLeft;
				signed short c = (signed short)(dwBits << shift);
				c >>= shift;
				dwBits = (DWORD)c;
			}
			dwBits += wTemp;
			wTemp = (signed short)dwBits;
			wTemp2 += wTemp;
#ifdef IT215_SUPPORT
			pDst[dwPos] = (b215) ? wTemp2 : wTemp;
#else
			pDst[dwPos] = wTemp;
#endif
		SkipByte:
			dwPos++;
		Next:
			if (pSrc >= lpMemFile+dwMemLength+1) return;
		} while (dwPos < d);
		// Move On
		wCount -= d;
		dwLen -= d;
		pDst += d;
		if (pSrc >= lpMemFile+dwMemLength) break;
	}
}


UINT CSoundFile::SaveMixPlugins(FILE *f, BOOL bUpdate)
//----------------------------------------------------
{
	DWORD chinfo[64];
	CHAR s[32];
	DWORD nPluginSize, writeSwapDWORD;
	SNDMIXPLUGININFO writePluginInfo;
	UINT nTotalSize = 0;
	UINT nChInfo = 0;

	for (UINT i=0; i<MAX_MIXPLUGINS; i++)
	{
		PSNDMIXPLUGIN p = &m_MixPlugins[i];
		if ((p->Info.dwPluginId1) || (p->Info.dwPluginId2))
		{
			nPluginSize = sizeof(SNDMIXPLUGININFO)+4; // plugininfo+4 (datalen)
			if ((p->pMixPlugin) && (bUpdate))
			{
				p->pMixPlugin->SaveAllParameters();
			}
			if (p->pPluginData)
			{
				nPluginSize += p->nPluginDataSize;
			}
			if (f)
			{
				s[0] = 'F';
				s[1] = 'X';
				s[2] = '0' + (i/10);
				s[3] = '0' + (i%10);
				fwrite(s, 1, 4, f);
				writeSwapDWORD = bswapLE32(nPluginSize);
				fwrite(&writeSwapDWORD, 1, 4, f);

				// Copy Information To Be Written for ByteSwapping
				memcpy(&writePluginInfo, &p->Info, sizeof(SNDMIXPLUGININFO));
				writePluginInfo.dwPluginId1 = bswapLE32(p->Info.dwPluginId1);
				writePluginInfo.dwPluginId2 = bswapLE32(p->Info.dwPluginId2);
				writePluginInfo.dwInputRouting = bswapLE32(p->Info.dwInputRouting);
				writePluginInfo.dwOutputRouting = bswapLE32(p->Info.dwOutputRouting);
				for (UINT j=0; j<4; j++)
				{
				        writePluginInfo.dwReserved[j] = bswapLE32(p->Info.dwReserved[j]);
				}

				fwrite(&writePluginInfo, 1, sizeof(SNDMIXPLUGININFO), f);
				writeSwapDWORD = bswapLE32(m_MixPlugins[i].nPluginDataSize);
				fwrite(&writeSwapDWORD, 1, 4, f);
				if (m_MixPlugins[i].pPluginData)
				{
					fwrite(m_MixPlugins[i].pPluginData, 1, m_MixPlugins[i].nPluginDataSize, f);
				}
			}
			nTotalSize += nPluginSize + 8;
		}
	}
	for (UINT j=0; j<m_nChannels; j++)
	{
		if (j < 64)
		{
			if ((chinfo[j] = ChnSettings[j].nMixPlugin) != 0)
			{
				nChInfo = j+1;
				chinfo[j] = bswapLE32(chinfo[j]); // inplace BS
			}
		}
	}
	if (nChInfo)
	{
		if (f)
		{
			nPluginSize = bswapLE32(0x58464843);
			fwrite(&nPluginSize, 1, 4, f);
			nPluginSize = nChInfo*4;
			writeSwapDWORD = bswapLE32(nPluginSize);
			fwrite(&writeSwapDWORD, 1, 4, f);
			fwrite(chinfo, 1, nPluginSize, f);
		}
		nTotalSize += nChInfo*4 + 8;
	}
	return nTotalSize;
}


UINT CSoundFile::LoadMixPlugins(const void *pData, UINT nLen)
//-----------------------------------------------------------
{
	const BYTE *p = (const BYTE *)pData;
	UINT nPos = 0;

	while (nPos+8 < nLen)
	{
		DWORD nPluginSize;
		UINT nPlugin;

		nPluginSize = bswapLE32(*(DWORD *)(p+nPos+4));
		if (nPluginSize > nLen-nPos-8) break;;
		if ((bswapLE32(*(DWORD *)(p+nPos))) == 0x58464843)
		{
			for (UINT ch=0; ch<64; ch++) if (ch*4 < nPluginSize)
			{
				ChnSettings[ch].nMixPlugin = bswapLE32(*(DWORD *)(p+nPos+8+ch*4));
			}
		} else
		{
			if ((p[nPos] != 'F') || (p[nPos+1] != 'X')
			 || (p[nPos+2] < '0') || (p[nPos+3] < '0'))
			{
				break;
			}
			nPlugin = (p[nPos+2]-'0')*10 + (p[nPos+3]-'0');
			if ((nPlugin < MAX_MIXPLUGINS) && (nPluginSize >= sizeof(SNDMIXPLUGININFO)+4))
			{
				DWORD dwExtra = bswapLE32(*(DWORD *)(p+nPos+8+sizeof(SNDMIXPLUGININFO)));
				m_MixPlugins[nPlugin].Info = *(const SNDMIXPLUGININFO *)(p+nPos+8);
				m_MixPlugins[nPlugin].Info.dwPluginId1 = bswapLE32(m_MixPlugins[nPlugin].Info.dwPluginId1);
				m_MixPlugins[nPlugin].Info.dwPluginId2 = bswapLE32(m_MixPlugins[nPlugin].Info.dwPluginId2);
				m_MixPlugins[nPlugin].Info.dwInputRouting = bswapLE32(m_MixPlugins[nPlugin].Info.dwInputRouting);
				m_MixPlugins[nPlugin].Info.dwOutputRouting = bswapLE32(m_MixPlugins[nPlugin].Info.dwOutputRouting);
				for (UINT j=0; j<4; j++)
				{
				        m_MixPlugins[nPlugin].Info.dwReserved[j] = bswapLE32(m_MixPlugins[nPlugin].Info.dwReserved[j]);
				}
				if ((dwExtra) && (dwExtra <= nPluginSize-sizeof(SNDMIXPLUGININFO)-4))
				{
					m_MixPlugins[nPlugin].nPluginDataSize = 0;
					m_MixPlugins[nPlugin].pPluginData = new signed char [dwExtra];
					if (m_MixPlugins[nPlugin].pPluginData)
					{
						m_MixPlugins[nPlugin].nPluginDataSize = dwExtra;
						memcpy(m_MixPlugins[nPlugin].pPluginData, p+nPos+8+sizeof(SNDMIXPLUGININFO)+4, dwExtra);
					}
				}
			}
		}
		nPos += nPluginSize + 8;
	}
	return nPos;
}
