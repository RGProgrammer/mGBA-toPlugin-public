#pragma once 

#include "mgba/script.h"
#include <mgba/core/core.h>
#include <memory>


const uint32_t MAX_PATH_LENGTH = 80;

typedef enum KEYS {
    A = (1 << 0),
    B = (1 << 1),
    SELECT = (1 << 2),
    START = (1 << 3),
    RIGHT = (1 << 4),
    LEFT = (1 << 5),
    UP = (1 << 6),
    DOWN = (1 << 7),
    R = (1 << 8),
    L = (1 << 9),
} KEYS;


typedef struct EmuInfo
{
    mCore *core = nullptr;
    uint8_t* romData = nullptr;
    uint8_t* bios = nullptr;
    uint8_t savePath[MAX_PATH_LENGTH] = { "\0" };
    size_t romSize = 0;
    size_t biosSize = 0;
} EmuInfo;

typedef struct FramePixelData
{
    uint8_t* pixels = nullptr;
    int width = 0;
    int height = 0;
    int stride = 0;
} FramePixelData;

typedef struct AudioBufferData
{
    float* buffer = nullptr;
    uint32_t size = 0;
    uint32_t numSamples = 0;
} AudioBufferData ;