#pragma once

#include <cstdint>

enum section_type : uint8_t
{
	SECTION_GENERAL,
	SECTION_EMPTY,
	SECTION_ANIMATION,
	SECTION_PUSHBUFFER_WC,
	SECTION_PUSHBUFFER,
	SECTION_TEXTURE,
	SECTION_WAVE,
	SECTION_DTPDATA,
	SECTION_SCRIPT,
	SECTION_SHADERLIB,

	NUM_SECTION_TYPES
};

struct section_info
{
	uint32_t magic;

	uint32_t size;
	section_type type;
	uint8_t skip;
	uint16_t version_id;
	uint32_t packed_data;
	int32_t id;
	uint32_t spec_mask;
};

struct wave_header
{
	uint32_t sample_rate;
	uint32_t loop_start;
	uint32_t loop_end;
};