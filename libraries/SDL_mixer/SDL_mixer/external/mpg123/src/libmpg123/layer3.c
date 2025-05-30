/*
	layer3.c: the layer 3 decoder

	copyright 1995-2021 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http://mpg123.org
	initially written by Michael Hipp

	Dear visitor:
	If you feel you don't understand fully the works of this file, your feeling might be correct.

	Optimize-TODO: put short bands into the band-field without the stride of 3 reals
	Length-optimze: unify long and short band code where it is possible

	The int-vs-pointer situation has to be cleaned up.
*/

#include "mpg123lib_intern.h"
#ifdef USE_NEW_HUFFTABLE
#include "newhuffman.h"
#else
#include "huffman.h"
#endif
#include "getbits.h"
#include "debug.h"



/* define CUT_SFB21 if you want to cut-off the frequency above 16kHz */
#if 0
#define CUT_SFB21
#endif

#include "l3tabs.h"
#include "l3bandgain.h"

#ifdef RUNTIME_TABLES
#include "init_layer3.h"
#endif

/* Decoder state data, living on the stack of do_layer3. */

struct gr_info_s
{
	int scfsi;
	unsigned part2_3_length;
	unsigned big_values;
	unsigned scalefac_compress;
	unsigned block_type;
	unsigned mixed_block_flag;
	unsigned table_select[3];
	/* Making those two signed int as workaround for open64/pathscale/sun compilers, and also for consistency, since they're worked on together with other signed variables. */
	int maxband[3];
	int maxbandl;
	unsigned maxb;
	unsigned region1start;
	unsigned region2start;
	unsigned preflag;
	unsigned scalefac_scale;
	unsigned count1table_select;
#ifdef REAL_IS_FIXED
	const real *full_gain[3];
	const real *pow2gain;
#else
	real *full_gain[3];
	real *pow2gain;
#endif
};

struct III_sideinfo
{
	unsigned main_data_begin;
	unsigned private_bits;
	/* Hm, funny... struct inside struct... */
	struct { struct gr_info_s gr[2]; } ch[2];
};

#ifdef OPT_MMXORSSE
real init_layer3_gainpow2_mmx(mpg123_handle *fr, int i)
{
	if(!fr->p.down_sample) return DOUBLE_TO_REAL(16384.0 * pow((double)2.0,-0.25 * (double) (i+210) ));
	else return DOUBLE_TO_REAL(pow((double)2.0,-0.25 * (double) (i+210)));
}
#endif

real init_layer3_gainpow2(mpg123_handle *fr, int i)
{
	return DOUBLE_TO_REAL_SCALE_LAYER3(pow((double)2.0,-0.25 * (double) (i+210)),i+256);
}

void init_layer3_stuff(mpg123_handle *fr, real (*gainpow2_func)(mpg123_handle *fr, int i))
{
	int i,j;

#ifdef REAL_IS_FIXED
	fr->gainpow2 = gainpow2;
#else
	for(i=-256;i<118+4;i++)
		fr->gainpow2[i+256] = gainpow2_func(fr,i);
#endif

	for(j=0;j<9;j++)
	{
		for(i=0;i<23;i++)
		{
			fr->longLimit[j][i] = (bandInfo[j].longIdx[i] - 1 + 8) / 18 + 1;
			if(fr->longLimit[j][i] > (fr->down_sample_sblimit) )
			fr->longLimit[j][i] = fr->down_sample_sblimit;
		}
		for(i=0;i<14;i++)
		{
			fr->shortLimit[j][i] = (bandInfo[j].shortIdx[i] - 1) / 18 + 1;
			if(fr->shortLimit[j][i] > (fr->down_sample_sblimit) )
			fr->shortLimit[j][i] = fr->down_sample_sblimit;
		}
	}
}

/*
	Observe!
	Now come the actualy decoding routines.
*/

/* read additional side information (for MPEG 1 and MPEG 2) */
static int III_get_side_info(mpg123_handle *fr, struct III_sideinfo *si,int stereo, int ms_stereo,long sfreq,int single)
{
	int ch, gr;
	int powdiff = (single == SINGLE_MIX) ? 4 : 0;

	const int tabs[2][5] = { { 2,9,5,3,4 } , { 1,8,1,2,9 } };
	const int *tab = tabs[fr->hdr.lsf];

	{ /* First ensure we got enough bits available. */
		unsigned int needbits = 0;
		needbits += tab[1]; /* main_data_begin */
		needbits += stereo == 1 ? tab[2] : tab[3]; /* private */
		if(!fr->hdr.lsf)
			needbits += stereo*4; /* scfsi */
		/* For each granule for each channel ... */
		needbits += tab[0]*stereo*(29+tab[4]+1+22+(!fr->hdr.lsf?1:0)+2);
		if(fr->bits_avail < needbits) \
		{
			if(NOQUIET)
				error2( "%u bits for side info needed, only %li available"
				,	needbits, fr->bits_avail );
			return 1;
		}
	}

	si->main_data_begin = getbits(fr, tab[1]);

	if(si->main_data_begin > fr->bitreservoir)
	{
		if(!fr->to_ignore && VERBOSE2) fprintf(stderr, "Note: missing %d bytes in bit reservoir for frame %li\n", (int)(si->main_data_begin - fr->bitreservoir), (long)fr->num);

		/*  overwrite main_data_begin for the really available bit reservoir */
		backbits(fr, tab[1]);
		if(fr->hdr.lsf == 0)
		{
			fr->wordpointer[0] = (unsigned char) (fr->bitreservoir >> 1);
			fr->wordpointer[1] = (unsigned char) ((fr->bitreservoir & 1) << 7);
		}
		else fr->wordpointer[0] = (unsigned char) fr->bitreservoir;

		/* zero "side-info" data for a silence-frame
		without touching audio data used as bit reservoir for following frame */
		memset(fr->wordpointer+2, 0, fr->hdr.ssize-2);

		/* reread the new bit reservoir offset */
		si->main_data_begin = getbits(fr, tab[1]);
	}

	/* Keep track of the available data bytes for the bit reservoir.
	   CRC is included in ssize already. */
	fr->bitreservoir = fr->bitreservoir + fr->hdr.framesize - fr->hdr.ssize;

	/* Limit the reservoir to the max for MPEG 1.0 or 2.x . */
	if(fr->bitreservoir > (unsigned int) (fr->hdr.lsf == 0 ? 511 : 255))
	fr->bitreservoir = (fr->hdr.lsf == 0 ? 511 : 255);

	/* Now back into less commented territory. It's code. It works. */

	if (stereo == 1)
	si->private_bits = getbits(fr, tab[2]);
	else 
	si->private_bits = getbits(fr, tab[3]);

	if(!fr->hdr.lsf) for(ch=0; ch<stereo; ch++)
	{
		si->ch[ch].gr[0].scfsi = -1;
		si->ch[ch].gr[1].scfsi = getbits(fr, 4);
	}

	for (gr=0; gr<tab[0]; gr++)
	for (ch=0; ch<stereo; ch++)
	{
		register struct gr_info_s *gr_info = &(si->ch[ch].gr[gr]);
		unsigned int qss;
		gr_info->part2_3_length = getbits(fr, 12);
		gr_info->big_values = getbits(fr, 9);
		if(gr_info->big_values > 288)
		{
			if(NOQUIET) error("big_values too large!");
			gr_info->big_values = 288;
		}
		qss = getbits_fast(fr, 8);
		gr_info->pow2gain = fr->gainpow2+256 - qss + powdiff;
		if(ms_stereo)
			gr_info->pow2gain += 2;
#ifndef NO_MOREINFO
		if(fr->pinfo)
			fr->pinfo->qss[gr][ch] = qss;
#endif
		gr_info->scalefac_compress = getbits(fr, tab[4]);
		if(gr_info->part2_3_length == 0)
		{
			if(gr_info->scalefac_compress > 0 && VERBOSE2)
				error1( "scalefac_compress should be zero instead of %i"
				,	gr_info->scalefac_compress );
			gr_info->scalefac_compress = 0;
		}

		/* 22 bits for if/else block */
		if(getbits(fr,1))
		{ /* window switch flag  */
			int i;
			gr_info->block_type       = getbits_fast(fr, 2);
			gr_info->mixed_block_flag = get1bit(fr);
			gr_info->table_select[0]  = getbits_fast(fr, 5);
			gr_info->table_select[1]  = getbits_fast(fr, 5);
			/*
				table_select[2] not needed, because there is no region2,
				but to satisfy some verification tools we set it either.
			*/
			gr_info->table_select[2] = 0;
			for(i=0;i<3;i++)
			{
				unsigned int sbg = (getbits_fast(fr, 3)<<3);
				gr_info->full_gain[i] = gr_info->pow2gain + sbg;
#ifndef NO_MOREINFO
				if(fr->pinfo)
					fr->pinfo->sub_gain[gr][ch][i] = sbg / 8;
#endif
			}

			if(gr_info->block_type == 0)
			{
				if(NOQUIET) error("Blocktype == 0 and window-switching == 1 not allowed.");
				return 1;
			}

			/* region_count/start parameters are implicit in this case. */       
			if( (!fr->hdr.lsf || (gr_info->block_type == 2)) && !fr->hdr.mpeg25)
			{
				gr_info->region1start = 36>>1;
				gr_info->region2start = 576>>1;
			}
			else
			{
				if(fr->hdr.mpeg25)
				{ 
					int r0c,r1c;
					if((gr_info->block_type == 2) && (!gr_info->mixed_block_flag) ) r0c = 5;
					else r0c = 7;

					/* r0c+1+r1c+1 == 22, always. */
					r1c = 20 - r0c;
					gr_info->region1start = bandInfo[sfreq].longIdx[r0c+1] >> 1 ;
					gr_info->region2start = bandInfo[sfreq].longIdx[r0c+1+r1c+1] >> 1; 
				}
				else
				{
					gr_info->region1start = 54>>1;
					gr_info->region2start = 576>>1; 
				} 
			}
		}
		else
		{
			int i,r0c,r1c;
			for (i=0; i<3; i++)
			gr_info->table_select[i] = getbits_fast(fr, 5);

			r0c = getbits_fast(fr, 4); /* 0 .. 15 */
			r1c = getbits_fast(fr, 3); /* 0 .. 7 */
			gr_info->region1start = bandInfo[sfreq].longIdx[r0c+1] >> 1 ;

			/* max(r0c+r1c+2) = 15+7+2 = 24 */
			if(r0c+1+r1c+1 > 22) gr_info->region2start = 576>>1;
			else gr_info->region2start = bandInfo[sfreq].longIdx[r0c+1+r1c+1] >> 1;

			gr_info->block_type = 0;
			gr_info->mixed_block_flag = 0;
		}
		if(!fr->hdr.lsf) gr_info->preflag = get1bit(fr);

		gr_info->scalefac_scale = get1bit(fr);
		gr_info->count1table_select = get1bit(fr);
	}
	return 0;
}


/* read scalefactors */
static int III_get_scale_factors_1(mpg123_handle *fr, int *scf,struct gr_info_s *gr_info,int ch,int gr)
{
	const unsigned char slen[2][16] =
	{
		{0, 0, 0, 0, 3, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4},
		{0, 1, 2, 3, 0, 1, 2, 3, 1, 2, 3, 1, 2, 3, 2, 3}
	};
	int numbits;
	int num0 = slen[0][gr_info->scalefac_compress];
	int num1 = slen[1][gr_info->scalefac_compress];

	if(gr_info->block_type == 2)
	{
		int i=18;
		numbits = (num0 + num1) * 18 /* num0 * (17+1?) + num1 * 18 */
		        - (gr_info->mixed_block_flag ? num0 : 0);
		if(fr->bits_avail < numbits)
			return -1;

		if(gr_info->mixed_block_flag)
		{
			for (i=8;i;i--)
			*scf++ = getbits_fast(fr, num0);

			i = 9;
		}

		for(;i;i--) *scf++ = getbits_fast(fr, num0);

		for(i = 18; i; i--) *scf++ = getbits_fast(fr, num1);

		*scf++ = 0; *scf++ = 0; *scf++ = 0; /* short[13][0..2] = 0 */
	}
	else
	{
		int i;
		int scfsi = gr_info->scfsi;

		if(scfsi < 0)
		{ /* scfsi < 0 => granule == 0 */
			numbits = (num0 + num1) * 10 + num0;
			if(fr->bits_avail < numbits)
				return -1;

			for(i=11;i;i--) *scf++ = getbits_fast(fr, num0);

			for(i=10;i;i--) *scf++ = getbits_fast(fr, num1);

			*scf++ = 0;
		}
		else
		{
			numbits = !(scfsi & 0x8) * num0 * 6
			        + !(scfsi & 0x4) * num0 * 5
			        + !(scfsi & 0x2) * num1 * 5
			        + !(scfsi & 0x1) * num1 * 5;
			if(fr->bits_avail < numbits)
				return -1;

			if(!(scfsi & 0x8))
			{
				for (i=0;i<6;i++) *scf++ = getbits_fast(fr, num0);
			}
			else scf += 6; 

			if(!(scfsi & 0x4))
			{
				for (i=0;i<5;i++) *scf++ = getbits_fast(fr, num0);
			}
			else scf += 5;

			if(!(scfsi & 0x2))
			{
				for(i=0;i<5;i++) *scf++ = getbits_fast(fr, num1);
			}
			else scf += 5;

			if(!(scfsi & 0x1))
			{
				for (i=0;i<5;i++) *scf++ = getbits_fast(fr, num1);
			}
			else scf += 5;

			*scf++ = 0;  /* no l[21] in original sources */
		}
	}

	return numbits;
}


static int III_get_scale_factors_2(mpg123_handle *fr, int *scf,struct gr_info_s *gr_info,int i_stereo)
{
	const unsigned char *pnt;
	int i,j,n=0,numbits=0;
	unsigned int slen, slen2;

	const unsigned char stab[3][6][4] =
	{
		{
			{ 6, 5, 5,5 } , { 6, 5, 7,3 } , { 11,10,0,0},
			{ 7, 7, 7,0 } , { 6, 6, 6,3 } , {  8, 8,5,0}
		},
		{
			{ 9, 9, 9,9 } , { 9, 9,12,6 } , { 18,18,0,0},
			{12,12,12,0 } , {12, 9, 9,6 } , { 15,12,9,0}
		},
		{
			{ 6, 9, 9,9 } , { 6, 9,12,6 } , { 15,18,0,0},
			{ 6,15,12,0 } , { 6,12, 9,6 } , {  6,18,9,0}
		}
	}; 

	if(i_stereo) /* i_stereo AND second channel -> do_layer3() checks this */
	slen = i_slen2[gr_info->scalefac_compress>>1];
	else
	slen = n_slen2[gr_info->scalefac_compress];

	gr_info->preflag = (slen>>15) & 0x1;

	n = 0;  
	if( gr_info->block_type == 2 )
	{
		n++;
		if(gr_info->mixed_block_flag) n++;
	}

	pnt = stab[n][(slen>>12)&0x7];

	slen2 = slen;
	for(i=0;i<4;i++)
	{
		int num = slen2 & 0x7;
		slen2 >>= 3;
		if(num)
			numbits += pnt[i] * num;
	}
	if(numbits > gr_info->part2_3_length)
		return -1;

	for(i=0;i<4;i++)
	{
		int num = slen & 0x7;
		slen >>= 3;
		if(num)
		{
			for(j=0;j<(int)(pnt[i]);j++) *scf++ = getbits_fast(fr, num);
		}
		else
		for(j=0;j<(int)(pnt[i]);j++) *scf++ = 0;
	}

	n = (n << 1) + 1;
	for(i=0;i<n;i++) *scf++ = 0;

	return numbits;
}

static unsigned char pretab_choice[2][22] =
{
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,2,2,3,3,3,2,0}
};

/*
	Dequantize samples
	...includes Huffman decoding
*/

/* 24 is enough because tab13 has max. a 19 bit huffvector */
/* The old code played games with shifting signed integers around in not quite */
/* legal ways. Also, it used long where just 32 bits are required. This could */
/* be good or bad on 64 bit architectures ... anyway, making clear that */
/* 32 bits suffice is a benefit. */
#if 0
/* To reconstruct old code, use this: */
#define MASK_STYPE long
#define MASK_UTYPE unsigned long
#define MASK_TYPE MASK_STYPE
#define MSB_MASK (mask < 0)
#else
/* This should be more proper: */
#define MASK_STYPE int32_t
#define MASK_UTYPE uint32_t
#define MASK_TYPE  MASK_UTYPE
#define MSB_MASK ((MASK_UTYPE)mask & (MASK_UTYPE)1<<(sizeof(MASK_TYPE)*8-1))
#endif
#define BITSHIFT ((sizeof(MASK_TYPE)-1)*8)
#define REFRESH_MASK \
	while(num < BITSHIFT) { \
		mask |= ((MASK_UTYPE)getbyte(fr))<<(BITSHIFT-num); \
		num += 8; \
		part2remain -= 8; }
/* Complicated way of checking for msb value. This used to be (mask < 0). */

static int III_dequantize_sample(mpg123_handle *fr, real xr[SBLIMIT][SSLIMIT],int *scf, struct gr_info_s *gr_info,int sfreq,int part2bits)
{
	int shift = 1 + gr_info->scalefac_scale;
	// Pointer cast to make pedantic compilers happy.
	real *xrpnt = (real*)xr;
	// Some compiler freaks out over &xr[SBLIMIT][0], which is the same.
	real *xrpntlimit = (real*)xr+SBLIMIT*SSLIMIT;
	int l[3],l3;
	int part2remain = gr_info->part2_3_length - part2bits;
	const short *me;
#ifdef REAL_IS_FIXED
	int gainpow2_scale_idx = 378;
#endif

	/* Assumption: If there is some part2_3_length at all, there should be
	   enough of it to work with properly. In case of zero length we silently
	   zero things. */
	if(gr_info->part2_3_length > 0)
	{

	/* mhipp tree has this split up a bit... */
	int num=getbitoffset(fr);
	MASK_TYPE mask;
	/* We must split this, because for num==0 the shift is undefined if you do it in one step. */
	mask  = ((MASK_UTYPE) getbits(fr, num))<<BITSHIFT;
	mask <<= 8-num;
	part2remain -= num;

	/* Bitindex is zero now, we are allowed to use getbyte(). */

	{
		int bv       = gr_info->big_values;
		int region1  = gr_info->region1start;
		int region2  = gr_info->region2start;
		l3 = ((576>>1)-bv)>>1;   

		/* we may lose the 'odd' bit here !! check this later again */
		if(bv <= region1)
		{
			l[0] = bv;
			l[1] = 0;
			l[2] = 0;
		}
		else
		{
			l[0] = region1;
			if(bv <= region2)
			{
				l[1] = bv - l[0];
				l[2] = 0;
			}
			else
			{
				l[1] = region2 - l[0];
				l[2] = bv - region2;
			}
		}
	}

#define CHECK_XRPNT if(xrpnt >= xrpntlimit) \
{ \
	if(NOQUIET) \
		error2("attempted xrpnt overflow (%p !< %p)", (void*) xrpnt, (void*) xrpntlimit); \
	return 1; \
}

	if(gr_info->block_type == 2)
	{
		/* decoding with short or mixed mode BandIndex table */
		int i,max[4];
		int step=0,lwin=3,cb=0;
		register real v = 0.0;
		register int mc;
		register const short *m;

		if(gr_info->mixed_block_flag)
		{
			max[3] = -1;
			max[0] = max[1] = max[2] = 2;
			m = map[sfreq][0];
			me = mapend[sfreq][0];
		}
		else
		{
			max[0] = max[1] = max[2] = max[3] = -1;
			/* max[3] not really needed in this case */
			m = map[sfreq][1];
			me = mapend[sfreq][1];
		}

		mc = 0;
		for(i=0;i<2;i++)
		{
			int lp = l[i];
			const struct newhuff *h = ht+gr_info->table_select[i];
			for(;lp;lp--,mc--)
			{
				register MASK_STYPE x,y;
				if( (!mc) )
				{
					mc    = *m++;
//fprintf(stderr, "%i setting xrpnt = xr + %i (%ld)\n", __LINE__, *m, xrpnt-(real*)xr);
					xrpnt = ((real *) xr) + (*m++);
					lwin  = *m++;
					cb    = *m++;
					if(lwin == 3)
					{
#ifdef REAL_IS_FIXED
						gainpow2_scale_idx = (int)(gr_info->pow2gain + (*scf << shift) - fr->gainpow2);
#endif
						v = gr_info->pow2gain[(*scf++) << shift];
						step = 1;
					}
					else
					{
#ifdef REAL_IS_FIXED
						gainpow2_scale_idx = (int)(gr_info->full_gain[lwin] + (*scf << shift) - fr->gainpow2);
#endif
						v = gr_info->full_gain[lwin][(*scf++) << shift];
						step = 3;
					}
				}
				{
					const short *val = h->table;
					REFRESH_MASK;
#ifdef USE_NEW_HUFFTABLE
					while((y=val[(MASK_UTYPE)mask>>(BITSHIFT+4)])<0)
					{
						val -= y;
						num -= 4;
						mask <<= 4;
					}
					num -= (y >> 8);
					mask <<= (y >> 8);
					x = (y >> 4) & 0xf;
					y &= 0xf;
#else
					while((y=*val++)<0)
					{
						if (MSB_MASK) val -= y;

						num--;
						mask <<= 1;
					}
					x = y >> 4;
					y &= 0xf;
#endif
				}
				CHECK_XRPNT;
				if(x == 15 && h->linbits)
				{
					max[lwin] = cb;
					REFRESH_MASK;
					x += ((MASK_UTYPE) mask) >> (BITSHIFT+8-h->linbits);
					num -= h->linbits+1;
					mask <<= h->linbits;
					if(MSB_MASK) *xrpnt = REAL_MUL_SCALE_LAYER3(-ispow[x], v, gainpow2_scale_idx);
					else         *xrpnt = REAL_MUL_SCALE_LAYER3( ispow[x], v, gainpow2_scale_idx);

					mask <<= 1;
				}
				else if(x)
				{
					max[lwin] = cb;
					if(MSB_MASK) *xrpnt = REAL_MUL_SCALE_LAYER3(-ispow[x], v, gainpow2_scale_idx);
					else         *xrpnt = REAL_MUL_SCALE_LAYER3( ispow[x], v, gainpow2_scale_idx);

					num--;
					mask <<= 1;
				}
				else *xrpnt = DOUBLE_TO_REAL(0.0);

				xrpnt += step;
				CHECK_XRPNT;
				if(y == 15 && h->linbits)
				{
					max[lwin] = cb;
					REFRESH_MASK;
					y += ((MASK_UTYPE) mask) >> (BITSHIFT+8-h->linbits);
					num -= h->linbits+1;
					mask <<= h->linbits;
					if(MSB_MASK) *xrpnt = REAL_MUL_SCALE_LAYER3(-ispow[y], v, gainpow2_scale_idx);
					else         *xrpnt = REAL_MUL_SCALE_LAYER3( ispow[y], v, gainpow2_scale_idx);

					mask <<= 1;
				}
				else if(y)
				{
					max[lwin] = cb;
					if(MSB_MASK) *xrpnt = REAL_MUL_SCALE_LAYER3(-ispow[y], v, gainpow2_scale_idx);
					else         *xrpnt = REAL_MUL_SCALE_LAYER3( ispow[y], v, gainpow2_scale_idx);

					num--;
					mask <<= 1;
				}
				else *xrpnt = DOUBLE_TO_REAL(0.0);

				xrpnt += step;
			}
		}

		for(;l3 && (part2remain+num > 0);l3--)
		{
			const struct newhuff* h;
			const short* val;
			register short a;

			h = htc+gr_info->count1table_select;
			val = h->table;

			REFRESH_MASK;
			while((a=*val++)<0)
			{
				if(MSB_MASK) val -= a;

				num--;
				mask <<= 1;
			}
			if(part2remain+num <= 0)
			{
				num -= part2remain+num;
				break;
			}

			for(i=0;i<4;i++)
			{
				if(!(i & 1))
				{
					if(!mc)
					{
						mc = *m++;
//fprintf(stderr, "%i setting xrpnt = xr + %i (%ld)\n", __LINE__, *m, xrpnt-(real*)xr);
						xrpnt = ((real *) xr) + (*m++);
						lwin = *m++;
						cb = *m++;
						if(lwin == 3)
						{
#ifdef REAL_IS_FIXED
							gainpow2_scale_idx = (int)(gr_info->pow2gain + (*scf << shift) - fr->gainpow2);
#endif
							v = gr_info->pow2gain[(*scf++) << shift];
							step = 1;
						}
						else
						{
#ifdef REAL_IS_FIXED
							gainpow2_scale_idx = (int)(gr_info->full_gain[lwin] + (*scf << shift) - fr->gainpow2);
#endif
							v = gr_info->full_gain[lwin][(*scf++) << shift];
							step = 3;
						}
					}
					mc--;
				}
				CHECK_XRPNT;
				if( (a & (0x8>>i)) )
				{
					max[lwin] = cb;
					if(part2remain+num <= 0)
					break;

					if(MSB_MASK) *xrpnt = -REAL_SCALE_LAYER3(v, gainpow2_scale_idx);
					else         *xrpnt =  REAL_SCALE_LAYER3(v, gainpow2_scale_idx);

					num--;
					mask <<= 1;
				}
				else *xrpnt = DOUBLE_TO_REAL(0.0);

				xrpnt += step;
			}
		}

		if(lwin < 3)
		{ /* short band? */
			while(1)
			{
				for(;mc > 0;mc--)
				{
					CHECK_XRPNT;
					*xrpnt = DOUBLE_TO_REAL(0.0); xrpnt += 3; /* short band -> step=3 */
					*xrpnt = DOUBLE_TO_REAL(0.0); xrpnt += 3;
				}
				if(m >= me)
				break;

				mc    = *m++;
				xrpnt = ((real *) xr) + *m++;
				if(*m++ == 0)
				break; /* optimize: field will be set to zero at the end of the function */

				m++; /* cb */
			}
		}

		gr_info->maxband[0] = max[0]+1;
		gr_info->maxband[1] = max[1]+1;
		gr_info->maxband[2] = max[2]+1;
		gr_info->maxbandl   = max[3]+1;

		{
			int rmax = max[0] > max[1] ? max[0] : max[1];
			rmax = (rmax > max[2] ? rmax : max[2]) + 1;
			gr_info->maxb = rmax ? fr->shortLimit[sfreq][rmax] : fr->longLimit[sfreq][max[3]+1];
		}

	}
	else
	{
		/* decoding with 'long' BandIndex table (block_type != 2) */
		const unsigned char *pretab = pretab_choice[gr_info->preflag];
		int i,max = -1;
		int cb = 0;
		const short *m = map[sfreq][2];
		register real v = 0.0;
		int mc = 0;

		/* long hash table values */
		for(i=0;i<3;i++)
		{
			int lp = l[i];
			const struct newhuff *h = ht+gr_info->table_select[i];

			for(;lp;lp--,mc--)
			{
				MASK_STYPE x,y;
				if(!mc)
				{
					mc = *m++;
					cb = *m++;
#ifdef CUT_SFB21
					if(cb == 21)
						v = 0.0;
					else
#endif
					{
#ifdef REAL_IS_FIXED
						gainpow2_scale_idx = (int)(gr_info->pow2gain + (*scf << shift) - fr->gainpow2);
#endif
						v = gr_info->pow2gain[(*(scf++) + (*pretab++)) << shift];
					}
				}
				{
					const short *val = h->table;
					REFRESH_MASK;
#ifdef USE_NEW_HUFFTABLE
					while((y=val[(MASK_UTYPE)mask>>(BITSHIFT+4)])<0)
					{
						val -= y;
						num -= 4;
						mask <<= 4;
					}
					num -= (y >> 8);
					mask <<= (y >> 8);
					x = (y >> 4) & 0xf;
					y &= 0xf;
#else
					while((y=*val++)<0)
					{
						if (MSB_MASK) val -= y;

						num--;
						mask <<= 1;
					}
					x = y >> 4;
					y &= 0xf;
#endif
				}

				CHECK_XRPNT;
				if(x == 15 && h->linbits)
				{
					max = cb;
					REFRESH_MASK;
					x += ((MASK_UTYPE) mask) >> (BITSHIFT+8-h->linbits);
					num -= h->linbits+1;
					mask <<= h->linbits;
					if(MSB_MASK) *xrpnt++ = REAL_MUL_SCALE_LAYER3(-ispow[x], v, gainpow2_scale_idx);
					else         *xrpnt++ = REAL_MUL_SCALE_LAYER3( ispow[x], v, gainpow2_scale_idx);

					mask <<= 1;
				}
				else if(x)
				{
					max = cb;
					if(MSB_MASK) *xrpnt++ = REAL_MUL_SCALE_LAYER3(-ispow[x], v, gainpow2_scale_idx);
					else         *xrpnt++ = REAL_MUL_SCALE_LAYER3( ispow[x], v, gainpow2_scale_idx);
					num--;

					mask <<= 1;
				}
				else *xrpnt++ = DOUBLE_TO_REAL(0.0);

				CHECK_XRPNT;
				if(y == 15 && h->linbits)
				{
					max = cb;
					REFRESH_MASK;
					y += ((MASK_UTYPE) mask) >> (BITSHIFT+8-h->linbits);
					num -= h->linbits+1;
					mask <<= h->linbits;
					if(MSB_MASK) *xrpnt++ = REAL_MUL_SCALE_LAYER3(-ispow[y], v, gainpow2_scale_idx);
					else         *xrpnt++ = REAL_MUL_SCALE_LAYER3( ispow[y], v, gainpow2_scale_idx);

					mask <<= 1;
				}
				else if(y)
				{
					max = cb;
					if(MSB_MASK) *xrpnt++ = REAL_MUL_SCALE_LAYER3(-ispow[y], v, gainpow2_scale_idx);
					else         *xrpnt++ = REAL_MUL_SCALE_LAYER3( ispow[y], v, gainpow2_scale_idx);

					num--;
					mask <<= 1;
				}
				else *xrpnt++ = DOUBLE_TO_REAL(0.0);
			}
		}

		/* short (count1table) values */
		for(;l3 && (part2remain+num > 0);l3--)
		{
			const struct newhuff *h = htc+gr_info->count1table_select;
			const short *val = h->table;
			register short a;

			REFRESH_MASK;
			while((a=*val++)<0)
			{
				if (MSB_MASK) val -= a;

				num--;
				mask <<= 1;
			}
			if(part2remain+num <= 0)
			{
				num -= part2remain+num;
				break;
			}

			for(i=0;i<4;i++)
			{
				if(!(i & 1))
				{
					if(!mc)
					{
						mc = *m++;
						cb = *m++;
#ifdef CUT_SFB21
						if(cb == 21)
							v = 0.0;
						else
#endif
						{
#ifdef REAL_IS_FIXED
							gainpow2_scale_idx = (int)(gr_info->pow2gain + (*scf << shift) - fr->gainpow2);
#endif
							v = gr_info->pow2gain[((*scf++) + (*pretab++)) << shift];
						}
					}
					mc--;
				}
				CHECK_XRPNT;
				if( (a & (0x8>>i)) )
				{
					max = cb;
					if(part2remain+num <= 0)
					break;

					if(MSB_MASK) *xrpnt++ = -REAL_SCALE_LAYER3(v, gainpow2_scale_idx);
					else         *xrpnt++ =  REAL_SCALE_LAYER3(v, gainpow2_scale_idx);

					num--;
					mask <<= 1;
				}
				else *xrpnt++ = DOUBLE_TO_REAL(0.0);
			}
		}

		gr_info->maxbandl = max+1;
		gr_info->maxb = fr->longLimit[sfreq][gr_info->maxbandl];
	}

	part2remain += num;
	backbits(fr, num);
	num = 0;

	}
	else
	{
		part2remain = 0;
		/* Not entirely sure what good values are, must be > 0. */
		gr_info->maxband[0] =
		gr_info->maxband[1] =
		gr_info->maxband[2] =
		gr_info->maxbandl   = 1; /* sfb=maxband[lwin]*3 + lwin - mixed_block_flag must be >= 0 */
		gr_info->maxb       = 1;
	}

	while(xrpnt < xrpntlimit)
	*xrpnt++ = DOUBLE_TO_REAL(0.0);

	while( part2remain > 16 )
	{
		skipbits(fr, 16); /* Dismiss stuffing Bits */
		part2remain -= 16;
	}
	if(part2remain > 0) skipbits(fr, part2remain);
	else if(part2remain < 0)
	{
		if(VERBOSE2)
			error1("Can't rewind stream by %d bits!",-part2remain);
		return 1; /* -> error */
	}
	return 0;
}


/* calculate real channel values for Joint-I-Stereo-mode */
static void III_i_stereo(real xr_buf[2][SBLIMIT][SSLIMIT],int *scalefac, struct gr_info_s *gr_info,int sfreq,int ms_stereo,int lsf)
{
	real (*xr)[SBLIMIT*SSLIMIT] = (real (*)[SBLIMIT*SSLIMIT] ) xr_buf;
	const struct bandInfoStruct *bi = &bandInfo[sfreq];

	const real *tab1,*tab2;

#if 1
	int tab;
/* TODO: optimize as static */
	const real *tabs[3][2][2] =
	{ 
		{ { tan1_1,tan2_1 }       , { tan1_2,tan2_2 } },
		{ { pow1_1[0],pow2_1[0] } , { pow1_2[0],pow2_2[0] } },
		{ { pow1_1[1],pow2_1[1] } , { pow1_2[1],pow2_2[1] } }
	};

	tab = lsf + (gr_info->scalefac_compress & lsf);
	tab1 = tabs[tab][ms_stereo][0];
	tab2 = tabs[tab][ms_stereo][1];
#else
	if(lsf)
	{
		int p = gr_info->scalefac_compress & 0x1;
		if(ms_stereo)
		{
			tab1 = pow1_2[p];
			tab2 = pow2_2[p];
		}
		else
		{
			tab1 = pow1_1[p];
			tab2 = pow2_1[p];
		}
	}
	else
	{
		if(ms_stereo)
		{
			tab1 = tan1_2;
			tab2 = tan2_2;
		}
		else
		{
			tab1 = tan1_1;
			tab2 = tan2_1;
		}
	}
#endif

	if(gr_info->block_type == 2)
	{
		int lwin,do_l = 0;
		if( gr_info->mixed_block_flag ) do_l = 1;

		for(lwin=0;lwin<3;lwin++)
		{ /* process each window */
			/* get first band with zero values */
			int is_p,sb,idx,sfb = gr_info->maxband[lwin];  /* sfb is minimal 3 for mixed mode */
			if(sfb > 3) do_l = 0;

			for(;sfb<12;sfb++)
			{
				is_p = scalefac[sfb*3+lwin-gr_info->mixed_block_flag]; /* scale: 0-15 */ 
				if(is_p != 7)
				{
					real t1,t2;
					sb  = bi->shortDiff[sfb];
					idx = bi->shortIdx[sfb] + lwin;
					t1  = tab1[is_p]; t2 = tab2[is_p];
					for (; sb > 0; sb--,idx+=3)
					{
						real v = xr[0][idx];
						xr[0][idx] = REAL_MUL_15(v, t1);
						xr[1][idx] = REAL_MUL_15(v, t2);
					}
				}
			}

#if 1
/* in the original: copy 10 to 11 , here: copy 11 to 12 
maybe still wrong??? (copy 12 to 13?) */
			is_p = scalefac[11*3+lwin-gr_info->mixed_block_flag]; /* scale: 0-15 */
			sb   = bi->shortDiff[12];
			idx  = bi->shortIdx[12] + lwin;
#else
			is_p = scalefac[10*3+lwin-gr_info->mixed_block_flag]; /* scale: 0-15 */
			sb   = bi->shortDiff[11];
			idx  = bi->shortIdx[11] + lwin;
#endif
			if(is_p != 7)
			{
				real t1,t2;
				t1 = tab1[is_p]; t2 = tab2[is_p];
				for( ; sb > 0; sb--,idx+=3 )
				{  
					real v = xr[0][idx];
					xr[0][idx] = REAL_MUL_15(v, t1);
					xr[1][idx] = REAL_MUL_15(v, t2);
				}
			}
		} /* end for(lwin; .. ; . ) */

		/* also check l-part, if ALL bands in the three windows are 'empty' and mode = mixed_mode */
		if(do_l)
		{
			int sfb = gr_info->maxbandl;
			int idx;
			if(sfb > 21) return; /* similarity fix related to CVE-2006-1655 */

			idx = bi->longIdx[sfb];
			for( ; sfb<8; sfb++ )
			{
				int sb = bi->longDiff[sfb];
				int is_p = scalefac[sfb]; /* scale: 0-15 */
				if(is_p != 7)
				{
					real t1,t2;
					t1 = tab1[is_p]; t2 = tab2[is_p];
					for( ; sb > 0; sb--,idx++)
					{
						real v = xr[0][idx];
						xr[0][idx] = REAL_MUL_15(v, t1);
						xr[1][idx] = REAL_MUL_15(v, t2);
					}
				}
				else idx += sb;
			}
		}     
	} 
	else
	{ /* ((gr_info->block_type != 2)) */
		int sfb = gr_info->maxbandl;
		int is_p,idx;
		if(sfb > 21) return; /* tightened fix for CVE-2006-1655 */

		idx = bi->longIdx[sfb];
		for ( ; sfb<21; sfb++)
		{
			int sb = bi->longDiff[sfb];
			is_p = scalefac[sfb]; /* scale: 0-15 */
			if(is_p != 7)
			{
				real t1,t2;
				t1 = tab1[is_p]; t2 = tab2[is_p];
				for( ; sb > 0; sb--,idx++)
				{
					 real v = xr[0][idx];
					 xr[0][idx] = REAL_MUL_15(v, t1);
					 xr[1][idx] = REAL_MUL_15(v, t2);
				}
			}
			else idx += sb;
		}

		is_p = scalefac[20];
		if(is_p != 7)
		{  /* copy l-band 20 to l-band 21 */
			int sb;
			real t1 = tab1[is_p],t2 = tab2[is_p]; 

			for( sb = bi->longDiff[21]; sb > 0; sb--,idx++ )
			{
				real v = xr[0][idx];
				xr[0][idx] = REAL_MUL_15(v, t1);
				xr[1][idx] = REAL_MUL_15(v, t2);
			}
		}
	}
}


static void III_antialias(real xr[SBLIMIT][SSLIMIT],struct gr_info_s *gr_info)
{
	int sblim;

	if(gr_info->block_type == 2)
	{
			if(!gr_info->mixed_block_flag) return;

			sblim = 1; 
	}
	else sblim = gr_info->maxb-1;

	/* 31 alias-reduction operations between each pair of sub-bands */
	/* with 8 butterflies between each pair                         */

	{
		int sb;
		real *xr1=(real *) xr[1];

		for(sb=sblim; sb; sb--,xr1+=10)
		{
			int ss;
			const real *cs=aa_cs,*ca=aa_ca;
			real *xr2 = xr1;

			for(ss=7;ss>=0;ss--)
			{ /* upper and lower butterfly inputs */
				register real bu = *--xr2,bd = *xr1;
				*xr2   = REAL_MUL(bu, *cs) - REAL_MUL(bd, *ca);
				*xr1++ = REAL_MUL(bd, *cs++) + REAL_MUL(bu, *ca++);
			}
		}
	}
}

/* 
	This is an optimized DCT from Jeff Tsay's maplay 1.2+ package.
	Saved one multiplication by doing the 'twiddle factor' stuff
	together with the window mul. (MH)

	This uses Byeong Gi Lee's Fast Cosine Transform algorithm, but the
	9 point IDCT needs to be reduced further. Unfortunately, I don't
	know how to do that, because 9 is not an even number. - Jeff.

	Original Message:

	9 Point Inverse Discrete Cosine Transform

	This piece of code is Copyright 1997 Mikko Tommila and is freely usable
	by anybody. The algorithm itself is of course in the public domain.

	Again derived heuristically from the 9-point WFTA.

	The algorithm is optimized (?) for speed, not for small rounding errors or
	good readability.

	36 additions, 11 multiplications

	Again this is very likely sub-optimal.

	The code is optimized to use a minimum number of temporary variables,
	so it should compile quite well even on 8-register Intel x86 processors.
	This makes the code quite obfuscated and very difficult to understand.

	References:
	[1] S. Winograd: "On Computing the Discrete Fourier Transform",
	    Mathematics of Computation, Volume 32, Number 141, January 1978,
	    Pages 175-199
*/

/* Calculation of the inverse MDCT
   used to be static without 3dnow - does that really matter? */
void dct36(real *inbuf,real *o1,real *o2,const real *wintab,real *tsbuf)
{
	real tmp[18];

	{
		register real *in = inbuf;

		in[17]+=in[16]; in[16]+=in[15]; in[15]+=in[14];
		in[14]+=in[13]; in[13]+=in[12]; in[12]+=in[11];
		in[11]+=in[10]; in[10]+=in[9];  in[9] +=in[8];
		in[8] +=in[7];  in[7] +=in[6];  in[6] +=in[5];
		in[5] +=in[4];  in[4] +=in[3];  in[3] +=in[2];
		in[2] +=in[1];  in[1] +=in[0];

		in[17]+=in[15]; in[15]+=in[13]; in[13]+=in[11]; in[11]+=in[9];
		in[9] +=in[7];  in[7] +=in[5];  in[5] +=in[3];  in[3] +=in[1];

#if 1
		{
			real t3;
			{
				real t0, t1, t2;

				t0 = REAL_MUL(COS6_2, (in[8] + in[16] - in[4]));
				t1 = REAL_MUL(COS6_2, in[12]);

				t3 = in[0];
				t2 = t3 - t1 - t1;
				tmp[1] = tmp[7] = t2 - t0;
				tmp[4]          = t2 + t0 + t0;
				t3 += t1;

				t2 = REAL_MUL(COS6_1, (in[10] + in[14] - in[2]));
				tmp[1] -= t2;
				tmp[7] += t2;
			}
			{
				real t0, t1, t2;

				t0 = REAL_MUL(cos9[0], (in[4] + in[8] ));
				t1 = REAL_MUL(cos9[1], (in[8] - in[16]));
				t2 = REAL_MUL(cos9[2], (in[4] + in[16]));

				tmp[2] = tmp[6] = t3 - t0      - t2;
				tmp[0] = tmp[8] = t3 + t0 + t1;
				tmp[3] = tmp[5] = t3      - t1 + t2;
			}
		}
		{
			real t1, t2, t3;

			t1 = REAL_MUL(cos18[0], (in[2]  + in[10]));
			t2 = REAL_MUL(cos18[1], (in[10] - in[14]));
			t3 = REAL_MUL(COS6_1,    in[6]);

			{
				real t0 = t1 + t2 + t3;
				tmp[0] += t0;
				tmp[8] -= t0;
			}

			t2 -= t3;
			t1 -= t3;

			t3 = REAL_MUL(cos18[2], (in[2] + in[14]));

			t1 += t3;
			tmp[3] += t1;
			tmp[5] -= t1;

			t2 -= t3;
			tmp[2] += t2;
			tmp[6] -= t2;
		}

#else
		{
			real t0, t1, t2, t3, t4, t5, t6, t7;

			t1 = REAL_MUL(COS6_2, in[12]);
			t2 = REAL_MUL(COS6_2, (in[8] + in[16] - in[4]));

			t3 = in[0] + t1;
			t4 = in[0] - t1 - t1;
			t5     = t4 - t2;
			tmp[4] = t4 + t2 + t2;

			t0 = REAL_MUL(cos9[0], (in[4] + in[8]));
			t1 = REAL_MUL(cos9[1], (in[8] - in[16]));

			t2 = REAL_MUL(cos9[2], (in[4] + in[16]));

			t6 = t3 - t0 - t2;
			t0 += t3 + t1;
			t3 += t2 - t1;

			t2 = REAL_MUL(cos18[0], (in[2]  + in[10]));
			t4 = REAL_MUL(cos18[1], (in[10] - in[14]));
			t7 = REAL_MUL(COS6_1, in[6]);

			t1 = t2 + t4 + t7;
			tmp[0] = t0 + t1;
			tmp[8] = t0 - t1;
			t1 = REAL_MUL(cos18[2], (in[2] + in[14]));
			t2 += t1 - t7;

			tmp[3] = t3 + t2;
			t0 = REAL_MUL(COS6_1, (in[10] + in[14] - in[2]));
			tmp[5] = t3 - t2;

			t4 -= t1 + t7;

			tmp[1] = t5 - t0;
			tmp[7] = t5 + t0;
			tmp[2] = t6 + t4;
			tmp[6] = t6 - t4;
		}
#endif

		{
			real t0, t1, t2, t3, t4, t5, t6, t7;

			t1 = REAL_MUL(COS6_2, in[13]);
			t2 = REAL_MUL(COS6_2, (in[9] + in[17] - in[5]));

			t3 = in[1] + t1;
			t4 = in[1] - t1 - t1;
			t5 = t4 - t2;

			t0 = REAL_MUL(cos9[0], (in[5] + in[9]));
			t1 = REAL_MUL(cos9[1], (in[9] - in[17]));

			tmp[13] = REAL_MUL((t4 + t2 + t2), tfcos36[17-13]);
			t2 = REAL_MUL(cos9[2], (in[5] + in[17]));

			t6 = t3 - t0 - t2;
			t0 += t3 + t1;
			t3 += t2 - t1;

			t2 = REAL_MUL(cos18[0], (in[3]  + in[11]));
			t4 = REAL_MUL(cos18[1], (in[11] - in[15]));
			t7 = REAL_MUL(COS6_1, in[7]);

			t1 = t2 + t4 + t7;
			tmp[17] = REAL_MUL((t0 + t1), tfcos36[17-17]);
			tmp[9]  = REAL_MUL((t0 - t1), tfcos36[17-9]);
			t1 = REAL_MUL(cos18[2], (in[3] + in[15]));
			t2 += t1 - t7;

			tmp[14] = REAL_MUL((t3 + t2), tfcos36[17-14]);
			t0 = REAL_MUL(COS6_1, (in[11] + in[15] - in[3]));
			tmp[12] = REAL_MUL((t3 - t2), tfcos36[17-12]);

			t4 -= t1 + t7;

			tmp[16] = REAL_MUL((t5 - t0), tfcos36[17-16]);
			tmp[10] = REAL_MUL((t5 + t0), tfcos36[17-10]);
			tmp[15] = REAL_MUL((t6 + t4), tfcos36[17-15]);
			tmp[11] = REAL_MUL((t6 - t4), tfcos36[17-11]);
		}

#define MACRO(v) { \
		real tmpval; \
		tmpval = tmp[(v)] + tmp[17-(v)]; \
		out2[9+(v)] = REAL_MUL(tmpval, w[27+(v)]); \
		out2[8-(v)] = REAL_MUL(tmpval, w[26-(v)]); \
		tmpval = tmp[(v)] - tmp[17-(v)]; \
		ts[SBLIMIT*(8-(v))] = out1[8-(v)] + REAL_MUL(tmpval, w[8-(v)]); \
		ts[SBLIMIT*(9+(v))] = out1[9+(v)] + REAL_MUL(tmpval, w[9+(v)]); }

		{
			register real *out2 = o2;
			register const real *w = wintab;
			register real *out1 = o1;
			register real *ts = tsbuf;

			MACRO(0);
			MACRO(1);
			MACRO(2);
			MACRO(3);
			MACRO(4);
			MACRO(5);
			MACRO(6);
			MACRO(7);
			MACRO(8);
		}

	}
}


/* new DCT12 */
static void dct12(real *in,real *rawout1,real *rawout2,register const real *wi,register real *ts)
{
#define DCT12_PART1 \
	in5 = in[5*3];  \
	in5 += (in4 = in[4*3]); \
	in4 += (in3 = in[3*3]); \
	in3 += (in2 = in[2*3]); \
	in2 += (in1 = in[1*3]); \
	in1 += (in0 = in[0*3]); \
	\
	in5 += in3; in3 += in1; \
	\
	in2 = REAL_MUL(in2, COS6_1); \
	in3 = REAL_MUL(in3, COS6_1);

#define DCT12_PART2 \
	in0 += REAL_MUL(in4, COS6_2); \
	\
	in4 = in0 + in2; \
	in0 -= in2;      \
	\
	in1 += REAL_MUL(in5, COS6_2); \
	\
	in5 = REAL_MUL((in1 + in3), tfcos12[0]); \
	in1 = REAL_MUL((in1 - in3), tfcos12[2]); \
	\
	in3 = in4 + in5; \
	in4 -= in5;      \
	\
	in2 = in0 + in1; \
	in0 -= in1;

	{
		real in0,in1,in2,in3,in4,in5;
		register real *out1 = rawout1;
		ts[SBLIMIT*0] = out1[0]; ts[SBLIMIT*1] = out1[1]; ts[SBLIMIT*2] = out1[2];
		ts[SBLIMIT*3] = out1[3]; ts[SBLIMIT*4] = out1[4]; ts[SBLIMIT*5] = out1[5];
 
		DCT12_PART1

		{
			real tmp0,tmp1 = (in0 - in4);
			{
				real tmp2 = REAL_MUL((in1 - in5), tfcos12[1]);
				tmp0 = tmp1 + tmp2;
				tmp1 -= tmp2;
			}
			ts[(17-1)*SBLIMIT] = out1[17-1] + REAL_MUL(tmp0, wi[11-1]);
			ts[(12+1)*SBLIMIT] = out1[12+1] + REAL_MUL(tmp0, wi[6+1]);
			ts[(6 +1)*SBLIMIT] = out1[6 +1] + REAL_MUL(tmp1, wi[1]);
			ts[(11-1)*SBLIMIT] = out1[11-1] + REAL_MUL(tmp1, wi[5-1]);
		}

		DCT12_PART2

		ts[(17-0)*SBLIMIT] = out1[17-0] + REAL_MUL(in2, wi[11-0]);
		ts[(12+0)*SBLIMIT] = out1[12+0] + REAL_MUL(in2, wi[6+0]);
		ts[(12+2)*SBLIMIT] = out1[12+2] + REAL_MUL(in3, wi[6+2]);
		ts[(17-2)*SBLIMIT] = out1[17-2] + REAL_MUL(in3, wi[11-2]);

		ts[(6 +0)*SBLIMIT]  = out1[6+0] + REAL_MUL(in0, wi[0]);
		ts[(11-0)*SBLIMIT] = out1[11-0] + REAL_MUL(in0, wi[5-0]);
		ts[(6 +2)*SBLIMIT]  = out1[6+2] + REAL_MUL(in4, wi[2]);
		ts[(11-2)*SBLIMIT] = out1[11-2] + REAL_MUL(in4, wi[5-2]);
	}

	in++;

	{
		real in0,in1,in2,in3,in4,in5;
		register real *out2 = rawout2;
 
		DCT12_PART1

		{
			real tmp0,tmp1 = (in0 - in4);
			{
				real tmp2 = REAL_MUL((in1 - in5), tfcos12[1]);
				tmp0 = tmp1 + tmp2;
				tmp1 -= tmp2;
			}
			out2[5-1] = REAL_MUL(tmp0, wi[11-1]);
			out2[0+1] = REAL_MUL(tmp0, wi[6+1]);
			ts[(12+1)*SBLIMIT] += REAL_MUL(tmp1, wi[1]);
			ts[(17-1)*SBLIMIT] += REAL_MUL(tmp1, wi[5-1]);
		}

		DCT12_PART2

		out2[5-0] = REAL_MUL(in2, wi[11-0]);
		out2[0+0] = REAL_MUL(in2, wi[6+0]);
		out2[0+2] = REAL_MUL(in3, wi[6+2]);
		out2[5-2] = REAL_MUL(in3, wi[11-2]);

		ts[(12+0)*SBLIMIT] += REAL_MUL(in0, wi[0]);
		ts[(17-0)*SBLIMIT] += REAL_MUL(in0, wi[5-0]);
		ts[(12+2)*SBLIMIT] += REAL_MUL(in4, wi[2]);
		ts[(17-2)*SBLIMIT] += REAL_MUL(in4, wi[5-2]);
	}

	in++; 

	{
		real in0,in1,in2,in3,in4,in5;
		register real *out2 = rawout2;
		out2[12]=out2[13]=out2[14]=out2[15]=out2[16]=out2[17]=0.0;

		DCT12_PART1

		{
			real tmp0,tmp1 = (in0 - in4);
			{
				real tmp2 = REAL_MUL((in1 - in5), tfcos12[1]);
				tmp0 = tmp1 + tmp2;
				tmp1 -= tmp2;
			}
			out2[11-1] = REAL_MUL(tmp0, wi[11-1]);
			out2[6 +1] = REAL_MUL(tmp0, wi[6+1]);
			out2[0+1] += REAL_MUL(tmp1, wi[1]);
			out2[5-1] += REAL_MUL(tmp1, wi[5-1]);
		}

		DCT12_PART2

		out2[11-0] = REAL_MUL(in2, wi[11-0]);
		out2[6 +0] = REAL_MUL(in2, wi[6+0]);
		out2[6 +2] = REAL_MUL(in3, wi[6+2]);
		out2[11-2] = REAL_MUL(in3, wi[11-2]);

		out2[0+0] += REAL_MUL(in0, wi[0]);
		out2[5-0] += REAL_MUL(in0, wi[5-0]);
		out2[0+2] += REAL_MUL(in4, wi[2]);
		out2[5-2] += REAL_MUL(in4, wi[5-2]);
	}
}


static void III_hybrid(real fsIn[SBLIMIT][SSLIMIT], real tsOut[SSLIMIT][SBLIMIT], int ch,struct gr_info_s *gr_info, mpg123_handle *fr)
{
	real (*block)[2][SBLIMIT*SSLIMIT] = fr->hybrid_block;
	int *blc = fr->hybrid_blc;

	real *tspnt = (real *) tsOut;
	real *rawout1,*rawout2;
	int bt = 0;
	size_t sb = 0;

	{
		int b = blc[ch];
		rawout1=block[b][ch];
		b=-b+1;
		rawout2=block[b][ch];
		blc[ch] = b;
	}
  
	if(gr_info->mixed_block_flag)
	{
		sb = 2;
		opt_dct36(fr)(fsIn[0],rawout1,rawout2,win[0],tspnt);
		opt_dct36(fr)(fsIn[1],rawout1+18,rawout2+18,win1[0],tspnt+1);
		rawout1 += 36; rawout2 += 36; tspnt += 2;
	}
 
	bt = gr_info->block_type;
	if(bt == 2)
	{
		for(; sb<gr_info->maxb; sb+=2,tspnt+=2,rawout1+=36,rawout2+=36)
		{
			dct12(fsIn[sb]  ,rawout1   ,rawout2   ,win[2] ,tspnt);
			dct12(fsIn[sb+1],rawout1+18,rawout2+18,win1[2],tspnt+1);
		}
	}
	else
	{
		for(; sb<gr_info->maxb; sb+=2,tspnt+=2,rawout1+=36,rawout2+=36)
		{
			opt_dct36(fr)(fsIn[sb],rawout1,rawout2,win[bt],tspnt);
			opt_dct36(fr)(fsIn[sb+1],rawout1+18,rawout2+18,win1[bt],tspnt+1);
		}
	}

	for(;sb<SBLIMIT;sb++,tspnt++)
	{
		int i;
		for(i=0;i<SSLIMIT;i++)
		{
			tspnt[i*SBLIMIT] = *rawout1++;
			*rawout2++ = DOUBLE_TO_REAL(0.0);
		}
	}
}

#ifndef NO_MOREINFO
static void fill_pinfo_side(mpg123_handle *fr, struct III_sideinfo *si, int gr, int stereo1)
{
	int   i, sb;
	float ifqstep; /* Why not double? */
	int ch, ss;;

	for(ch = 0; ch < stereo1; ++ch)
	{
		struct gr_info_s *gr_infos = &(si->ch[ch].gr[gr]);
		fr->pinfo->big_values[gr][ch] = gr_infos->big_values;
		fr->pinfo->scalefac_scale[gr][ch] = gr_infos->scalefac_scale;
		fr->pinfo->mixed[gr][ch] = gr_infos->mixed_block_flag;
		fr->pinfo->blocktype[gr][ch] = gr_infos->block_type;
		fr->pinfo->mainbits[gr][ch] = gr_infos->part2_3_length;
		fr->pinfo->preflag[gr][ch] = gr_infos->preflag;
		if(gr == 1)
			fr->pinfo->scfsi[ch] = gr_infos->scfsi;
	}

	for(ch = 0; ch < stereo1; ++ch)
	{
		struct gr_info_s *gr_infos = &(si->ch[ch].gr[gr]);
		ifqstep = (fr->pinfo->scalefac_scale[gr][ch] == 0) ? .5 : 1.0;
		if(2 == gr_infos->block_type)
		{
			for(i = 0; i < 3; ++i)
			{
				for(sb = 0; sb < 12; ++sb)
				{
					int	  j = 3 * sb + i;
					/*
						 is_p = scalefac[sfb*3+lwin-gr_infos->mixed_block_flag]; 
					*/
					/* scalefac was copied into pinfo->sfb_s[] before */
					fr->pinfo->sfb_s[gr][ch][j] = -ifqstep *
						fr->pinfo->sfb_s[gr][ch][j - gr_infos->mixed_block_flag];
					fr->pinfo->sfb_s[gr][ch][j] -= 2 *
						(fr->pinfo->sub_gain[gr][ch][i]);
				}
				fr->pinfo->sfb_s[gr][ch][3 * sb + i] =
					-2 * (fr->pinfo->sub_gain[gr][ch][i]);
			}
		} else
		{
			for(sb = 0; sb < 21; ++sb)
			{
				/* scalefac was copied into pinfo->sfb[] before */
				fr->pinfo->sfb[gr][ch][sb] = fr->pinfo->sfb_s[gr][ch][sb];
				if (gr_infos->preflag)
					fr->pinfo->sfb[gr][ch][sb] += pretab_choice[1][sb];
				fr->pinfo->sfb[gr][ch][sb] *= -ifqstep;
			}
			fr->pinfo->sfb[gr][ch][21] = 0;
		}
	}


	for(ch = 0; ch < stereo1; ++ch)
	{
		int j = 0;
		for(sb = 0; sb < SBLIMIT; ++sb)
			for (ss = 0; ss < SSLIMIT; ++ss, ++j)
				fr->pinfo->xr[gr][ch][j] = fr->layer3.hybrid_in[ch][sb][ss];
	}
}
#endif

/* And at the end... the main layer3 handler */
int do_layer3(mpg123_handle *fr)
{
	int gr, ch, ss,clip=0;
	int scalefacs[2][39]; /* max 39 for short[13][3] mode, mixed: 38, long: 22 */
	struct III_sideinfo sideinfo;
	int stereo = fr->stereo;
	int single = fr->single;
	int ms_stereo,i_stereo;
	int sfreq = fr->hdr.sampling_frequency;
	int stereo1,granules;

	if(stereo == 1)
	{ /* stream is mono */
		stereo1 = 1;
		single = SINGLE_LEFT;
	}
	else if(single != SINGLE_STEREO) /* stream is stereo, but force to mono */
	stereo1 = 1;
	else
	stereo1 = 2;

	if(fr->hdr.mode == MPG_MD_JOINT_STEREO)
	{
		ms_stereo = (fr->hdr.mode_ext & 0x2)>>1;
		i_stereo  = fr->hdr.mode_ext & 0x1;
	}
	else ms_stereo = i_stereo = 0;

	granules = fr->hdr.lsf ? 1 : 2;

	/* quick hack to keep the music playing */
	/* after having seen this nasty test file... */
	if(III_get_side_info(fr, &sideinfo,stereo,ms_stereo,sfreq,single))
	{
		if(NOQUIET) error("bad frame - unable to get valid sideinfo");
		return clip;
	}

	set_pointer(fr, 1, sideinfo.main_data_begin);
#ifndef NO_MOREINFO
	if(fr->pinfo)
	{
		fr->pinfo->maindata = sideinfo.main_data_begin;
		fr->pinfo->padding  = fr->hdr.padding;
	}
#endif
	for(gr=0;gr<granules;gr++)
	{
		/*  hybridIn[2][SBLIMIT][SSLIMIT] */
		real (*hybridIn)[SBLIMIT][SSLIMIT] = fr->layer3.hybrid_in;
		/*  hybridOut[2][SSLIMIT][SBLIMIT] */
		real (*hybridOut)[SSLIMIT][SBLIMIT] = fr->layer3.hybrid_out;

		{
			struct gr_info_s *gr_info = &(sideinfo.ch[0].gr[gr]);
			long part2bits;
			if(gr_info->part2_3_length > fr->bits_avail)
			{
				if(NOQUIET)
					error2(
						"part2_3_length (%u) too large for available bit count (%li)"
					,	gr_info->part2_3_length, fr->bits_avail );
				return clip;
			}
			if(fr->hdr.lsf)
			part2bits = III_get_scale_factors_2(fr, scalefacs[0],gr_info,0);
			else
			part2bits = III_get_scale_factors_1(fr, scalefacs[0],gr_info,0,gr);

			if(part2bits < 0)
			{
				if(VERBOSE2)
					error("not enough bits for scale factors");
				return clip;
			}

#ifndef NO_MOREINFO
			if(fr->pinfo)
			{
				int i;
				fr->pinfo->sfbits[gr][0] = part2bits;
				for(i=0; i<39; ++i)
					fr->pinfo->sfb_s[gr][0][i] = scalefacs[0][i];
			}
#endif

			if(III_dequantize_sample(fr, hybridIn[0], scalefacs[0],gr_info,sfreq,part2bits))
			{
				if(NOQUIET)
					error("dequantization failed!");
				return clip;
			}
			if(fr->bits_avail < 0)
			{
				if(NOQUIET)
					error("bit deficit after dequant");
				return clip;
			}
		}

		if(stereo == 2)
		{
			struct gr_info_s *gr_info = &(sideinfo.ch[1].gr[gr]);
			long part2bits;
			if(fr->hdr.lsf) 
			part2bits = III_get_scale_factors_2(fr, scalefacs[1],gr_info,i_stereo);
			else
			part2bits = III_get_scale_factors_1(fr, scalefacs[1],gr_info,1,gr);

			if(part2bits < 0)
			{
				if(VERBOSE2)
					error("not enough bits for scale factors");
				return clip;
			}

#ifndef NO_MOREINFO
			if(fr->pinfo)
			{
				int i;
				fr->pinfo->sfbits[gr][1] = part2bits;
				for(i=0; i<39; ++i)
					fr->pinfo->sfb_s[gr][1][i] = scalefacs[1][i];
			}
#endif

			if(III_dequantize_sample(fr, hybridIn[1],scalefacs[1],gr_info,sfreq,part2bits))
			{
				if(NOQUIET)
					error("dequantization failed!");
				return clip;
			}
			if(fr->bits_avail < 0)
			{
				if(NOQUIET)
					error("bit deficit after dequant");
				return clip;
			}

			if(ms_stereo)
			{
				int i;
				unsigned int maxb = sideinfo.ch[0].gr[gr].maxb;
				if(sideinfo.ch[1].gr[gr].maxb > maxb) maxb = sideinfo.ch[1].gr[gr].maxb;

				for(i=0;i<SSLIMIT*(int)maxb;i++)
				{
					real tmp0 = ((real *)hybridIn[0])[i];
					real tmp1 = ((real *)hybridIn[1])[i];
					((real *)hybridIn[0])[i] = tmp0 + tmp1;
					((real *)hybridIn[1])[i] = tmp0 - tmp1;
				}
			}

			if(i_stereo) III_i_stereo(hybridIn,scalefacs[1],gr_info,sfreq,ms_stereo,fr->hdr.lsf);

			if(ms_stereo || i_stereo || (single == SINGLE_MIX) )
			{
				if(gr_info->maxb > sideinfo.ch[0].gr[gr].maxb) 
				sideinfo.ch[0].gr[gr].maxb = gr_info->maxb;
				else
				gr_info->maxb = sideinfo.ch[0].gr[gr].maxb;
			}

			switch(single)
			{
				case SINGLE_MIX:
				{
					register int i;
					register real *in0 = (real *) hybridIn[0],*in1 = (real *) hybridIn[1];
					for(i=0;i<SSLIMIT*(int)gr_info->maxb;i++,in0++)
					*in0 = (*in0 + *in1++); /* *0.5 done by pow-scale */ 
				}
				break;
				case SINGLE_RIGHT:
				{
					register int i;
					register real *in0 = (real *) hybridIn[0],*in1 = (real *) hybridIn[1];
					for(i=0;i<SSLIMIT*(int)gr_info->maxb;i++)
					*in0++ = *in1++;
				}
				break;
			}
		}

#ifndef NO_MOREINFO
		if(fr->pinfo)
			fill_pinfo_side(fr, &sideinfo, gr, stereo1);
#endif

		for(ch=0;ch<stereo1;ch++)
		{
			struct gr_info_s *gr_info = &(sideinfo.ch[ch].gr[gr]);
			III_antialias(hybridIn[ch],gr_info);
			III_hybrid(hybridIn[ch], hybridOut[ch], ch,gr_info, fr);
		}

#ifdef OPT_I486
		if(single != SINGLE_STEREO || fr->af.encoding != MPG123_ENC_SIGNED_16 || fr->down_sample != 0)
		{
#endif
		for(ss=0;ss<SSLIMIT;ss++)
		{
			if(single != SINGLE_STEREO)
			clip += (fr->synth_mono)(hybridOut[0][ss], fr);
			else
			clip += (fr->synth_stereo)(hybridOut[0][ss], hybridOut[1][ss], fr);

		}
#ifdef OPT_I486
		} else
		{
			/* Only stereo, 16 bits benefit from the 486 optimization. */
			ss=0;
			while(ss < SSLIMIT)
			{
				int n;
				n=(fr->buffer.size - fr->buffer.fill) / (2*2*32);
				if(n > (SSLIMIT-ss)) n=SSLIMIT-ss;

				/* Clip counting makes no sense with this function. */
				absynth_1to1_i486(hybridOut[0][ss], 0, fr, n);
				absynth_1to1_i486(hybridOut[1][ss], 1, fr, n);
				ss+=n;
				fr->buffer.fill+=(2*2*32)*n;
			}
		}
#endif
	}
  
	return clip;
}
