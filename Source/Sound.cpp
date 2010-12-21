#include "CommonTypes.h"
#include "Sound.h"
#include <iostream>
#include "Types.h"
#include "LZ77.h"
#include <stack>

struct BNS
{
	struct BNSHeader
	{
		FourCC magic;
		u32 endian;
		u32 size;
		u32 unk;
		u32 info_off;
		u32 info_len;
		u32 data_off;
		u32 data_len;
	};

	struct BNSInfo
	{
		FourCC magic;
		u32 size;
		u8 codec;
		u8 loop;
		u8 num_channels;
		u8 unk;
		u16 sample_rate;
		u16 unk2;
		u32 loop_start; // do we care?
		// TODO SO MANY unknowns
		u32 right_start;
		s16 coefs[2][16];
	};

	struct BNSData
	{
		FourCC magic;
		u32 size;
	};

	struct DSPRegs
	{
		s16 coefs[16];
		u8 pred_scale;
		s16 yn1;
		s16 yn2;

		void ClearHistory() { yn1 = yn2 = 0; }
	};

	std::streamoff start;
	BNSHeader hdr;
	BNSInfo info;
	BNSData data;
	u8 *adpcm;
	DSPRegs dsp_regs;

	BNS() : adpcm(nullptr) {}
	~BNS() { delete [] adpcm; }

	void Open(std::istream& in)
	{
		start = in.tellg();

		in >> hdr.magic >> BE >> hdr.endian >> hdr.size >> hdr.unk
			>> hdr.info_off >> hdr.info_len >> hdr.data_off >> hdr.data_len;

		in.seekg(start + hdr.info_off, in.beg);
		in >> info.magic >> BE >> info.size >> info.codec >> info.loop
			>> info.num_channels >> info.unk >> info.sample_rate
			>> info.unk2 >> info.loop_start;

		in.seekg(7 * sizeof(u32), in.cur);
		
		if (info.num_channels == 1)
		{
			ReadBEArray(in, info.coefs[0], 16);
		}
		else if (info.num_channels == 2)
		{
			// L, R
			in.seekg(1 * sizeof(u32), in.cur);
			in >> BE >> info.right_start;
			in.seekg(2 * sizeof(u32), in.cur);
			ReadBEArray(in, info.coefs[0], 16);
			in.seekg(4 * sizeof(u32), in.cur);
			ReadBEArray(in, info.coefs[1], 16);
		}
		else
		{
			std::cout << info.num_channels << " channels unsupported!\n";
		}

		in.seekg(start + hdr.data_off, in.beg);
		in >> data.magic >> BE >> data.size;
		adpcm = new u8[data.size];
		ReadBEArray(in, adpcm, data.size);

		if ((hdr.info_len != info.size)
			|| (hdr.data_len != data.size)
			|| (info.magic != "INFO")
			|| (data.magic != "DATA"))
			std::cout << "sound.bin appears invalid\n";
	}

	u32 DecodeToPCM(s16 *pcm)
	{		
		u32 pcm_pos;

		dsp_regs.ClearHistory();
		memcpy(dsp_regs.coefs, info.coefs[0], 16 * 2);
		pcm_pos = DecodeChannelToPCM(pcm, 0, 0,
			(info.num_channels == 2) ? info.right_start : data.size);

		if (info.num_channels == 2)
		{
			dsp_regs.ClearHistory();
			memcpy(dsp_regs.coefs, info.coefs[1], 16 * 2);
			DecodeChannelToPCM(pcm, 1, info.right_start,
				data.size - info.right_start);
		}

		return pcm_pos;
	}

	u32 DecodeChannelToPCM(s16 *pcm, u32 pcm_start_pos,
		u32 adpcm_start_pos, u32 adpcm_size)
	{
		u32 adpcm_pos = adpcm_start_pos;
		u32 pcm_pos = pcm_start_pos;

		while (adpcm_pos < adpcm_size * 2)
		{
			if ((adpcm_pos & 15) == 0)
			{
				dsp_regs.pred_scale = adpcm[(adpcm_pos & ~15) / 2];
				adpcm_pos += 2;
			}

			int scale = 1 << (dsp_regs.pred_scale & 0xf);
			int coef_idx = (dsp_regs.pred_scale >> 4) & 7;

			s32 coef1 = dsp_regs.coefs[coef_idx * 2];
			s32 coef2 = dsp_regs.coefs[coef_idx * 2 + 1];

			for (int i = 0; i < 14; i++, adpcm_pos++)
			{
				// unpack a nybble
				s16 nybble = (adpcm_pos & 1) ?
					adpcm[adpcm_pos / 2] & 0xf : adpcm[adpcm_pos / 2] >> 4;

				// sign extension
				if (nybble >= 8)
					nybble -= 16;

				// calc the sample
				int sample = (scale * nybble) +
					((0x400 + coef1 * dsp_regs.yn1 + coef2 * dsp_regs.yn2) >> 11);

				// clamp
				if (sample > 0x7FFF)
					sample = 0x7FFF;
				else if (sample < -0x7FFF)
					sample = -0x7FFF;

				// history
				dsp_regs.yn2 = dsp_regs.yn1;
				dsp_regs.yn1 = sample;

				pcm[pcm_pos++] = sample;
				if (info.num_channels == 2)
					pcm_pos++;
			}
		}
		return pcm_pos;
	}
};


BannerStream::BannerStream()
	: myOffset(0)
	, myBufferSize(40000)
{
}

bool BannerStream::Open(std::istream& in)
{
	BNS bns_file;
	const std::streamoff in_start = in.tellg();
	FourCC magic;
	u32 file_len;

	in >> magic;

	if (magic == "LZ77")
	{
		std::cout << "sound.bin compressed\n";
		LZ77Decompressor decomp(in);
		// TODO
	}

	if (magic == "RIFF")
	{
		format = FORMAT_WAV;
		in >> LE >> file_len;
	}
	else if (magic == "FORM")
	{
		format = FORMAT_AIFF;
		in >> BE >> file_len;
	}
	else if (magic == "BNS ")
	{
		format = FORMAT_BNS;
		in.seekg(in_start, in.beg);
		bns_file.Open(in);
	}
	else
		return false;

	char *data = nullptr;
	bool ret = false;
	sf::SoundBuffer SoundData;

	if (format == FORMAT_WAV || format == FORMAT_AIFF)
	{
		in.seekg(in_start, in.beg);
		char *data = new char[file_len];
		ReadLEArray(in, data, file_len);
		ret = SoundData.LoadFromMemory(data, file_len);
		delete [] data;
	}
	else
	{
		s16 *pcm = new s16[bns_file.data.size * 2];
		u32 pcm_samples = bns_file.DecodeToPCM(pcm);
		ret = SoundData.LoadFromSamples(pcm, pcm_samples,
			bns_file.info.num_channels, bns_file.info.sample_rate);
		delete [] pcm;
	}

	if (ret)
	{
		Initialize(SoundData.GetChannelsCount(), SoundData.GetSampleRate());
		const sf::Int16* Data = SoundData.GetSamples();
		myBuffer.assign(Data, Data + SoundData.GetSamplesCount());
	}
	
	return ret;
}

bool BannerStream::OnStart()
{
	// Reset the read offset
	myOffset = 0;

	return true;
}

bool BannerStream::OnGetData(sf::SoundStream::Chunk& Data)
{
	// Check if there is enough data to stream
	if (myOffset + myBufferSize >= myBuffer.size())
	{
		// Returning false means that we want to stop playing the stream
		return false;
	}

	// Fill the stream chunk with a pointer to the audio data and the number
	// of samples to stream
	Data.Samples   = &myBuffer[myOffset];
	Data.NbSamples = myBufferSize;

	// Update the read offset
	myOffset += myBufferSize;

	return true;
}
