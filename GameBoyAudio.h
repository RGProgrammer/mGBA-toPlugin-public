#pragma once
#include <stdint.h>
//const uint32_t AUDIO_BUFFER_SIZE = 2192;
//const uint32_t AUDIO_BUFFER_SIZE = 4096 ;
//const uint32_t AUDIO_BUFFER_SIZE = 2192 ;
const uint32_t AUDIO_BUFFER_SIZE_MULTIPLIER = 3;

struct mCore;

struct AudioData

{
	float* buffer = nullptr;
	uint32_t sizeInSamples = 0;
	uint32_t freqency;
	uint32_t numChannels = 2;
	uint32_t readCursorPosition = 0;
	uint32_t writeCursorPosition = 0;
};


const auto GBA_SAMPLE_RATE = 32768U;
//const auto GBA_SAMPLE_RATE = 44100U;
const auto GBA_NUM_CHANNELS = 2;
// Specify the cutoff frequency in Hertz
const float cutoffFrequency = 1000.0f; // Example cutoff frequency in Hertz

// Normalize the cutoff frequency
const float normalizedCutoff = cutoffFrequency / (GBA_SAMPLE_RATE / 2.0f);

// Calculate the filter coefficient
const float audioLowPassRange = 1.0f - normalizedCutoff;

 static bool useLowPassFilter = false;
 static float _audioLowPassLeftPrev = 0.0f;
 static float _audioLowPassRightPrev = 0.0f;


void audioLowPassFilter(float* buffer, uint32_t count);
uint32_t saveAudioData(mCore* core, AudioData& target);
uint32_t numAvailableSamples(AudioData source);
uint32_t readAudioSamlpes(AudioData& source, float* target, uint32_t count);
float readOneAudioSample(AudioData& source);
