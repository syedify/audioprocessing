/*
 * audio.c
 *
 *  Created on: Feb 1, 2017
 *      Author: Syed
 */

#include <audio.h>

#define AIC23_CONFIG {\
	0x0017,  /* 0 DSK6713_AIC23_LEFTINVOL  Left line input channel volume */ \
    0x0017,  /* 1 DSK6713_AIC23_RIGHTINVOL Right line input channel volume */\
    0x01f9,  /* 2 DSK6713_AIC23_LEFTHPVOL  Left channel headphone volume */  \
    0x01f9,  /* 3 DSK6713_AIC23_RIGHTHPVOL Right channel headphone volume */ \
    0x0011,  /* 4 DSK6713_AIC23_ANAPATH    Analog audio path control */      \
    0x0000,  /* 5 DSK6713_AIC23_DIGPATH    Digital audio path control */     \
    0x0000,  /* 6 DSK6713_AIC23_POWERDOWN  Power down control */             \
    0x0043,  /* 7 DSK6713_AIC23_DIGIF      Digital audio interface format */ \
    0x0001,  /* 8 DSK6713_AIC23_SAMPLERATE Sample rate control */            \
    0x0001   /* 9 DSK6713_AIC23_DIGACT     Digital interface activation */   \
}

/*
 * Frequency supported are 8kHz, 16kHz, 24kHz, 32kHz, 44kHz, 48kHz, 96kHz
 * returns the code on succession
 * returns -1 on error
 */
DSK6713_AIC23_CodecHandle Audio_Codec_Init(int freq)
{
	DSK6713_AIC23_Config cfg = AIC23_CONFIG;
	DSK6713_AIC23_CodecHandle codec_h = DSK6713_AIC23_openCodec(0, &cfg);
	switch (freq)
	{
		case 8:
			DSK6713_AIC23_setFreq(codec_h, DSK6713_AIC23_FREQ_8KHZ);
			break;
		case 16:
			DSK6713_AIC23_setFreq(codec_h, DSK6713_AIC23_FREQ_16KHZ);
			break;
		case 24:
			DSK6713_AIC23_setFreq(codec_h, DSK6713_AIC23_FREQ_24KHZ);
			break;
		case 32:
			DSK6713_AIC23_setFreq(codec_h, DSK6713_AIC23_FREQ_32KHZ);
			break;
		case 44:
			DSK6713_AIC23_setFreq(codec_h, DSK6713_AIC23_FREQ_44KHZ);
			break;
		case 48:
			DSK6713_AIC23_setFreq(codec_h, DSK6713_AIC23_FREQ_48KHZ);
			break;
		case 96:
			DSK6713_AIC23_setFreq(codec_h, DSK6713_AIC23_FREQ_96KHZ);
			break;
		default:
			return -1;
	}
	return codec_h;
}

int Audio_Codec_Cleanup (DSK6713_AIC23_CodecHandle codec_h)
{
	if (codec_h != NULL)
	{
		DSK6713_AIC23_closeCodec(codec_h);
		return 0;
	}
	return -1;
}
/*
 * Pass in the codec handle, desired L or R channel, data buffer and size of buffer
 * returns 0 on successful completion
 * returns -1 on error
 */
int Audio_Out(DSK6713_AIC23_CodecHandle code_h, Uint8 ch, Uint32* bufL, Uint32*bufR, int size)
{
	int i = 0;
	Uint32 empty = 0;

	switch (ch)
	{
		case LEFT_CHANNEL:
			if (bufL == NULLPTR)
				break;

			while (i < size)
			{
				while (!DSK6713_AIC23_write(code_h, bufL[i]));
				while (!DSK6713_AIC23_write(code_h, empty));
				i++;
			}
			return 0;

		case RIGHT_CHANNEL:
			if (bufR == NULLPTR)
				break;

			while (i < size)
			{
				while (!DSK6713_AIC23_write(code_h, empty));
				while (!DSK6713_AIC23_write(code_h, bufR[i]));
				i++;
			}

			return 0;

		case DUAL_CHANNEL:
			if (bufL == NULLPTR || bufR == NULLPTR)
				break;

			while (i < size)
			{
				while (!DSK6713_AIC23_write(code_h, bufL[i]));
				while (!DSK6713_AIC23_write(code_h, bufR[i]));
				i++;
			}

			return 0;

		default:
			break;
	}
	return -1;
}

int Audio_Out_Upsample(DSK6713_AIC23_CodecHandle code_h, Uint8 ch, Uint32* bufL, Uint32*bufR, int size, int sample)
{
	int i;
	Uint32 chanL = 0;
	Uint32 chanR = 0;
	Uint32 empty = 0;

	switch (ch)
	{
		case LEFT_CHANNEL:
			if (bufL == NULLPTR)
				break;

			for (i = 0; i < size; i+=sample)
			{
				chanL = bufL[i%size];
				while (!DSK6713_AIC23_write(code_h, chanL));
				while (!DSK6713_AIC23_write(code_h, empty));
			}
			return 0;

		case RIGHT_CHANNEL:
			if (bufR == NULLPTR)
				break;

			for (i = 0; i < size; i+=sample)
			{
				chanR = bufR[i%size];
				while (!DSK6713_AIC23_write(code_h, empty));
				while (!DSK6713_AIC23_write(code_h, chanR));
			}
			return 0;

		case DUAL_CHANNEL:
			if (bufL == NULLPTR || bufR == NULLPTR)
				break;

			for (i = 0; i < size; i+=sample)
			{
				chanL = bufL[i%size];
				chanR = bufR[i%size];

				while (!DSK6713_AIC23_write(code_h, chanL));
				while (!DSK6713_AIC23_write(code_h, chanR));
			}
			return 0;

		default:
			break;
	}
	return -1;
}

int Audio_Out_Downsample(DSK6713_AIC23_CodecHandle code_h, Uint8 ch, Uint32* bufL, Uint32*bufR, int size, int sample)
{
	int i;
	int j;
	Uint32 chanL = 0;
	Uint32 chanR = 0;
	Uint32 empty = 0;

	switch (ch)
	{
		case LEFT_CHANNEL:
			if (bufL == NULLPTR)
				break;

			for (i = 0; i < size; i++)
			{
				chanL = bufL[i];
				for (j = 0; j < sample; j++)
				{
					while (!DSK6713_AIC23_write(code_h, chanL));
					while (!DSK6713_AIC23_write(code_h, empty));
				}
			}
			return 0;

		case RIGHT_CHANNEL:
			if (bufR == NULLPTR)
				break;

			for (i = 0; i < size; i++)
			{
				chanR = bufR[i];
				for (j = 0; j < sample; j++)
				{
					while (!DSK6713_AIC23_write(code_h, empty));
					while (!DSK6713_AIC23_write(code_h, chanR));
				}
			}
			return 0;

		case DUAL_CHANNEL:
			if (bufL == NULLPTR || bufR == NULLPTR)
				break;

			for (i = 0; i < size; i++)
			{
				chanL = bufL[i];
				chanR = bufR[i];

				for (j = 0; j < sample; j++)
				{
					while (!DSK6713_AIC23_write(code_h, chanL));
					while (!DSK6713_AIC23_write(code_h, chanR));
				}
			}
			return 0;

		default:
			break;
	}
	return -1;
}
