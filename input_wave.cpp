#include "pch.h"
#include "wave.h"
#include "adpcm.h"

#include <array>

class input_wave : public input_stubs
{
private:
	service_ptr_t<file> m_file;
	wave_header m_wave;

	std::array<uint8_t, adpcm_block_size> adpcm_block;
	std::array<uint8_t, pcm_block_size> pcm_block;

public:
	input_wave() : m_wave(), adpcm_block(), pcm_block()
	{
	}

	void open(service_ptr_t<file> p_filehint, const char* p_path, t_input_open_reason p_reason, abort_callback& p_abort)
	{
		m_file = p_filehint;
		input_open_file_helper(m_file, p_path, p_reason, p_abort);

		// Read the section header
		section_info info;
		m_file->read(&info, sizeof(section_info), p_abort);

		// Make sure the file is a wave section
		if (info.type != SECTION_WAVE)
		{
			throw exception_io_unsupported_format();
		}

		// Read the audio header
		m_file->read(&m_wave, sizeof(wave_header), p_abort);
	}

	void get_info(file_info& p_info, abort_callback& p_abort)
	{
		p_info.info_set_int("samplerate", m_wave.sample_rate);
		p_info.info_set_int("channels", 1);
		p_info.info_set_int("bitspersample", bits_per_sample);

		p_info.set_length(
			audio_math::samples_to_time(get_sample_count(p_abort), m_wave.sample_rate)
		);
	}

	t_filestats2 get_stats2(uint32_t f, abort_callback& p_abort)
	{
		return m_file->get_stats2_(f, p_abort);
	}

	void decode_initialize(unsigned p_flags, abort_callback& p_abort)
	{
		// Seek to the audio data
		m_file->seek(36, p_abort);
	}

	bool decode_run(audio_chunk& p_chunk, abort_callback& p_abort)
	{
		// Read the block
		auto read = m_file->read(adpcm_block.data(), adpcm_block.size(), p_abort);

		if (read == 0)
			return false;

		// Decode the block
		adpcm_decode_block(adpcm_block.data(), (int16_t*)pcm_block.data());

		// Set the audio data
		p_chunk.set_data_fixedpoint(pcm_block.data(), pcm_block.size(), m_wave.sample_rate, 1, bits_per_sample, audio_chunk::g_guess_channel_config(1));

		return true;
	}

	void decode_seek(double p_seconds, abort_callback& p_abort)
	{
	}

	bool decode_can_seek()
	{
		return false;
	}

	void retag(const file_info& p_info, abort_callback& p_abort)
	{
		throw exception_tagging_unsupported();
	}

	void remove_tags(abort_callback& p_abort)
	{
		throw exception_tagging_unsupported();
	}

	static bool g_is_our_content_type(const char* p_content_type)
	{
		return false;
	}

	static bool g_is_our_path(const char* p_path, const char* p_extension)
	{
		return stricmp_utf8(p_extension, "wave") == 0;
	}

	static const char* g_get_name()
	{
		return "Crystal Dynamics Wave";
	}

	static GUID g_get_guid()
	{
		// {5BD2C27B-7CBF-4853-BA9B-6C96F3499426}
		return { 0x5bd2c27b, 0x7cbf, 0x4853, { 0xba, 0x9b, 0x6c, 0x96, 0xf3, 0x49, 0x94, 0x26 } };
	}

private:
	uint64_t get_sample_count(abort_callback& p_abort)
	{
		// Get the size without the header
		auto size = m_file->get_size(p_abort) - 36;

		return ((size / adpcm_block_size) * pcm_block_size) / (bits_per_sample / 8);
	}
};

static input_singletrack_factory_t<input_wave> g_input_wave_factory;

DECLARE_FILE_TYPE("Crystal Dynamics Wave", "*.wave");