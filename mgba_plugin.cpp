#include "mgba_plugin.h"
#include "EmuInstanceInfo.h"
#include "GameBoySave.h"
#include <core/blip_buf.h>



#define MIN(a,b) (a<b)? a : b
#define MAX_INSTANCES 20

const auto SCREEN_HEIGHT = 160;
const auto  SCREEN_WIDTH = 240;
constexpr auto SCREEN_RATIO = SCREEN_WIDTH / (float)SCREEN_HEIGHT;
const float frameTime = 1.0f/ 60.0f;



struct GBAEmu {
	EmuInfo info;
	FramePixelData frameData;
	AudioData audioData;
	float timer = 0;
};

static GBAEmu _instances[MAX_INSTANCES];


int createInstance(const uint8_t* rom,const int romSize,
	const uint8_t* bios, const int biosSize ,const char* savePath)
{
	struct VFile* vfrom = nullptr;
	struct VFile* vfbios = nullptr;

	int insertionIndex = -1;
	
	for (int index = 0; index < MAX_INSTANCES; ++index) {
		if (_instances[index].info.core == nullptr) {
			insertionIndex = index;
			break;
		}
	}
	if (insertionIndex == -1 || insertionIndex == MAX_INSTANCES) {
		return -1;
	}

	if (!rom || romSize == 0) {
		return -1;
	}
	_instances[insertionIndex].info.romData = (uint8_t *)malloc(romSize);
	_instances[insertionIndex].info.romSize = romSize;
	if (_instances[insertionIndex].info.romData != nullptr) {
		memcpy(_instances[insertionIndex].info.romData, rom, romSize);
	}
	else {
		releaseInstance(insertionIndex);
		return -1;
	}
	vfrom = VFileFromConstMemory(_instances[insertionIndex].info.romData,
		_instances[insertionIndex].info.romSize);
	if (bios && biosSize > 0) {
		_instances[insertionIndex].info.bios = (uint8_t*)malloc(biosSize);
		_instances[insertionIndex].info.biosSize = biosSize;
		if (_instances[insertionIndex].info.bios != nullptr) {
			memcpy(_instances[insertionIndex].info.bios, bios, biosSize);
		}
		vfbios = VFileFromConstMemory(_instances[insertionIndex].info.bios,
			_instances[insertionIndex].info.biosSize);
	}
	//Find systemType
	_instances[insertionIndex].info.core = mCoreFindVF(vfrom);
	if (!_instances[insertionIndex].info.core) {
		return -1;
	}
	//init
	mCoreInitConfig(_instances[insertionIndex].info.core, nullptr);
	_instances[insertionIndex].info.core->init(_instances[insertionIndex].info.core);

	//settings

	struct mCoreOptions opts {

		.skipBios = true,
		.useBios = true,
			//.rewindEnable= true,
			.fpsTarget = 60,
			.suspendScreensaver = false,
			.savegamePath = NULL,
			.savestatePath = NULL,
			.screenshotPath = NULL,
			.volume = 255,	

	};
	mCoreConfigLoadDefaults(&_instances[insertionIndex].info.core->config, &opts);
	mCoreLoadConfig(_instances[insertionIndex].info.core);

	_instances[insertionIndex].info.core->loadROM(_instances[insertionIndex].info.core, vfrom);
	if (_instances[insertionIndex].info.core->opts.useBios && _instances[insertionIndex].info.bios != nullptr) {
		_instances[insertionIndex].info.core->loadBIOS(_instances[insertionIndex].info.core, vfbios, 0);
	}

	//InitVideo 
	size_t screenSize = SCREEN_WIDTH * SCREEN_HEIGHT * BYTES_PER_PIXEL;

	//TODO delegate frameCreation to a helper
	if (_instances[insertionIndex].frameData.pixels == nullptr) {
		_instances[insertionIndex].frameData = {
			.pixels = (uint8_t*)malloc(screenSize),
			.width = SCREEN_WIDTH,
			.height = SCREEN_HEIGHT,
		};
		memset(_instances[insertionIndex].frameData.pixels,
			0xFF,
			screenSize);
	}

	_instances[insertionIndex].info.core->setVideoBuffer(
		_instances[insertionIndex].info.core,
		(color_t*)_instances[insertionIndex].frameData.pixels,
		_instances[insertionIndex].frameData.width);


	//init Audio
	
	uint32_t audioSamplesPerFrame =
		(uint32_t)((float)GBA_SAMPLE_RATE * 
			(float)_instances[insertionIndex].info.core->frameCycles(_instances[insertionIndex].info.core) /
			(float)_instances[insertionIndex].info.core->frequency(_instances[insertionIndex].info.core));
	auto audioSampleBufferSize = audioSamplesPerFrame ;
	_instances[insertionIndex].info.core->setAudioBufferSize(_instances[insertionIndex].info.core, audioSampleBufferSize);
	blip_set_rates(_instances[insertionIndex].info.core->getAudioChannel(_instances[insertionIndex].info.core, 0),
		_instances[insertionIndex].info.core->frequency(_instances[insertionIndex].info.core), GBA_SAMPLE_RATE);

	blip_set_rates(_instances[insertionIndex].info.core->getAudioChannel(_instances[insertionIndex].info.core, 1),
		_instances[insertionIndex].info.core->frequency(_instances[insertionIndex].info.core), GBA_SAMPLE_RATE);
	uint32_t size = _instances[insertionIndex].info.core->getAudioBufferSize(
		_instances[insertionIndex].info.core)*2 * AUDIO_BUFFER_SIZE_MULTIPLIER ;

	_instances[insertionIndex].audioData = {
		.buffer = (float*)malloc(size * sizeof(float)),
		.sizeInSamples = size,
		.freqency = GBA_SAMPLE_RATE,
		.numChannels = 2
	};
	_instances[insertionIndex].info.core->reset(_instances[insertionIndex].info.core);
	loadGameSave(_instances[insertionIndex].info,savePath);
	return insertionIndex;
}

int releaseInstance(int instanceID)
{
	if (instanceID < 0 || instanceID >= MAX_INSTANCES)
		return 0;
	if (_instances[instanceID].info.core) {
		mCoreConfigDeinit(&(_instances[instanceID].info.core)->config);
		_instances[instanceID].info.core->deinit(_instances[instanceID].info.core);
		_instances[instanceID].info.core = nullptr;
	}
	if (_instances[instanceID].info.romData) {
		free(_instances[instanceID].info.romData);
		_instances[instanceID].info.romData = nullptr;
		_instances[instanceID].info.romSize = 0;
	}
	if (_instances[instanceID].info.bios) {
		free(_instances[instanceID].info.bios);
		_instances[instanceID].info.bios = nullptr;
		_instances[instanceID].info.biosSize = 0;
	}

	if (_instances[instanceID].frameData.pixels) {
		free(_instances[instanceID].frameData.pixels);
		_instances[instanceID].frameData.pixels = nullptr;
		_instances[instanceID].frameData.width = 0;
		_instances[instanceID].frameData.height = 0;
	}
	if (_instances[instanceID].audioData.buffer) {
		free(_instances[instanceID].audioData.buffer);
		_instances[instanceID].audioData.buffer = nullptr;
		_instances[instanceID].audioData.sizeInSamples=0;
		_instances[instanceID].audioData.freqency = 0;
		_instances[instanceID].audioData.numChannels = 0;
		_instances[instanceID].audioData.writeCursorPosition = 0;
		_instances[instanceID].audioData.readCursorPosition = 0;
	}
	return 1;
}

void releaseAll()
{
	for (int index = 0; index < MAX_INSTANCES; ++index) {
		if (_instances[index].info.core != nullptr) {
			releaseInstance(index);
		}
	}

}

int reset(int instanceID)
{
	if (instanceID < 0 || instanceID >= MAX_INSTANCES)
		return 0;
	if (_instances[instanceID].info.core) {
		_instances[instanceID].info.core->reset(_instances[instanceID].info.core);
		return 1;
	}
	return 0;
}


uint32_t getNumSamplesAvailable(int instanceID)
{
	if (instanceID < 0 || instanceID >= MAX_INSTANCES)
		return 0;
	if (!_instances[instanceID].info.core)
		return 0;
	return  numAvailableSamples(_instances[instanceID].audioData);
}

uint32_t getCurrentFrameSoundSamples(int instanceID, uint32_t count, float* outBuffer)
{
	if (instanceID < 0 || instanceID >= MAX_INSTANCES)
		return 0;
	if (!_instances[instanceID].info.core || outBuffer == nullptr)
		return 0;
	uint32_t toRead = MIN(numAvailableSamples(_instances[instanceID].audioData), count);
	toRead = readAudioSamlpes(_instances[instanceID].audioData, outBuffer, toRead);
	return  toRead;
}

float getOneSample(int instanceID)
{
	if (instanceID < 0 || instanceID >= MAX_INSTANCES)
		return 0.0f;
	if (!_instances[instanceID].info.core)
		return 00.f;
	if (getNumSamplesAvailable(instanceID))
	{
		return readOneAudioSample(_instances[instanceID].audioData);
	}
	return 0.0f;
}

uint32_t getAudioBufferSizeInSamples(int instanceID)
{
	if (instanceID < 0 || instanceID >= MAX_INSTANCES)
		return 0;
	if (!_instances[instanceID].info.core)
		return 0;
	return _instances[instanceID].info.core->getAudioBufferSize(_instances[instanceID].info.core) *
		_instances[instanceID].audioData.numChannels;
}


uint32_t getAudioBufferSizeInBytes(int instanceID)
{
	return getAudioBufferSizeInSamples(instanceID) * sizeof(float);
}


void clearAudioData(int instanceID)
{
	if (instanceID < 0 || instanceID >= MAX_INSTANCES)
		return;
	if (!_instances[instanceID].info.core)
		return;
	if (_instances[instanceID].audioData.buffer) {
		memset(_instances[instanceID].audioData.buffer, 0xFF,
			_instances[instanceID].audioData.sizeInSamples * sizeof(float));
		_instances[instanceID].audioData.readCursorPosition = 0;
		_instances[instanceID].audioData.writeCursorPosition = 0;
	}
}

uint32_t getSampleRate(int instanceID)
{
	if (instanceID < 0 || instanceID >= MAX_INSTANCES)
		return 0;
	if (!_instances[instanceID].info.core)
		return 0;
	return _instances[instanceID].audioData.freqency;
}



int updateSingleFrame(int instanceID)
{
	if (instanceID < 0 || instanceID >= MAX_INSTANCES)
		return 0;
	if (!_instances[instanceID].info.core)
		return 0;
	_instances[instanceID].info.core->runFrame(_instances[instanceID].info.core);
	saveAudioData(_instances[instanceID].info.core , _instances[instanceID].audioData);
	/*if (useLowPassFilter) {
		_audioLowPassFilter(_instances[instanceID].audioData.buffer, 
			_instances[instanceID].audioData.numSamples);
	}*/
	
	return 1;
}

int updateFrames(int instanceID, float deltaTime)
{
	if (instanceID < 0 || instanceID >= MAX_INSTANCES)
		return 0;
	if (!_instances[instanceID].info.core)
		return 0;
	int numGenFrame = 0;
	_instances[instanceID].timer += deltaTime;
	while(_instances[instanceID].timer >= frameTime){
		_instances[instanceID].info.core->runFrame(_instances[instanceID].info.core); 
		saveAudioData(_instances[instanceID].info.core, _instances[instanceID].audioData);
		_instances[instanceID].timer -= frameTime;
		numGenFrame++;
	}
	/*if (useLowPassFilter) {
		_audioLowPassFilter(_instances[instanceID].audioData.buffer,
			_instances[instanceID].audioData.numSamples);
	}*/
	return numGenFrame ;

}


uint8_t* getCurrentFramePixels(int instanceID)
{
	if (instanceID < 0 || instanceID >= MAX_INSTANCES)
		return 0;
	if (!_instances[instanceID].info.core)
		return 0;
	return  _instances[instanceID].frameData.pixels;
}

 uint32_t getCurrentFramePixelsAt(int instanceID, uint32_t index)
{
	if (instanceID < 0 || instanceID >= MAX_INSTANCES)
		return 0;
	if (!_instances[instanceID].info.core)
		return 0;
	if (index >= SCREEN_WIDTH * SCREEN_HEIGHT) {
		return 0;
	}
	return ((uint32_t*)_instances[instanceID].frameData.pixels)[index];
}

 uint32_t getCurrentFramePixels2At(int instanceID, uint32_t row, uint32_t column,bool vertical)
{
	if (instanceID < 0 || instanceID >= MAX_INSTANCES)
		return 0;
	if (!_instances[instanceID].info.core)
		return 0;
	if (row >= SCREEN_WIDTH || column >= SCREEN_HEIGHT) {
		return 0;
	}
	if(vertical)
		return ((uint32_t*)_instances[instanceID].frameData.pixels)[column * SCREEN_HEIGHT + row];
	else
		return ((uint32_t*)_instances[instanceID].frameData.pixels)[row * SCREEN_WIDTH + column];

}


 int pushKeys(int instanceID, uint16_t  keys)
{
	if (instanceID < 0 || instanceID >= MAX_INSTANCES)
		return 0;
	if (!_instances[instanceID].info.core) {
		return 0;
	}
	if (keys > 512) {
		return 0;
	}
	_instances[instanceID].info.core->setKeys(_instances[instanceID].info.core, keys);
	return 1;
}

#if defined(_WIN32) || defined(_WIN64) 
//#include <Windows.h>
//BOOL APIENTRY DllMain(HMODULE hModule,
//	DWORD  ul_reason_for_call,
//	LPVOID lpReserved
//)
//{
//	switch (ul_reason_for_call)
//	{
//	case DLL_PROCESS_ATTACH:
//	case DLL_THREAD_ATTACH:
//		break;
//	case DLL_THREAD_DETACH:
//	case DLL_PROCESS_DETACH:
//		releaseAll();
//		break;
//	}
//	return TRUE;
//}

#elif defined(__linux__) || defined(ANDROID)

// This function is called when the shared object is loaded.
//__attribute__((constructor))
//void so_init() {
//	memset(_instances, 0x00, MAX_INSTANCES * sizeof(GBAEmu));
//}

// This function is called when the shared object is unloaded.
__attribute__((destructor))
void so_cleanup() {
	releaseAll();
}

#else
#error "Unknown platform"
#endif