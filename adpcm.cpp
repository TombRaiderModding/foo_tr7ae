#include "pch.h"
#include "adpcm.h"

#include <algorithm>

static int index_table[16] = {
	-1, -1, -1, -1, 2, 4, 6, 8,
	-1, -1, -1, -1, 2, 4, 6, 8
};

static int step_table[89] = {
	7, 8, 9, 10, 11, 12, 13, 14,
	16, 17, 19, 21, 23, 25, 28, 31,
	34, 37, 41, 45, 50, 55, 60, 66,
	73, 80, 88, 97, 107, 118, 130, 143,
	157, 173, 190, 209, 230, 253, 279, 307,
	337, 371, 408, 449, 494, 544, 598, 658,
	724, 796, 876, 963, 1060, 1166, 1282, 1411,
	1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
	3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
	7132, 7845, 8191, 8191, 8191, 8191, 8191, 8191,
	8191, 8191, 8191, 8191, 8191, 8191, 8191, 8191,
	8191
};

static int decode_sample(int code, int predicted, int step)
{
	auto delta = step >> 3;

	if (code & 1) delta += (step >> 2);
	if (code & 2) delta += (step >> 1);
	if (code & 4) delta += step;
	if (code & 8) delta  = -delta;

	delta += predicted;
	delta  = std::clamp(delta, -32768, 32767);

	return delta;
}

static int next_step_index(int step, int code)
{
	step += index_table[code];
	step  = std::clamp(step, 0, 88);

	return step;
}

void adpcm_decode_block(uint8_t* src, int16_t* dest)
{
	int code, step, index, delta;

	// Initial predictor
	dest[0] = *(int16_t*)src;

	// Sample
	code = src[4] >> 4;
	index = step_table[src[2]];
	delta = decode_sample(code, *(int16_t*)src, index);
	step = next_step_index(src[2], code);

	dest[1] = (int16_t)delta;

	for (int i = 0; i < 31; i++)
	{
		auto sample = src[5];

		// Sample 1
		code = sample & 0xf;
		index = step_table[step];
		delta = decode_sample(code, delta, index);
		step = next_step_index(step, code);

		dest[2] = (int16_t)delta;

		// Sample 2
		code = sample >> 4;
		index = step_table[step];
		delta = decode_sample(code, delta, index);
		step = next_step_index(step, code);

		dest[3] = (int16_t)delta;

		src++;
		dest += 2;
	}
}