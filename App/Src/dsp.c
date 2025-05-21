// ****************************************************************************
//     Filename: DSP.C
// Date Created: 5/19/2025
//
//     Comments: DSP module for the Robertsonics OpenMP3 Player
//
// Build Environment: Visual Studio Code
//                    STM32CubeMx
//
// COPYRIGHT (c) 2025, Robertsonics
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.
//
// ****************************************************************************

#include "player.h"


// ****************************************************************************
// External variables

extern uint16_t gMidiPitchUp;			// MIDI pitch bend up
extern uint16_t gMidiPitchDn;			// MIDI pitch bend down


// ****************************************************************************
// Global variables


// In order not to have to use floating point math, we use a lookup table
//  to convert dB Gain values to linear Q1_15 values that we need for volume
//  scaling. The values are caclulated off-line and imported into this file.
//
// The following table is linear Q1_15 gain values corresponding to -80 dB
//  to 0 dB in 0.5 dB steps. The lowest values of the table have been forced
//  to 0 since they expose quantization error.

q15_t gain_tble[] = {
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		2,
		4,
		5,
		6,
		7,
		8,
		9,
		10,
		12,
		13,
		14,
		15,
		16,
		17,
		18,
		20,
		21,
		22,
		23,
		25,
		26,
		28,
		29,
		31,
		33,
		35,
		37,
		39,
		41,
		44,
		46,
		49,
		52,
		55,
		58,
		62,
		65,
		69,
		73,
		78,
		82,
		87,
		92,
		98,
		104,
		110,
		116,
		123,
		130,
		138,
		146,
		155,
		164,
		174,
		184,
		195,
		207,
		219,
		232,
		246,
		260,
		276,
		292,
		309,
		328,
		347,
		368,
		389,
		413,
		437,
		463,
		490,
		519,
		550,
		583,
		617,
		654,
		693,
		734,
		777,
		823,
		872,
		924,
		978,
		1036,
		1098,
		1163,
		1232,
		1305,
		1382,
		1464,
		1550,
		1642,
		1740,
		1843,
		1952,
		2068,
		2190,
		2320,
		2457,
		2603,
		2757,
		2920,
		3093,
		3277,
		3471,
		3677,
		3894,
		4125,
		4370,
		4629,
		4903,
		5193,
		5501,
		5827,
		6172,
		6538,
		6925,
		7336,
		7771,
		8231,
		8719,
		9235,
		9783,
		10362,
		10976,
		11627,
		12315,
		13045,
		13818,
		14637,
		15504,
		16423,
		17396,
		18427,
		19519,
		20675,
		21900,
		23198,
		24573,
		26029,
		27571,
		29205,
		30935,
		32767
};
	

//*****************************************************************************
// initDsp
//*****************************************************************************
void initDsp(void) {

}


//*****************************************************************************
// dBtoIndex
//*****************************************************************************
uint8_t dBtoIndex(int16_t dbGain)
{

uint8_t idx;
	
	if (dbGain <= MIN_GAIN_DB)
		return 0;
	if (dbGain >= MAX_GAIN_DB)
		return MAX_GAIN_TABLE_ENTRY;
	idx = dbGain - MIN_GAIN_DB;
	return (idx << 1);
}


//*****************************************************************************
// arm_ramp_q15
//*****************************************************************************
void arm_ramp_q15(
	q15_t * pSrc,
	q15_t scaleStart,
	q15_t scaleEnd,
	int8_t shift,
	q15_t * pDst,
	uint32_t blockSize) {

	int8_t kShift = 15 - shift;                    /* shift to apply after scaling */
	uint32_t blkCnt;                               /* loop counter */

	q15_t in1, in2, in3, in4;
	q15_t scaleFract;
	q31_t inA1, inA2;
	q31_t out1, out2, out3, out4;
	int8_t e;
	int32_t delta;
	int32_t accum;

	if (scaleEnd > scaleStart) {
		delta = __QSUB16(scaleEnd, scaleStart) << 16u;
		delta = delta / (blockSize >> 1);
		scaleFract = scaleStart;
		accum = 0;
	
		/*loop Unrolling */
		blkCnt = blockSize >> 2u;

		/* First part of the processing with loop unrolling.  Compute 4 outputs at a time.        
		** a second loop below computes the remaining 1 to 3 samples. */
		while(blkCnt > 0u) {
			
			/* Reading 2 inputs from memory */
			inA1 = *__SIMD32(pSrc)++;
			inA2 = *__SIMD32(pSrc)++;

			/* C = A * scale */
			/* Scale the inputs and then store the 2 results in the destination buffer        
			 * in single cycle by packing the outputs */
			out1 = (q31_t) ((q15_t) (inA1 >> 16) * scaleFract);
			out2 = (q31_t) ((q15_t) inA1 * scaleFract);	  
			accum += delta;
			scaleFract = scaleStart + (q15_t)(accum >> 16);
			out3 = (q31_t) ((q15_t) (inA2 >> 16) * scaleFract);
			out4 = (q31_t) ((q15_t) inA2 * scaleFract);
			accum += delta;
			scaleFract = scaleStart + (q15_t)(accum >> 16);

			/* apply shifting */
			out1 = out1 >> kShift;
			out2 = out2 >> kShift;
			out3 = out3 >> kShift;
			out4 = out4 >> kShift;

			/* saturate the output */
			in1 = (q15_t) (__SSAT(out1, 16));
			in2 = (q15_t) (__SSAT(out2, 16));
			in3 = (q15_t) (__SSAT(out3, 16));
			in4 = (q15_t) (__SSAT(out4, 16));

			/* store the result to destination */
			*__SIMD32(pDst)++ = __PKHBT(in2, in1, 16);
			*__SIMD32(pDst)++ = __PKHBT(in4, in3, 16);

			/* Decrement the loop counter */
			blkCnt--;
		}

		/* If the blockSize is not a multiple of 4, compute any remaining output samples here.    
		** No loop unrolling is used. */
		blkCnt = blockSize % 0x4u;
		e = 0;
		while(blkCnt > 0u) {

			/* C = A * scale */
			/* Scale the input and then store the result in the destination buffer. */
			*pDst++ = (q15_t) (__SSAT(((*pSrc++) * scaleFract) >> kShift, 16));
			  
			/* Modify the scaleFact every other sample */
			if (++e > 1) {
				e = 0;
				accum += delta;
				scaleFract = scaleStart + (q15_t)(accum >> 16);
			}

			/* Decrement the loop counter */
			blkCnt--;
		}
		
	} // if (scaleEnd > scaleStart) {
	
	else if (scaleEnd < scaleStart) {
		delta = __QSUB16(scaleStart, scaleEnd) << 16u;
		delta = delta / (blockSize >> 1);
		scaleFract = scaleStart;
		accum = 0;
	
		/*loop Unrolling */
		blkCnt = blockSize >> 2u;

		/* First part of the processing with loop unrolling.  Compute 4 outputs at a time.        
		** a second loop below computes the remaining 1 to 3 samples. */
		while(blkCnt > 0u) {
			
			/* Reading 2 inputs from memory */
			inA1 = *__SIMD32(pSrc)++;
			inA2 = *__SIMD32(pSrc)++;

			/* C = A * scale */
			/* Scale the inputs and then store the 2 results in the destination buffer        
			 * in single cycle by packing the outputs */
			out1 = (q31_t) ((q15_t) (inA1 >> 16) * scaleFract);
			out2 = (q31_t) ((q15_t) inA1 * scaleFract);	  
			accum += delta;
			scaleFract = scaleStart - (q15_t)(accum >> 16);
			out3 = (q31_t) ((q15_t) (inA2 >> 16) * scaleFract);
			out4 = (q31_t) ((q15_t) inA2 * scaleFract);
			accum += delta;
			scaleFract = scaleStart - (q15_t)(accum >> 16);

			/* apply shifting */
			out1 = out1 >> kShift;
			out2 = out2 >> kShift;
			out3 = out3 >> kShift;
			out4 = out4 >> kShift;

			/* saturate the output */
			in1 = (q15_t) (__SSAT(out1, 16));
			in2 = (q15_t) (__SSAT(out2, 16));
			in3 = (q15_t) (__SSAT(out3, 16));
			in4 = (q15_t) (__SSAT(out4, 16));

			/* store the result to destination */
			*__SIMD32(pDst)++ = __PKHBT(in2, in1, 16);
			*__SIMD32(pDst)++ = __PKHBT(in4, in3, 16);

			/* Decrement the loop counter */
			blkCnt--;
		}

		/* If the blockSize is not a multiple of 4, compute any remaining output samples here.    
		** No loop unrolling is used. */
		blkCnt = blockSize % 0x4u;
		e = 0;
		while(blkCnt > 0u) {

			/* C = A * scale */
			/* Scale the input and then store the result in the destination buffer. */
			*pDst++ = (q15_t) (__SSAT(((*pSrc++) * scaleFract) >> kShift, 16));
			  
			/* Modify the scaleFact every other sample */
			if (++e > 1) {
				e = 0;
				accum += delta;
				scaleFract = scaleStart - (q15_t)(accum >> 16);
			}

			/* Decrement the loop counter */
			blkCnt--;
		}
		
	} // if (scaleEnd < scaleStart) {
	
	else {
		scaleFract = scaleStart;

		/*loop Unrolling */
		blkCnt = blockSize >> 2u;

		/* First part of the processing with loop unrolling.  Compute 4 outputs at a time.        
		** a second loop below computes the remaining 1 to 3 samples. */
		while(blkCnt > 0u) {
			
			/* Reading 2 inputs from memory */
			inA1 = *__SIMD32(pSrc)++;
			inA2 = *__SIMD32(pSrc)++;

			/* C = A * scale */
			/* Scale the inputs and then store the 2 results in the destination buffer        
			 * in single cycle by packing the outputs */
			out1 = (q31_t) ((q15_t) (inA1 >> 16) * scaleFract);
			out2 = (q31_t) ((q15_t) inA1 * scaleFract);	  
			out3 = (q31_t) ((q15_t) (inA2 >> 16) * scaleFract);
			out4 = (q31_t) ((q15_t) inA2 * scaleFract);

			/* apply shifting */
			out1 = out1 >> kShift;
			out2 = out2 >> kShift;
			out3 = out3 >> kShift;
			out4 = out4 >> kShift;

			/* saturate the output */
			in1 = (q15_t) (__SSAT(out1, 16));
			in2 = (q15_t) (__SSAT(out2, 16));
			in3 = (q15_t) (__SSAT(out3, 16));
			in4 = (q15_t) (__SSAT(out4, 16));

			/* store the result to destination */
			*__SIMD32(pDst)++ = __PKHBT(in2, in1, 16);
			*__SIMD32(pDst)++ = __PKHBT(in4, in3, 16);

			/* Decrement the loop counter */
			blkCnt--;
		}

		/* If the blockSize is not a multiple of 4, compute any remaining output samples here.    
		** No loop unrolling is used. */
		blkCnt = blockSize % 0x4u;
		e = 0;
		while(blkCnt > 0u) {

			/* C = A * scale */
			/* Scale the input and then store the result in the destination buffer. */
			*pDst++ = (q15_t) (__SSAT(((*pSrc++) * scaleFract) >> kShift, 16));
			  
			/* Decrement the loop counter */
			blkCnt--;
		}
		
	} // if (scaleEnd == scaleStart) {
}


//*****************************************************************************
// arm_copy_split_q15
//*****************************************************************************
void arm_copy_split_q15(
	q15_t * pSrc,
	q15_t * pDstL,
	q15_t * pDstR,
	uint32_t blockSize)
{
	q31_t inA1, inA2;
	uint32_t blkCnt;                               /* loop counter */

	/* Run the below code for Cortex-M4 and Cortex-M3 */

	/*loop Unrolling */
	blkCnt = blockSize >> 2U;

	/* First part of the processing with loop unrolling.  Compute 4 outputs at a time.
	** a second loop below computes the remaining 1 to 3 samples. */
	while (blkCnt > 0U) {
	  
		/* Reading 2 inputs from memory */
		inA1 = *__SIMD32(pSrc)++;
		inA2 = *__SIMD32(pSrc)++;
		 *__SIMD32(pDstL)++ = __PKHBT(inA1, inA2, 16);
		 *__SIMD32(pDstR)++ = __PKHTB(inA2, inA1, 16);

		/* Decrement the loop counter */
		blkCnt--;
	}

	/* If the blockSize is not a multiple of 4, compute any remaining output samples here.
	** No loop unrolling is used. */
	blkCnt = blockSize % 0x4U;

	while (blkCnt > 0U) {
 		*pDstL++ = *pSrc++;
		*pDstR++ = *pSrc++;
		blkCnt--;
	}
}


//*****************************************************************************
// copy_q15
//*****************************************************************************
void copy_q15(
	q15_t * pSrc,
	q15_t * pDst,
	uint32_t blockSize) {
	
uint32_t cnt;

	for (cnt = 0; cnt < blockSize; cnt++) {  
 		*pDst++ = *pSrc++;
	}
}


//*****************************************************************************
// add_q15
//*****************************************************************************
void add_q15(
	q15_t * pSrcA,
	q15_t * pSrcB,
	q15_t * pDst,
	uint32_t blockSize) {

uint32_t cnt;

    /* C = A + B */
	for (cnt = 0; cnt < blockSize; cnt++) {  
		*pDst++ = (q15_t) __QADD16(*pSrcA++, *pSrcB++);
	}
}


//*****************************************************************************
// dspClip16
//*****************************************************************************
float dspClip16(float fI) {

float fO;

	fO = fI;
	if (fO > 32767.0f) fO = 32767.0f;
	else if (fO < -32768.0f) fO = -32768.0f;
	return fO;
}

//*****************************************************************************
// dspClip32
//*****************************************************************************
float dspClip32(float fI) {

float fO;

	fO = fI;
	if (fO > 2147483647.0f) fO = 2147483647.0f;
	else if (fO < -2147483648.0f) fO = -2147483648.0f;
	return fO;
}
