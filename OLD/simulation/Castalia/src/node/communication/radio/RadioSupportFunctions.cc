/****************************************************************************
 *  Copyright: National ICT Australia,  2007 - 2010                         *
 *  Developed at the ATP lab, Networked Systems research theme              *
 *  Author(s): Athanassios Boulis, Dimos Pediaditakis, Yuriy Tselishchev	*
 *  This file is distributed under the terms in the attached LICENSE file.  *
 *  If you do not find this file, copies can be found by writing to:        *
 *                                                                          *
 *      NICTA, Locked Bag 9013, Alexandria, NSW 1435, Australia             *
 *      Attention:  License Inquiry.                                        *
 *                                                                          *  
 ****************************************************************************/

#include "RadioSupportFunctions.h"

#define ERFINV_ERROR 100000.0

/* Approximates the addition of 2 signals expressed in dBm.
 * Value returned in dBm
 */
float addPower_dBm(float a, float b)
{
	float diff = a - b;
	/* accurate additions for specific diff values
	 * A = 10^(a/10), B = 10^(b/10), A/B = 10^(diff/10)
	 * M = max(A,B), m = min(A,B)
	 * sum = M*(1+ m/M)),  dB_add_to_max = 10*log(1+ m/M)
	 * For diff -> dB_add_to_max
	 * 0 -> 3
	 * 1 -> 2.52
	 * 2 -> 2.12
	 * 3 -> 1.76
	 * 4 -> 1.46
	 * 5 -> 1.17
	 * 6 -> 1
	 * 7 -> 0.79
	 */
	if (diff > 7.0)
		return a;
	if (diff < -7.0)
		return b;
	if (diff > 5.0)
		return (a + 1.0);
	if (diff < -5.0)
		return (b + 1.0);
	if (diff > 3.0)
		return (a + 1.5);
	if (diff < -3.0)
		return (b + 1.5);
	if (diff > 2.0)
		return (a + 2.0);
	if (diff < -2.0)
		return (b + 2.0);
	if (diff > 1.0)
		return (a + 2.5);
	if (diff < -1.0)
		return (b + 2.5);
	if (diff > 0.0)
		return (a + 3.0);
	return (b + 3.0);
}

/* Approximates the subtraction of 2 signals expressed in dBm.
 * Substract signal b from signal a. Value returned in dBm
 */
float subtractPower_dBm(float a, float b)
{
	float diff = a - b;
	/* accurate results for specific diff values
	 * A = 10^(a/10), B = 10^(b/10), A/B = 10^(diff/10)
	 * A, B expressed in mW  a, b expressed in dBm
	 * For diff -> subtraction_result_mW -> subtraction_result_dBm
	 * 0   -> 0                ->  - infinite
	 * 0.5 -> 0.12*B           ->  -9.2dBm + b
	 * 1   -> 0.26*B           ->  -5.8dBm + b
	 * 2   -> 0.58*B           ->  -2.4dBm + b
	 * 3   -> 1.00*B           ->   0.0dBm + b
	 * 4   -> 1.51*B or 0.33*A ->   1.8dBm + b  or -4.81dBm + a
	 * 5   -> 2.16*B or 0.68*A ->   3.3dBm + b  or -1.67dBm + a
	 * 6   -> 2.98*B or 0.75*A ->   4.7dBm + b  or -1.25dBm + a
	 * 7   -> 4.01*B or 0.80*A ->   6.0dBm + b  or -0.97dBm + a
	 * 8   -> 5.31*B or 0.84*A ->               or -0.75dBm + a
	 * 9   -> 6.95*B or 0.87*A ->               or -0.58dBm + a
	 * 10  -> 9.00*B or 0.90*A ->               or -0.45dBm + a
	 * 11  -> 11.6*B or 0.92*A ->               or -0.36dBm + a
	 * 12  -> 14.9*B or 0.94*A ->               or -0.28dBm + a
	 * 13  -> 19.0*B or 0.95*A ->               or -0.22dBm + a
	 */

	/* The approximations we do here subtract a bit more in most cases
	 * This is best as we do not want to have residual error positive
	 * interference. Moreover intereference of many signals is a bit
	 * less than additive, so it's fine to subtract a bit more.
	 */
	if (diff < 0.5)
		return -200.0;	// practically -infinite
	if (diff < 1.0)
		return (b - 9.0);
	if (diff < 2.0)
		return (b - 5.0);
	if (diff < 3.0)
		return (b - 2.0);
	if (diff < 4.0)
		return b;
	if (diff < 5.0)
		return (b + 2.0);
	if (diff < 6.0)
		return (a - 1.6);
	if (diff < 7.0)
		return (a - 1.2);
	if (diff < 8.0)
		return (a - 0.9);
	if (diff < 9.0)
		return (a - 0.7);
	if (diff < 12.0)
		return (a - 0.5);

	return a;

}

float ratioTodB(float ratio)
{
	static float ratioTodB_array[15] = { -12.041200,	// dB returned for 1/16<= ratio <2/16
		-9.030900,	// dB returned for 2/16<= ratio <3/16
		-7.269987,
		-6.020600,
		-5.051500,
		-4.259687,
		-3.590219,
		-3.010300,
		-2.498775,
		-2.041200,
		-1.627273,
		-1.249387,
		-0.901766,
		-0.579919,
		-0.280287
	};			// dB returned for 15/16<= ratio <16/16

	if (ratio >= 1.0)
		return 0.0;	// the input to this function should be ratios up to 1.0
	if (ratio < 0.0625)
		return -100.0;	// if < 1/16 then return a very small number in dB
	return ratioTodB_array[(int)floor(16.0 * ratio) - 1];
}

/* Simple approximation function only used to calculate time of CS Interrupt*/
float dBToRatio(float dB)
{
	if (dB > 9.0)
		return 8.0;
	if (dB > 6.0)
		return 4.0;
	if (dB > 3.0)
		return 2.0;
	if (dB > 1.25)
		return 1.3333;	// = 4/3
	return 1.0;
}

float erfInv(float y)
{
	static float a[] = { 0.0, 0.886226899, -1.645349621, 0.914624893, -0.140543331 };
	static float b[] = { 0.0, -2.118377725, 1.442710462, -0.329097515, 0.012229801 };
	static float c[] = { 0.0, -1.970840454, -1.624906493, 3.429567803, 1.641345311 };
	static float d[] = { 0.0, 3.543889200, 1.637067800 };

	float x, z;

	if (y > -1.) {
		if (y >= -.7) {
			if (y <= .7) {
				z = y * y;
				x = y * (((a[4] * z + a[3]) * z + a[2]) * z + a[1]) /
					((((b[4] * z + b[3]) * z + b[2]) * z + b[1]) * z + 1.0);
			} else if (y < 1) {
				z = sqrt(-log((1 - y) / 2.0));
				x = (((c[4] * z + c[3]) * z + c[2]) * z + c[1]) / 
					((d[2] * z + d[1]) * z + 1.0);
			} else
				return ERFINV_ERROR;
		} else {
			z = sqrt(-log((1 + y) / 2.0));
			x = -(((c[4] * z + c[3]) * z + c[2]) * z + c[1]) / 
				((d[2] * z + d[1]) * z + 1);
		}
	} else
		return ERFINV_ERROR;

	return x;
}

float erfcInv(float y)
{
	return erfInv(1.0 - y);
}

/* SNR->BER curve for differential QPSK modulation.
 * Since the function that gives this curve is rather
 * complex, we precomputed several values in Matlab
 * and we are doing a simple lookup here. Effective
 * range 6.0dB to 12.2dB (resolution 0.2dB)
 */
float diffQPSK_SNR2BER(float SNR)
{
	static float BER_array[32] = {
		0.01723590060469,	// BER for SNR 6.0dB  PER(200bits) = 0.03
		0.01515859222543,	// BER for SNR 6.2dB
		0.01326127169356,	// ...
		0.01153737654219,
		0.00997961569163,
		0.00858004434130,	// BER for SNR 7.0dB
		0.00733015079524,
		0.00622095368274,
		0.00524310766416,
		0.00438701538062,
		0.00364294312896,	// BER for SNR 8.0dB
		0.00300113753889,
		0.00245194041114,
		0.00198589885920,
		0.00159386799020,
		0.00126710356868,	// BER for SNR 9.0dB
		0.00099734242733,
		0.00077686881248,
		0.00059856536337,
		0.00045594799891,
		0.00034318459603,	// BER for SNR 10.0dB
		0.00025509795641,
		0.00018715413812,
		0.00013543774315,
		0.00009661616875,
		0.00006789512818,	// BER for SNR 11.0dB
		0.00004696790793,
		0.00003196084958,
		0.00002137743081,
		0.00001404308722,
		0.00000905258912,	// BER for SNR 12.0dB
		0.00000572139679	// BER for SNR 12.2dB  PER(4000bits) = 0.9773
	};

	// The values of the SNR parameter should be within 6.0 and 12.2 dB, if not
	// we should issue a warning. here we just return appropriate values

	if (SNR < 6.0)
		return 1.0;
	if (SNR >= 12.2)
		return 0.0;

	// the index of the array element that is just less than SNR
	// or in other words: the max array element that is less than SNR
	int index = (int)floor((SNR - 6.0) / 0.2);

	// the remainder, a number in [0, 1)
	float a = ((SNR - 6.0) / 0.2) - index;

	// return a linear combination of the 2 array elements that the SNR
	return (a * BER_array[index] + (1 - a) * BER_array[index + 1]);

}

/*
 * Function to compute the probability of having EXACTLY k errors
 * in total N bits when the Bit Error Rate/Probability is p.
 * This is given by the formula (N choose k) * p^k * (1-p)^(N-k)
 * (N choose k) = N!/((N-k)!*k!) = (N*(N-1)*...*(N-k)) / (1*2*...*k)
 * We have optimized here to return for 0 errors quickly.
 * Due to other optimizations there might be some rounding error
 * in computing (N choose k) but this should be negligible
 */
double probabilityOfExactly_N_Errors(double BER, int errors, int numOfBits)
{
	if (errors == 0)
		return pow(1.0 - BER, numOfBits);

	if (errors >= numOfBits)
		return pow(BER, numOfBits);

	if (errors > numOfBits / 2.0)
		errors = numOfBits - errors;
	double combinations = 1.0;
	for (int i = 1; i <= errors; i++) {
		combinations /= i;
		combinations *= (numOfBits + 1 - i);
	}
	return combinations * pow(BER, errors) * pow(1.0 - BER, numOfBits - errors);
}

RadioControlCommand *createRadioCommand(RadioControlCommand_type kind, double value)
{
	if (kind != SET_TX_OUTPUT && kind != SET_CARRIER_FREQ && kind != SET_CCA_THRESHOLD)
		opp_error("incorrect usage of createRadioCommand, double argument is only compatible with SET_TX_OUTPUT, SET_CCA_THRESHOLD or SET_CARRIER_FREQ");
	RadioControlCommand *cmd = new RadioControlCommand("Radio control command", RADIO_CONTROL_COMMAND);
	cmd->setRadioControlCommandKind(kind);
	cmd->setParameter(value);
	return cmd;
}

RadioControlCommand *createRadioCommand(RadioControlCommand_type kind,const char *name)
{
	if (kind != SET_MODE && kind != SET_SLEEP_LEVEL)
		opp_error("incorrect usage of createRadioCommand, string argument is only compatible with SET_MODE or SET_SLEEP_LEVEL");
	RadioControlCommand *cmd = new RadioControlCommand("Radio control command", RADIO_CONTROL_COMMAND);
	cmd->setRadioControlCommandKind(kind);
	cmd->setName(name);
	return cmd;
}

RadioControlCommand *createRadioCommand(RadioControlCommand_type kind, BasicState_type state)
{
	if (kind != SET_STATE)
		opp_error("incorrect usage of createRadioCommand, double argument is only compatible with SET_TX_OUTPUT or SET_CARRIER_FREQ");
	RadioControlCommand *cmd = new RadioControlCommand("Radio control command", RADIO_CONTROL_COMMAND);
	cmd->setRadioControlCommandKind(kind);
	cmd->setState(state);
	return cmd;
}

RadioControlCommand *createRadioCommand(RadioControlCommand_type kind)
{
	if (kind != SET_CS_INTERRUPT_ON && kind != SET_CS_INTERRUPT_OFF)
		opp_error("incorrect usage of createRadioCommand, no argument is only compatible with SET_CS_INTERRUPT_OFF or SET_CS_INTERRUPT_ON");
	RadioControlCommand *cmd = new RadioControlCommand("Radio control command", RADIO_CONTROL_COMMAND);
	cmd->setRadioControlCommandKind(kind);
	return cmd;
}

