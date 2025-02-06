#include "GameBoyAudio.h"
#include <core/blip_buf.h>
#include <core/core.h>
#include <stdint.h>


#define MIN(a,b) (a<b)? a : b


void audioLowPassFilter(float* buffer, uint32_t count) {
	float* out = buffer;

	/* Restore previous samples */
	float audioLowPassLeft = _audioLowPassLeftPrev;
	float audioLowPassRight = _audioLowPassRightPrev;

	/* Calculate low-pass filter coefficients */
	const float factorA = 1.0f - audioLowPassRange;
	const float factorB = audioLowPassRange;

	for (uint32_t frame = 0; frame < count; ++frame) {
		/* Apply low-pass filter */
		audioLowPassLeft = (audioLowPassLeft * factorA) + (out[0] * factorB);
		audioLowPassRight = (audioLowPassRight * factorA) + (out[1] * factorB);

		/* Update sound buffer */
		out[0] = audioLowPassLeft;
		out[1] = audioLowPassRight;
		out += 2;
	}

	/* Save last samples for next frame */
	_audioLowPassLeftPrev = audioLowPassLeft;
	_audioLowPassRightPrev = audioLowPassRight;
}

uint32_t saveAudioData(mCore* core, AudioData& target) {
	if (core->platform(core) == mPLATFORM_GBA) {
		//try shifting data first
		if (target.readCursorPosition != 0) {
			memcpy(target.buffer,
				target.buffer + target.readCursorPosition,
				(target.writeCursorPosition - target.readCursorPosition) * sizeof(float));
			target.writeCursorPosition -= target.readCursorPosition;
			target.readCursorPosition = 0;
		}
		blip_t* audioChannelLeft = core->getAudioChannel(core, 0);
		blip_t* audioChannelRight = core->getAudioChannel(core, 1);
		uint32_t samplesAvail = blip_samples_avail(audioChannelLeft);
		if (samplesAvail > 0) {
			
			samplesAvail = MIN(samplesAvail, (target.sizeInSamples - target.writeCursorPosition) /2);
			if (samplesAvail >= 2) {
				samplesAvail = blip_read_samples_f(
					audioChannelLeft,
					target.buffer + target.writeCursorPosition,
					samplesAvail,
					true);

				blip_read_samples_f(
					audioChannelRight,
					target.buffer + target.writeCursorPosition + 1,
					samplesAvail,
					true);
				//audioLowPassFilter(target.buffer + target.writeCursorPosition, samplesAvail);
				target.writeCursorPosition += (samplesAvail*2);

				blip_clear(audioChannelLeft);
				blip_clear(audioChannelRight);
			}
		}
		return samplesAvail *2;
	}
	return 0;
}
uint32_t numAvailableSamples(AudioData source)
{
	return  source.writeCursorPosition - source.readCursorPosition;
}
uint32_t readAudioSamlpes(AudioData& source, float* target, uint32_t count)
{
	auto toRead = MIN(count, source.writeCursorPosition - source.readCursorPosition);
	if (toRead > 0 && target != nullptr) {
		memcpy(target, source.buffer + source.readCursorPosition, toRead * sizeof(float));
		source.readCursorPosition += toRead;	
		return toRead;
	}
	return 0;
}

float readOneAudioSample(AudioData& source)
{
	float value = 0.0f;
	if (source.readCursorPosition < source.writeCursorPosition ) {
		value = *(source.buffer+source.readCursorPosition);
		source.readCursorPosition++;
	}
	return value;
}
