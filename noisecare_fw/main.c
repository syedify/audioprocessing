/*
 * main.c
 */

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include "dsk6713_dip.h"
#include <audio.h>
#include <stdint.h>
#include "dsk6713_led.h"
#include "DSPF_sp_lms.h"
#include "DSPF_sp_fir_gen.h"

#define CHANNEL_SIZE 8
#define SINE_TABLE_SIZE_48 48
#define FREQUENCY_SAMPLE_RATE 6
#define NUM_COEFFS 32
#define SINE_TABLE_SIZE 48

#define DURATION 10
#define ADAPTATION_RATE 0.00000000001
#define MAX_ERROR ADAPTATION_RATE/100  //100 //1000
#define BUFF_SIZE 8
#define SAMPLE_FREQ 8
#define NUM_COEFFICIENTS 36
#define NUM_SAMPLES 16//16 //36 //24 //32

#define TONE	0
#define NOISE 	1

typedef union {	Uint32 uint; short channel[2]; } AIC23_data;

DSK6713_AIC23_Config config = { \
	0x0017,  /* 0 LEFTINVOL  Left line input channel volume */ \
	0x0017,  /* 1 RIGHTINVOL Right line input channel volume */\
	0x01f9,  /* 2 LEFTHPVOL  Left channel headphone volume */  \
	0x01f9,  /* 3 RIGHTHPVOL Right channel headphone volume */ \
	0x0011,  /* 4 ANAPATH    Analog audio path control */      \
	0x0000,  /* 5 DIGPATH    Digital audio path control */     \
	0x0000,  /* 6 POWERDOWN  Power down control */             \
	0x0043,  /* 7 DIGIF      Digital audio interface format */ \
	0x0081,  /* 8 SAMPLERATE Sample rate control */            \
	0x0001   /* 9 DIGACT     Digital interface activation */   \
};

// This is needed to modify the BSL's data channel McBSP configuration
MCBSP_Config AIC23CfgData = {
	MCBSP_FMKS(SPCR, FREE, NO)              |
	MCBSP_FMKS(SPCR, SOFT, NO)              |
	MCBSP_FMKS(SPCR, FRST, YES)             |
	MCBSP_FMKS(SPCR, GRST, YES)             |
	MCBSP_FMKS(SPCR, XINTM, XRDY)           |
	MCBSP_FMKS(SPCR, XSYNCERR, NO)          |
	MCBSP_FMKS(SPCR, XRST, YES)             |
	MCBSP_FMKS(SPCR, DLB, OFF)              |
	MCBSP_FMKS(SPCR, RJUST, RZF)            |
	MCBSP_FMKS(SPCR, CLKSTP, DISABLE)       |
	MCBSP_FMKS(SPCR, DXENA, OFF)            |
	MCBSP_FMKS(SPCR, RINTM, RRDY)           |
	MCBSP_FMKS(SPCR, RSYNCERR, NO)          |
	MCBSP_FMKS(SPCR, RRST, YES),

	MCBSP_FMKS(RCR, RPHASE, SINGLE)         |
	MCBSP_FMKS(RCR, RFRLEN2, DEFAULT)       |
	MCBSP_FMKS(RCR, RWDLEN2, DEFAULT)       |
	MCBSP_FMKS(RCR, RCOMPAND, MSB)          |
	MCBSP_FMKS(RCR, RFIG, NO)               |
	MCBSP_FMKS(RCR, RDATDLY, 0BIT)          |
	MCBSP_FMKS(RCR, RFRLEN1, OF(0))         | // This changes to 1 FRAME
	MCBSP_FMKS(RCR, RWDLEN1, 32BIT)         | // This changes to 32 bits per frame
	MCBSP_FMKS(RCR, RWDREVRS, DISABLE),

	MCBSP_FMKS(XCR, XPHASE, SINGLE)         |
	MCBSP_FMKS(XCR, XFRLEN2, DEFAULT)       |
	MCBSP_FMKS(XCR, XWDLEN2, DEFAULT)       |
	MCBSP_FMKS(XCR, XCOMPAND, MSB)          |
	MCBSP_FMKS(XCR, XFIG, NO)               |
	MCBSP_FMKS(XCR, XDATDLY, 0BIT)          |
	MCBSP_FMKS(XCR, XFRLEN1, OF(0))         | // This changes to 1 FRAME
	MCBSP_FMKS(XCR, XWDLEN1, 32BIT)         | // This changes to 32 bits per frame
	MCBSP_FMKS(XCR, XWDREVRS, DISABLE),

	MCBSP_FMKS(SRGR, GSYNC, DEFAULT)        |
	MCBSP_FMKS(SRGR, CLKSP, DEFAULT)        |
	MCBSP_FMKS(SRGR, CLKSM, DEFAULT)        |
	MCBSP_FMKS(SRGR, FSGM, DEFAULT)         |
	MCBSP_FMKS(SRGR, FPER, DEFAULT)         |
	MCBSP_FMKS(SRGR, FWID, DEFAULT)         |
	MCBSP_FMKS(SRGR, CLKGDV, DEFAULT),

	MCBSP_MCR_DEFAULT,
	MCBSP_RCER_DEFAULT,
	MCBSP_XCER_DEFAULT,

	MCBSP_FMKS(PCR, XIOEN, SP)              |
	MCBSP_FMKS(PCR, RIOEN, SP)              |
	MCBSP_FMKS(PCR, FSXM, EXTERNAL)         |
	MCBSP_FMKS(PCR, FSRM, EXTERNAL)         |
	MCBSP_FMKS(PCR, CLKXM, INPUT)           |
	MCBSP_FMKS(PCR, CLKRM, INPUT)           |
	MCBSP_FMKS(PCR, CLKSSTAT, DEFAULT)      |
	MCBSP_FMKS(PCR, DXSTAT, DEFAULT)        |
	MCBSP_FMKS(PCR, FSXP, ACTIVEHIGH)       |
	MCBSP_FMKS(PCR, FSRP, ACTIVEHIGH)       |
	MCBSP_FMKS(PCR, CLKXP, FALLING)         |
	MCBSP_FMKS(PCR, CLKRP, RISING)
};

//for 500 hz
float filter [36] = {
	 0.0056, 0.0771, 0.0735, 0.0021, 0.0453,-0.0076,-0.0022,-0.0407,-0.0467,-0.0666,-0.0635,-0.0598,-0.0396,	\
	-0.0167, 0.0125, 0.0384, 0.0598, 0.0711, 0.0711, 0.0598, 0.0384, 0.0125,-0.0167,-0.0396,-0.0598,-0.0635,	\
	-0.0666,-0.0467,-0.0407,-0.0022,-0.0076, 0.0453, 0.0021, 0.0735, 0.0771, 0.0056								\
};

void main()
{
	float* inputLMS = malloc (sizeof(*inputLMS)*BUFF_SIZE*NUM_SAMPLES);
	float* desiredLMS = malloc (sizeof(desiredLMS)*BUFF_SIZE*NUM_SAMPLES);
	float* outputLMS = malloc (sizeof(*outputLMS)*BUFF_SIZE*NUM_SAMPLES);

	Int16 outL;
	Int16 outR;
	Uint32 in;

	int ms = 0;
	int i = 0;
	int offset = 0;

	DSK6713_AIC23_CodecHandle codec;

	/* Initialize the board support library, must be called first */
	DSK6713_init();
	DSK6713_LED_init();
	DSK6713_DIP_init();

	/* Start the codec */
	codec = DSK6713_AIC23_openCodec(0, &config);
	/* Configure codec control McBSP */

	/* Configure codec data McBSP */
	MCBSP_config(DSK6713_AIC23_DATAHANDLE, &AIC23CfgData);

	/* Reset the AIC23 */
	DSK6713_AIC23_rset(0, DSK6713_AIC23_RESET, 0);

	/* Configure the rest of the AIC23 registers */
	DSK6713_AIC23_config(0, &config);

	/* Clear any garbage from the codec data port */
	if (MCBSP_rrdy(DSK6713_AIC23_DATAHANDLE))
		MCBSP_read(DSK6713_AIC23_DATAHANDLE);

	/* Start McBSP2 as the codec data channel */
	MCBSP_start(DSK6713_AIC23_DATAHANDLE, MCBSP_XMIT_START |
	MCBSP_RCV_START | MCBSP_SRGR_START | MCBSP_SRGR_FRAMESYNC, 220);
	
	DSK6713_AIC23_setFreq(codec, DSK6713_AIC23_FREQ_8KHZ);

	short s_val;
	float f_val;
	int i_val;
	DSK6713_LED_off(0);
	DSK6713_LED_off(1);
	DSK6713_LED_off(2);
	DSK6713_LED_off(3);
	while(1)
	{	
		AIC23_data data;
		for (i = 0; i < BUFF_SIZE*NUM_SAMPLES; i++)
		{	
			DSK6713_LED_on(3);
			while(!DSK6713_AIC23_read(codec, &data.uint));
			inputLMS[i] = (float)data.channel[NOISE];
			desiredLMS[i] = (float)data.channel[TONE];
		}
		DSK6713_LED_toggle(3);
		DSPF_sp_lms(&inputLMS[1], filter, desiredLMS, outputLMS, ADAPTATION_RATE, MAX_ERROR, 36, BUFF_SIZE*NUM_SAMPLES);

		while(DSK6713_DIP_get(3) == 0)
		{
			DSK6713_LED_on(2);
			for (i = 0; i < BUFF_SIZE*NUM_SAMPLES; i++)
			{
				i_val = (int)outputLMS[i];
				while(!DSK6713_AIC23_write(codec, i_val));
			}
		}
		DSK6713_LED_toggle(2);
		while(DSK6713_DIP_get(2) == 0)
		{
			DSK6713_LED_on(1);
			for (i = 0; i < BUFF_SIZE*NUM_SAMPLES; i++)
			{
				i_val = (int)inputLMS[i];
				while(!DSK6713_AIC23_write(codec, i_val));
			}
		}
		DSK6713_LED_toggle(1);
		while(DSK6713_DIP_get(1) == 0)
		{
			DSK6713_LED_on(0);
			for (i = 0; i < BUFF_SIZE*NUM_SAMPLES; i++)
			{
				i_val = (int)desiredLMS[i];
				while(!DSK6713_AIC23_write(codec, i_val));
			}
		}

		if (DSK6713_DIP_get(0) == 0)
		{
			DSK6713_LED_toggle(0);
			for (i = 0; i < BUFF_SIZE*NUM_SAMPLES; i++)
			{
				i_val = (int)outputLMS[i];
				while(!DSK6713_AIC23_write(codec, i_val));
			}
		}

		if (DSK6713_DIP_get(0) == 0 && DSK6713_DIP_get(3) == 0)
		{
			DSK6713_LED_toggle(0);
			for (i = 0; i < BUFF_SIZE*NUM_SAMPLES; i++)
			{
				f_val = outputLMS[i]+desiredLMS[i];
				i_val = (int) f_val;
				while(!DSK6713_AIC23_write(codec, i_val));
			}
		}

		if (DSK6713_DIP_get(2) == 0 && DSK6713_DIP_get(3) == 0)
		{
			DSPF_sp_fir_gen(desiredLMS, filter, outputLMS, 36, BUFF_SIZE*NUM_SAMPLES);
			for (i = 0; i < BUFF_SIZE*NUM_SAMPLES; i++)
			{
				i_val = (int)outputLMS[i];
				while(!DSK6713_AIC23_write(codec, i_val));
			}
		}
	}
}

