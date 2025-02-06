
#include "GameBoySave.h"
#include <mgba/internal/gba/gba.h>
#include <mgba/internal/gb/gb.h>
#include <stdio.h>

#ifdef _WIN32 
	#include <direct.h>
#else
	#include <sys/stat.h>
#endif

void create_directories_if_needed(const char* path) { //TODO add support for android and linux
#ifdef _WIN32
wchar_t buffer[MAX_PATH_LENGTH];
	mbstowcs(buffer, path, strnlen_s(path,MAX_PATH_LENGTH));
	for (wchar_t* p = buffer; *p; p++) {
		if (*p == L'/' || *p == L'\\') {
			*p = L'\0';
			_wmkdir(buffer);
			*p = L'\\';
		}
	}
	
#else
char buffer[MAX_PATH_LENGTH];
strncpy(buffer, path, MAX_PATH_LENGTH);
	for (char* p = buffer; *p; p++) {
		if (*p == '/' || *p == '\\') {
			*p = '\0';
			mkdir(buffer, 777);
			*p = '/';
		}
	}
	
#endif
}


bool loadGameSave(EmuInfo& instance, const char *savePath)
{
	char title[12];
	if (strnlen((const char *)instance.savePath, MAX_PATH_LENGTH) == 0) {
		instance.core->getGameTitle(instance.core, title);
		sprintf((char*)instance.savePath, 
			"%s/%s.sav",savePath == nullptr ? "./saves" : savePath ,
			title);
		create_directories_if_needed((char*)instance.savePath);
	}
	VFile* source = VFileOpen((char*)instance.savePath, O_CREAT | O_RDWR);
	if (source == nullptr)
		return false;

	return instance.core->loadSave(instance.core, source);
}

