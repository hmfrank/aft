#include "2009_DaPlSt/demo.h"
#include <cmath>
#include <Gamma/SoundFile.h>

using namespace gam;


int main(int argc, char **argv)
{
	return demo(argc, argv);

//	const double SAMPLE_RATE = 44100;
//	const double DURATION = 6;
//	const int CHANNELS = 5;
//	const double FREQYENCY = 440;
//
//
//	SoundFile file("example.wav");
//	file.openWrite();
//
//	file.frameRate(SAMPLE_RATE);
//	file.channels(CHANNELS);
//	file.format(SoundFile::Format::WAV);
//	file.encoding(SoundFile::EncodingType::PCM_16);
//
//	int n_frames = (int)round(SAMPLE_RATE * DURATION);
//	int n_samples = n_frames * CHANNELS;
//	float buffer[n_samples];
//
//	for (int frame = 0; frame < n_frames; ++frame)
//	{
//		for (int channel = 0; channel < CHANNELS; ++channel)
//		{
//			buffer[frame * CHANNELS + channel] = sinf((float)frame * 2.0f * M_PI / (float)SAMPLE_RATE * (float)FREQYENCY);
//		}
//	}
//
//	file.write(buffer, n_frames);
}
