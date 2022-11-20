#pragma once
#include "FileDefs.h"
#include <fstream>
#include <unordered_map>


class FileManager {
	EFX_Header header{};
	std::string path;
	std::unordered_map<uint32_t, Effect> effects;
	std::vector<unsigned char> befEfxData;
	std::vector<unsigned char> aftEfxData;
public:
	//constructor
	FileManager(std::string path);
	//methods
	const char* getPath();
	EFX_Header& getHeader();
	bool openAndReadFile();
	uint32_t getSegmentSize(uint32_t id, std::streampos segstart, std::ifstream& filestream);
	std::string getSName(uint32_t id);
	std::unordered_map<uint32_t, Effect>& getEffects();
	std::vector<unsigned char>& getBefEfxData();
	void saveFile();
};

