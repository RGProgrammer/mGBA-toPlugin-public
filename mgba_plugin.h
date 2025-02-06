#pragma once 

#include "GameBoyAudio.h"
#include <stdint.h>


#if defined(_WIN32) || defined(_WIN64)
#ifdef mgba_plugin_EXPORTS
#define MGBA_PLUGIN_ENTRY __declspec(dllexport)
#else
#define MGBA_PLUGIN_ENTRY __declspec(dllimport)
#endif
#else
#define MGBA_PLUGIN_ENTRY
#endif


#ifdef __cplusplus
extern "C" {
#endif



	MGBA_PLUGIN_ENTRY int createInstance(const uint8_t* rom,const int romSize,
		const uint8_t* bios = nullptr,const int biosSize = 0, const char* saveGamePath = nullptr );
	MGBA_PLUGIN_ENTRY int releaseInstance(int instanceID);
	MGBA_PLUGIN_ENTRY void releaseAll();
	MGBA_PLUGIN_ENTRY int reset(int instanceID);

	MGBA_PLUGIN_ENTRY uint32_t getNumSamplesAvailable(int instanceID);
	MGBA_PLUGIN_ENTRY uint32_t getCurrentFrameSoundSamples(int instanceID, uint32_t count, float* outBuffer);
	MGBA_PLUGIN_ENTRY float getOneSample(int instanceID);
	MGBA_PLUGIN_ENTRY uint32_t getAudioBufferSizeInSamples(int instanceID);
	MGBA_PLUGIN_ENTRY uint32_t getAudioBufferSizeInBytes(int instanceID);
	MGBA_PLUGIN_ENTRY void clearAudioData(int instanceID);
	MGBA_PLUGIN_ENTRY uint32_t getSampleRate(int instanceID);

	MGBA_PLUGIN_ENTRY int updateSingleFrame(int instanceID);
	MGBA_PLUGIN_ENTRY int updateFrames(int instanceID, float deltaTime);
	MGBA_PLUGIN_ENTRY uint8_t* getCurrentFramePixels(int instanceID);
	MGBA_PLUGIN_ENTRY uint32_t getCurrentFramePixelsAt(int instanceID, uint32_t index);
	MGBA_PLUGIN_ENTRY uint32_t getCurrentFramePixels2At(int instanceID, uint32_t row,uint32_t column,bool vertical=false);

	MGBA_PLUGIN_ENTRY int pushKeys(int instanceID, uint16_t  keys =0);

#ifdef __cplusplus
}
#endif