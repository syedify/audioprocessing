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

//#define M_PI 3.14159265358979323846

#define DURATION 10
//#define ADAPTATION_RATE 0.00000000001
#define ADAPTATION_RATE 0.00000000001
//#define MAX_ERROR 1
#define MAX_ERROR ADAPTATION_RATE/100  //100 //1000
#define BUFF_SIZE 8
#define SAMPLE_FREQ 8
#define NUM_COEFFICIENTS 36
#define NUM_SAMPLES 16//16 //36 //24 //32



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
#if 1
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
#endif


//#define beta 0.01
//#define N 30

// Codec configuration settings
//DSK6713_AIC23_Config config = { \
//    0x0017,  /* 0 DSK6713_AIC23_LEFTINVOL  Left line input channel volume */ \
//    0x0017,  /* 1 DSK6713_AIC23_RIGHTINVOL Right line input channel volume */\
//    0x01f9,  /* 2 DSK6713_AIC23_LEFTHPVOL  Left channel headphone volume */  \
//    0x01f9,  /* 3 DSK6713_AIC23_RIGHTHPVOL Right channel headphone volume */ \
//    0x0011,  /* 4 DSK6713_AIC23_ANAPATH    Analog audio path control */      \
//    0x0000,  /* 5 DSK6713_AIC23_DIGPATH    Digital audio path control */     \
//    0x0000,  /* 6 DSK6713_AIC23_POWERDOWN  Power down control */             \
//    0x0043,  /* 7 DSK6713_AIC23_DIGIF      Digital audio interface format */ \
//    0x0001,  /* 8 DSK6713_AIC23_SAMPLERATE Sample rate control */            \
//    0x0001   /* 9 DSK6713_AIC23_DIGACT     Digital interface activation */   \
//};

//for 500 hz
float filter [36] = {
	 0.0056, 0.0771, 0.0735, 0.0021, 0.0453,-0.0076,-0.0022,-0.0407,-0.0467,-0.0666,-0.0635,-0.0598,-0.0396,	\
	-0.0167, 0.0125, 0.0384, 0.0598, 0.0711, 0.0711, 0.0598, 0.0384, 0.0125,-0.0167,-0.0396,-0.0598,-0.0635,	\
	-0.0666,-0.0467,-0.0407,-0.0022,-0.0076, 0.0453, 0.0021, 0.0735, 0.0771, 0.0056								\
};
//
//for 100hz
// float filter[36] = {
// 	0.0058,0.0955,0.0148,0.0216,0.0231,0.0248,0.0265,0.0282,0.0298,0.0312,0.0324,0.0336,	\
// 	0.0347,0.0356,0.0362,0.0368,0.0372,0.0373,0.0373,0.0372,0.0368,0.0362,0.0356,0.0347,	\
// 	0.0336,0.0324,0.0312,0.0298,0.0282,0.0265,0.0248,0.0231,0.0216,0.0148,0.0955,0.0058		\
// };

//DO NOT TOUCH VALUES ON THE HEAP!!!!!!!!!!!!!!!!
// Uint32 SINE[SINE_TABLE_SIZE] = {
//     0x0000, 0x10b4, 0x2120, 0x30fb, 0x3fff, 0x4dea,
//     0x5a81, 0x658b, 0x6ed8, 0x763f, 0x7ba1, 0x7ee5,
//     0x7ffd, 0x7ee5, 0x7ba1, 0x76ef, 0x6ed8, 0x658b,
//     0x5a81, 0x4dea, 0x3fff, 0x30fb, 0x2120, 0x10b4,
//     0x0000, 0xef4c, 0xdee0, 0xcf06, 0xc002, 0xb216,
//     0xa57f, 0x9a75, 0x9128, 0x89c1, 0x845f, 0x811b,
//     0x8002, 0x811b, 0x845f, 0x89c1,0x9128, 0x9a76,
//     0xa57f, 0xb216, 0xc002, 0xcf06, 0xdee0, 0xef4c
// };




// float filterBPF [36] = {
// 		 0.0056, 0.0771, 0.0735, 0.0021, 0.0453,-0.0076,-0.0022,-0.0407,-0.0467,-0.0666,-0.0635,-0.0598,-0.0396,	\
// 		-0.0167, 0.0125, 0.0384, 0.0598, 0.0711, 0.0711, 0.0598, 0.0384, 0.0125,-0.0167,-0.0396,-0.0598,-0.0635,	\
// 		-0.0666,-0.0467,-0.0407,-0.0022,-0.0076, 0.0453, 0.0021, 0.0735, 0.0771, 0.0056								\
// };


// //TODO: DO NOT TOUCH, OUTPUT BUFFER DOES NOT WORK FOR SOME REASON IF FUNCTOIN NOT PRESENT
// void ftoa(float n, char *res, int afterpoint)
// {
//     // Extract integer part
//     int ipart = (int)n;

//     // Extract floating part
//     float fpart = n - (float)ipart;

//     // convert integer part to string
//     //int i = intToStr(ipart, res, 0);

//     // check for display option after point
//     if (afterpoint != 0)
//     {
// //        res[i] = '.';  // add dot

//         // Get the value of fraction part upto given no.
//         // of points after dot. The third parameter is needed
//         // to handle cases like 233.007
//         fpart = fpart * pow(10, afterpoint);

//         //intToStr((int)fpart, res + i + 1, afterpoint);
//     }
// }
#define TONE	0
#define NOISE 	1
void main()
{
	//Odd indices = Left Channel, Even indices = Right Channel

	//Uint32* input = malloc (sizeof(Uint32)*BUFF_SIZE*NUM_SAMPLES);
	//AIC23_data* input = malloc (sizeof(*input)*BUFF_SIZE*NUM_SAMPLES);
	//AIC2_data* output = malloc (sizeof(*output)*BUFF_SIZE*NUM_SAMPLES);
	float* inputLMS = malloc (sizeof(*inputLMS)*BUFF_SIZE*NUM_SAMPLES);
	float* desiredLMS = malloc (sizeof(desiredLMS)*BUFF_SIZE*NUM_SAMPLES);
	float* outputLMS = malloc (sizeof(*outputLMS)*BUFF_SIZE*NUM_SAMPLES);
	//Int16* output = malloc (sizeof(Int16)*BUFF_SIZE*NUM_SAMPLES);

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
	//DSK6713_AIC23_setFreq(codec, DSK6713_AIC23_FREQ_48KHZ);

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
		#if 0 //EXAMPLE WHICH CHANNEL IS WHICH
		while(!DSK6713_AIC23_read(codec, &data.uint));
		if (DSK6713_DIP_get(0) == 0)
		{
			while(!DSK6713_AIC23_write(codec, data.channel[0]));	
		}
		else if (DSK6713_DIP_get(1) == 0)
		{

			while(!DSK6713_AIC23_write(codec, data.channel[1]));
		}
		else if (DSK6713_DIP_get(2) == 0)
		{
			f_val = (float)data.channel[0];
			i_val = (int)f_val;
			while(!DSK6713_AIC23_write(codec, i_val));
		}
		else if (DSK6713_DIP_get(3) == 0)
		{
			f_val = (float)data.channel[1];
			i_val = (int)f_val;
			while(!DSK6713_AIC23_write(codec, i_val));
		}
		else
		{

			while(!DSK6713_AIC23_write(codec, data.uint));
		}
		#endif
		
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
//
//		while (DSK6713_DIP_get(0) == 0)
//		{
			// float max = 0;
			// /* Find the peak value */
			// for (i = 0; i < BUFF_SIZE*NUM_SAMPLES; i++)
			// {
			// 	f_val = outputLMS[i]+desiredLMS[i];
			// 	if (f_val > max)
			// 		max = f_val;
			// }

//			for (i = 0; i < BUFF_SIZE*NUM_SAMPLES; i++)
//			{
//				/* Add and normalise the summation */
//				// f_val = (outputLMS[i]+desiredLMS[i])/(max/2.0f);
//				f_val = outputLMS[i]+desiredLMS[i];
//				i_val = (int) f_val;
//				while(!DSK6713_AIC23_write(codec, i_val));
//			}
//		}
#if 0
//		for (ms = 0; ms < NUM_SAMPLES; ms++)
//		{
//			offset = ms*BUFF_SIZE;
//			DSK6713_LED_toggle(0);
//
//			for (i = 0; i < BUFF_SIZE; i+=2)
//			{
//				/* Get input data */
//
//				/* Get the noise signal */
//				while(!DSK6713_AIC23_read(codec, &input[i+offset]);
//				/* Get the desired signal */
//				while(!DSK6713_AIC23_read(codec, &input[i+1+offset]);
//				/* Write to LMS buffer*/
//				inputLMS [i+offset] = (float) input[i+offset];
//				desiredLMS [i+offset] = (float) input[i+1+offset];
//			}
//		}
		for (i = 0; i < NUM_SAMPLES*BUFF_SIZE; i++)
		{
			DSK6713_LED_toggle(0);

			/* Get input data */

			/* Get the noise signal */
			while(!DSK6713_AIC23_read(codec, &in));
			in = input[i].uint;
			/* Write to LMS buffer*/
			inputLMS [i] = (float) input[i].channel[LEFT];
			/* Get the desired signal */
			desiredLMS [i] = (float) input[i].channel[RIGHT];
		}

		DSK6713_LED_on(0);

		/* LEFT INPUT - NOISE */
		while (DSK6713_DIP_get(3)==0)
		{
//			for (ms = 0; ms < NUM_SAMPLES; ms++)
//			{
//				offset = ms*BUFF_SIZE;
//
//				for (i = 0; i < BUFF_SIZE; i+=2)
//				{
//					outL = (Int16) inputLMS [i+offset];
//					//outL = (Int16) input[i+offset];
//					while (!DSK6713_AIC23_write(codec, outL));
//					while (!DSK6713_AIC23_write(codec, outL));
//				}
//			}
			for (i = 0; i < NUM_SAMPLES*BUFF_SIZE; i++)
			{
				outL = (Uint32) inputLMS[i];
				//while (!DSK6713_AIC23_write(codec, outL));
				while (!DSK6713_AIC23_write(codec, outL));
			}
		}
		DSK6713_LED_toggle(1);
		/* RIGHT INPUT - DESIRED TONE*/
		while (DSK6713_DIP_get(2)==0)
		{
//			for (ms = 0; ms < NUM_SAMPLES; ms++)
//			{
//				offset = ms*BUFF_SIZE;
//				for (i = 0; i < BUFF_SIZE; i+=2)
//				{
//					//outR = (Int16) desiredLMS[i+offset];
//					outR = (Int16) input[i+1+offset];
//					while (!DSK6713_AIC23_write(codec, outR));
//					while (!DSK6713_AIC23_write(codec, outR));
//				}
//			}
			for (i = 0; i < NUM_SAMPLES*BUFF_SIZE; i++)
			{
				outR = (Uint32) desiredLMS[i];
				//while (!DSK6713_AIC23_write(codec, outL));
				while (!DSK6713_AIC23_write(codec, outL));
			}
		}

		DSK6713_LED_toggle(1);


		DSPF_sp_lms(&inputLMS[1], filter, desiredLMS, outputLMS, ADAPTATION_RATE, MAX_ERROR, 36, BUFF_SIZE*NUM_SAMPLES);

		for (i = 0; i < BUFF_SIZE*NUM_SAMPLES; i++)
		{
			DSK6713_LED_toggle(2);
			output[i] = (Int16) outputLMS[i];
		}
		DSK6713_LED_toggle(1);
		DSK6713_LED_toggle(0);

		/* OUTPUT  */
		while (DSK6713_DIP_get(0) == 0)
		{
//			for (ms = 10; ms < NUM_SAMPLES; ms++)
//			{
//				offset = ms*BUFF_SIZE;
//				DSK6713_LED_toggle(3);
//				for (i = 0; i < BUFF_SIZE; i+=2)
//				{
//					/* LMS OUTPUT SIGNAL */
//					outL = (Int16) output[i+offset];
//					/* DESIRED SIGNAL TONE */
//					//outR = (Int16) input[i+1+offset];
//					outR = (Int16) output[i+1+offset];
//
//					while (!DSK6713_AIC23_write(codec, outL));
//					while (!DSK6713_AIC23_write(codec, outR));
//				}
//				//Audio_Out(codec, RIGHT_CHANNEL, &output[offset], &output[offset], BUFF_SIZE);
//			}
			for (i = 0; i < NUM_SAMPLES*BUFF_SIZE; i++)
			{
				//outL = (Int16) output[i];
				//while (!DSK6713_AIC23_write(codec, outL));
				while (!DSK6713_AIC23_write(codec, (int)output[i]));
			}
		}

//		for (ms = 10; ms < NUM_SAMPLES; ms++)
//		{
//			offset = ms*BUFF_SIZE;
//			DSK6713_LED_toggle(3);
//			for (i = 0; i < BUFF_SIZE; i++)
//			{
//				outL = output[i+offset];
//				outR = output[i+1+offset];
//
//				while (!DSK6713_AIC23_write(codec, outL));
//				while (!DSK6713_AIC23_write(codec, outR));
//			}
//		}
#endif
	}
}

