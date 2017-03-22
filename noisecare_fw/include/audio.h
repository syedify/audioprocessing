/*
 * audio.h
 *
 *  Created on: Feb 1, 2017
 *      Author: Syed
 */

#ifndef AUDIO_H_
#define AUDIO_H_

#include <dsk6713.h>
#include <dsk6713_aic23.h>
#include <math.h>

#define NULLPTR ((void*)0)

#define DUAL_CHANNEL	0
#define LEFT_CHANNEL 	1
#define RIGHT_CHANNEL 	2

#define LEFT  0
#define RIGHT 1

DSK6713_AIC23_CodecHandle Audio_Codec_Init(int freq);

int Audio_Codec_Cleanup (DSK6713_AIC23_CodecHandle codec_h);

int Audio_Out(DSK6713_AIC23_CodecHandle code_h, Uint8 ch, Uint32* bufL, Uint32*bufR, int size);

int Audio_Out_Upsample(DSK6713_AIC23_CodecHandle code_h, Uint8 ch, Uint32* bufL, Uint32*bufR, int size, int sample);

int Audio_Out_Downsample(DSK6713_AIC23_CodecHandle code_h, Uint8 ch, Uint32* bufL, Uint32*bufR, int size, int sample);

#endif /* AUDIO_H_ */
