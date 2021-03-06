/*
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# (c) 2009 Lion
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
*/

#include <errno.h>
#include <stdio.h>
#include <kernel.h>
#include <libgs.h>

#include "internal.h"

/******************************************************************
* Miscellaneous
*
*
******************************************************************/
#define gif_chcr	0x1000a000	// GIF Channel Control Register
#define gif_madr	0x1000a010	// Transfer Address Register
#define gif_qwc		0x1000a020	// Transfer Size Register (in qwords)
#define gif_tadr	0x1000a030	// ...

 #define DMA_TAG_REFE	0x00
 #define DMA_TAG_CNT	0x01
 #define DMA_TAG_NEXT	0x02
 #define DMA_TAG_REF	0x03
 #define DMA_TAG_REFS	0x04
 #define DMA_TAG_CALL	0x05
 #define DMA_TAG_RET	0x06
 #define DMA_TAG_END	0x07

typedef struct {
	unsigned direction	:1;	// Direction
	unsigned pad1		:1; // Pad with zeros
	unsigned mode		:2;	// Mode
	unsigned asp		:2;	// Address stack pointer
	unsigned tte		:1;	// Tag trasfer enable
	unsigned tie		:1;	// Tag interrupt enable
	unsigned start_flag	:1;	// start
	unsigned pad2		:7; // Pad with more zeros
	unsigned tag		:16;// DMAtag
}DMA_CHCR;

void GsDmaInit(void)
{
	/*Reset/init dma(gif channel only)*/
	__asm__(
	"li	$2,0x1000A000	\n"
	"nop				\n"
	"sw	$0,0x80($2)		\n"
	"sw	$0,0($2)		\n"
	"sw	$0,0x30($2)		\n"
	"sw	$0,0x10($2)		\n"
	"sw	$0,0x50($2)		\n"
	"sw	$0,0x40($2)		\n"
	"li	$2,0xFF1F		\n"//	0xFF1F
	"sw	$2,0x1000E010	\n"
	"lw	$2,0x1000E010	\n"
	"li	$3,0xFF1F		\n"//0xFF1F
	"and	$2,$3		\n"
	"sw	$2,0x1000E010	\n"
	"sync.p				\n"
	"sw	$0,0x1000E000	\n"
	"sw	$0,0x1000E020	\n"
	"sw	$0,0x1000E030	\n"
	"sw	$0,0x1000E050	\n"
	"sw	$0,0x1000E040	\n"
	"li	$3,1			\n"
	"lw	$2,0x1000E000	\n"
	"ori	$3,$2,1		\n"
	"nop				\n"
	"sw	$3,0x1000E000	\n"
	"nop				\n"
	);
}

void GsDmaSend(const void *addr, unsigned int qwords)
{
	DMA_CHCR		chcr;
	static char		spr;

	if((unsigned int)addr >= 0x70000000 && (unsigned int)addr <= 0x70003fff)
	{
		spr = 1;
	}
	else
	{
		spr = 0;
	}

	*((volatile unsigned int *)(gif_madr)) = ( unsigned int )((( unsigned int )addr) & 0x7FFFFFFF) << 0 | (unsigned int)((spr) & 0x00000001) << 31;;

	*((volatile unsigned int *)(gif_qwc)) = qwords;

	chcr.direction	=1;
	chcr.mode		=0;
	chcr.asp		=0;
	chcr.tte		=0;
	chcr.tie		=0;
	chcr.start_flag	=1;
	chcr.tag		=0;
	chcr.pad1		=0;
	chcr.pad2		=0;
	*((volatile unsigned int *)(gif_chcr)) = *(unsigned int *)&chcr;
}

void GsDmaSend_tag(const void *addr, unsigned int qwords, const GS_GIF_DMACHAIN_TAG *tag)
{
	DMA_CHCR		chcr;
	static char		spr;

	if((unsigned int)addr >= 0x70000000 && (unsigned int)addr <= 0x70003fff)
	{
		spr = 1;
	}
	else
	{
		spr = 0;
	}

	*((volatile unsigned int *)(gif_madr)) = ( unsigned int )((( unsigned int )addr) & 0x7FFFFFFF) << 0 | (unsigned int)((spr) & 0x00000001) << 31;
	*((volatile unsigned int *)(gif_qwc)) = qwords;
	*((volatile unsigned int *)(gif_tadr)) = ( unsigned int )((( unsigned int )tag) & 0x7FFFFFFF) << 0 | (unsigned int)((0) & 0x00000001) << 31;

	chcr.direction	=1;
	chcr.mode		=1; //chain
	chcr.asp		=0;
	chcr.tte		=0;
	chcr.tie		=0;
	chcr.start_flag	=1;
	chcr.tag		=0;
	chcr.pad1		=0;
	chcr.pad2		=0;
	*((volatile unsigned int *)(gif_chcr)) = *(unsigned int *)&chcr;
}

void GsDmaWait(void)
{
	while(*((volatile unsigned int *)(0x1000a000)) & ((unsigned int)1<<8));
}
