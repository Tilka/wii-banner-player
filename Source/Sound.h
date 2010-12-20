#pragma once

#include <sstream>
#include <SFML/Audio.hpp>

class BannerStream : public sf::SoundStream
{
public:
	BannerStream();
	bool Open(std::istream& in);

private:
	virtual bool OnStart();
	virtual bool OnGetData(sf::SoundStream::Chunk& Data);

	std::vector<sf::Int16> myBuffer;     ///< Internal buffer that holds audio samples
	std::size_t            myOffset;     ///< Read offset in the sample array
	std::size_t            myBufferSize; ///< Size of the audio data to stream

	enum SoundFormat
	{
		FORMAT_BNS,
		FORMAT_WAV,
		FORMAT_AIFF
	};
	SoundFormat format;
};
