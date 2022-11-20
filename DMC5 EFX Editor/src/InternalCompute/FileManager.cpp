#pragma once
#include "FileManager.h"
#include "efxbullshit.hpp"

//constructor
FileManager::FileManager(std::string path) {
	this->path = path;
}

const char* FileManager::getPath() {
	return path.c_str();
}

EFX_Header& FileManager::getHeader() {
	return header;
}

bool readEFX(std::ifstream&, FileManager&);

bool FileManager::openAndReadFile() {
	std::ifstream filestream(path, std::ifstream::binary);
	if (filestream) {
		bool succ = readEFX(filestream, *this);
		if (!succ) {
			return false;
		}
		//read remaining data
		while (filestream.peek() && !filestream.eof()) {
			unsigned char tmpdata{ 0 };
			filestream.read((char*)&tmpdata, 1);
			aftEfxData.emplace_back(tmpdata);
		}
	}
	return true;
}

//helper for open file
bool readEFX(std::ifstream& filestream, FileManager& manager) {
	auto fileStartAddr = filestream.tellg();
	//read header
	filestream.read(manager.getHeader().magic, 4);
	char ukn[5] = {};
	filestream.read((char*)&manager.getHeader().ukn, 4);
	filestream.read((char*)&manager.getHeader().effectCount, 4);
	filestream.read((char*)&manager.getHeader().nameBufferSize, 4);
	filestream.read((char*)&manager.getHeader().linkedEfxCount, 4);
	filestream.read((char*)&manager.getHeader().msk4Count, 4);
	filestream.read((char*)&manager.getHeader().modifierCount, 4);
	filestream.read((char*)&manager.getHeader().cndBlockCount, 4);
	filestream.read((char*)&manager.getHeader().cndBuffSize, 4);
	//move ptr to effects section
	//skip namebuffer
	filestream.seekg(fileStartAddr + std::streampos(36 + manager.getHeader().nameBufferSize + (manager.getHeader().modifierCount * 24)));
	//move beyond linked efx if exists
	for (int i = 0; i < manager.getHeader().linkedEfxCount; i++) {
		//todo change this
		//skip 20 byte
		filestream.seekg(filestream.tellg() + std::streampos(20));
		uint32_t pathsize{ 0 };
		filestream.read((char*)&pathsize, 4);
		//check for efxr
		auto pathsizeendpos = filestream.tellg();
		char isefxr[5] = {};
		filestream.read(isefxr, 4);
		if (std::strcmp(isefxr, "efxr") == 0) {
			FileManager tmp{""};
			filestream.seekg(pathsizeendpos);
			readEFX(filestream, tmp);
		}
		else {
			// skip to end if linked efx
			filestream.seekg(pathsizeendpos + std::streampos(pathsize * 2));
		}
	}
	//move beyond msk4
	for (int i = 0; i < manager.getHeader().msk4Count; i++) {
		//skip 44 byte
		filestream.seekg(filestream.tellg() + std::streampos(44));
		uint32_t pathsize{ 0 };
		filestream.read((char*)&pathsize, 4);
		//skip to end if linked efx
		filestream.seekg(filestream.tellg() + std::streampos(pathsize * 2));
	}
	//effects section
	std::streampos effectsStartAddr = filestream.tellg();
	//read before data
	filestream.seekg(fileStartAddr);
	while (filestream.tellg() < effectsStartAddr) {
		unsigned char tmpdata{ 0 };
		filestream.read((char*)&tmpdata, 1);
		manager.getBefEfxData().emplace_back(tmpdata);
	}
	for (int32_t i = 0; i < manager.getHeader().effectCount; i++) {
		//init
		Effect effect;
		effect.id = i;
		//read start data
		std::streampos effstart = filestream.tellg();
		filestream.read((char*)effect.dataBefSeg, 16);
		filestream.seekg(effstart);
		//read info from file
		uint32_t segCount{ 0 };
		filestream.seekg(filestream.tellg() + std::streampos(4));
		uint32_t namehash{ 0 };
		filestream.read((char*)&namehash, 4);
		std::wstring ws{ eff_name_utf8_hash_decode(namehash) };
		effect.name = std::string{ ws.begin(),ws.end() };
		filestream.seekg(effstart + std::streampos(12));
		filestream.read((char*)&segCount, 4);
		//segments overall size
		uint32_t fullSize{ 0 };
		for (int32_t j = 0; j < segCount; j++) {
			std::streampos segstart = filestream.tellg();
			uint32_t itemType{ 0 };
			uint32_t segSize{ 0 };
			filestream.read((char*)&itemType, 4);
			segSize = manager.getSegmentSize(itemType, filestream.tellg(), filestream);
			//if unsupported segments found
			if (segSize == 0) {
				return false;
			}
			//segment
			Segment segment;
			segment.id = j;
			segment.segName = manager.getSName(itemType);
			segment.size = segSize;
			fullSize += segSize;
			//insert data
			filestream.seekg(segstart);
			for (int c = 0; c < segSize; c++) {
				unsigned char tmpc{ 0 };
				filestream.read((char*)&tmpc, 1);
				segment.segData.emplace_back(tmpc);
			}
			//insert into map
			effect.segments.insert(std::make_pair(j, segment));
			//move fileptr to start of next seg
			filestream.seekg(segstart + std::streampos(segSize));
		}
		effect.size = fullSize + 16;
		manager.getEffects().emplace(std::make_pair(i, effect));
	}
	return true;
}

void FileManager::saveFile() {
	std::ofstream filestream(path, std::ofstream::binary | std::ofstream::trunc);
	if (filestream) {
		std::streampos headerstartpos = filestream.tellp();
		//before
		for (auto uchar : befEfxData) {
			filestream.write((char*)&uchar, 1);
		}
		//effects
		for (const auto& effectpair : effects) {
			//array
			filestream.write((char*)effectpair.second.dataBefSeg, 16);
			//segments
			for (const auto& segpair : effectpair.second.segments) {
				for (auto schar : segpair.second.segData) {
					filestream.write((char*)&schar, 1);
				}
			}
		}
		//after
		for (auto uchar : aftEfxData) {
			filestream.write((char*)&uchar, 1);
		}
		//update effect count in header
		filestream.seekp(headerstartpos + std::streampos(8));
		filestream.write((char*)&header.effectCount, 4);
	}
}

std::unordered_map<uint32_t, Effect>& FileManager::getEffects() {
	return effects;
}

std::vector<unsigned char>& FileManager::getBefEfxData() {
	return befEfxData;
}


uint32_t FileManager::getSegmentSize(uint32_t id, std::streampos segstart, std::ifstream& filestream) {
	switch (id) {
	case ItemType_Unknown:
		return 0;
		break;
	case ItemType_Spawn:
		return 44;
		break;
	case ItemType_SpawnExpression: 
	{
		filestream.seekg(segstart + std::streampos(24));
		uint32_t secPSize{ 0 };
		filestream.read((char*)&secPSize, 4);
		return 4 + 24 + 4 + secPSize;
		break;
	}
	case ItemType_Transform2D:
		return 28;
		break;
	case ItemType_Transform2DModifier:
		return 0;
		break;
	case ItemType_Transform2DClip:
		return 0;
		break;
	case ItemType_Transform2DExpression:
		return 0;
		break;
	case ItemType_Transform3D:
		return 48;
		break;
	case ItemType_Transform3DModifier:
		return 228;
		break;
	case ItemType_Transform3DClip: 
	{
		filestream.seekg(segstart + std::streampos(32));
		uint32_t part2Size{ 0 };
		uint32_t part3Size{ 0 };
		uint32_t part4Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		filestream.read((char*)&part3Size, 4);
		filestream.read((char*)&part4Size, 4);
		return 4 + 32 + 12 + part2Size + part3Size + part4Size;
		break;
	}		
	case ItemType_Transform3DExpression: 
	{
		filestream.seekg(segstart + std::streampos(44));
		uint32_t secChunkSize{ 0 };
		filestream.read((char*)&secChunkSize, 4);
		return 4 + 44 + 4 + secChunkSize;
		break;
	}	
	case ItemType_ParentOptions: 
	{
		filestream.seekg(segstart + std::streampos(44));
		uint32_t boneNameSize{ 0 };
		filestream.read((char*)&boneNameSize, 4);
		return 4 + 44 + 4 + boneNameSize;
		break;
	}		
	case ItemType_FixRandomGenerator:
		return 28;
		break;
	case ItemType_TypeBillboard2D:
		return 72;
		break;
	case ItemType_TypeBillboard2DExpression:
		return 0;
		break;
	case ItemType_TypeBillboard3D:
		return 80;
		break;
	case ItemType_TypeBillboard3DExpression: 
	{
		filestream.seekg(segstart + std::streampos(40));
		uint32_t part2Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		return 4 + 40 + 4 + part2Size;
		break;
	}	
	case ItemType_TypeMesh:
	{
		filestream.seekg(segstart + std::streampos(8));
		uint32_t hashCount{ 0 };
		filestream.read((char*)&hashCount, 4);
		filestream.seekg(filestream.tellg() + std::streampos(112));
		uint32_t meshPSize{ 0 };
		uint32_t mdfPSize{ 0 };
		uint32_t texPSize{ 0 };
		filestream.read((char*)&meshPSize, 4);
		filestream.read((char*)&mdfPSize, 4);
		filestream.read((char*)&texPSize, 4);
		return 4 + 136 + (hashCount * 28) + (meshPSize * 2) + (mdfPSize * 2) + (texPSize * 2);
		break;
	}
	case ItemType_TypeMeshClip: 
	{
		filestream.seekg(segstart + std::streampos(8));
		uint32_t part2Size{ 0 };
		uint32_t part3Size{ 0 };
		uint32_t part4Size{ 0 };
		uint32_t part5Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		filestream.seekg(filestream.tellg()  + std::streampos(20));
		filestream.read((char*)&part3Size, 4);
		filestream.read((char*)&part4Size, 4);
		filestream.read((char*)&part5Size, 4);
		return 4 + 8 + 16 + 20 + (part2Size * 16) + part3Size + part4Size + part5Size;
		break;
	}	
	case ItemType_TypeMeshExpression: 
	{
		filestream.seekg(segstart + std::streampos(8));
		uint32_t part2Size{ 0 };
		uint32_t part3Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		filestream.seekg(filestream.tellg() + std::streampos(88));
		filestream.read((char*)&part3Size, 4);
		return 4 + 104 + part2Size + part3Size;
		break;
	}		
	case ItemType_TypeRibbonFollow:
		return 112;
		break;
	case ItemType_TypeRibbonLength:
		return 152;
		break;
	case ItemType_TypeRibbonChain:
		return 212;
		break;
	case ItemType_TypeRibbonFixEnd:
		return 124;
		break;
	case ItemType_TypeRibbonLightweight:
		return 0x38;
		break;
	case ItemType_TypeRibbonFollowExpression: 
	{
		filestream.seekg(segstart + std::streampos(36));
		uint32_t part2Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		return 4 + 36 + 4 + part2Size;
		break;
	}	
	case ItemType_TypeRibbonLengthExpression:
	{
		filestream.seekg(segstart + std::streampos(36));
		uint32_t part2Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		return 4 + 36 + 4 + part2Size;
		break;
	}	
	case ItemType_TypeRibbonChainExpression:
		return 0;
		break;
	case ItemType_TypeRibbonFixEndExpression: 
	{
		filestream.seekg(segstart + std::streampos(36));
		uint32_t uknSize{ 0 };
		filestream.read((char*)&uknSize, 4);
		return 4 + 36 + 4 + uknSize;
		break;
	}		
	case ItemType_TypePolygon:
		return 96;
		break;
	case ItemType_TypePolygonClip:
	{
		filestream.seekg(segstart + std::streampos(28));
		uint32_t part2Size{ 0 };
		uint32_t part3Size{ 0 };
		uint32_t part4Size{ 0 };
		uint32_t part5Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		filestream.read((char*)&part3Size, 4);
		filestream.read((char*)&part4Size, 4);
		filestream.read((char*)&part5Size, 4);
		return 4 + 28 + 16 + part2Size + part3Size + part4Size + part5Size;
		break;
	}	
	case ItemType_TypePolygonExpression:
	{
		filestream.seekg(segstart + std::streampos(80));
		uint32_t secondPSize{ 0 };
		filestream.read((char*)&secondPSize, 4);
		return 4 + 80 + 4 + secondPSize;
		break;
	}
	case ItemType_TypeRibbonTrail:
		return 0x34;
		break;
	case ItemType_TypePolygonTrail:
		return 88;
		break;
	case ItemType_TypeNoDraw:
		return 72;
		break;
	case ItemType_TypeNoDrawExpression:
	{
		filestream.seekg(segstart + std::streampos(64));
		uint32_t part2Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		return 4 + 64 + 4 + part2Size;
		break;
	}
	case ItemType_Velocity2D:
		return 60;
		break;
	case ItemType_Velocity2DExpression:
		return 0;
		break;
	case ItemType_Velocity3D:
		return 104;
		break;
	case ItemType_Velocity3DExpression:
	{
		filestream.seekg(segstart + std::streampos(56));
		uint32_t part2Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		return 4 + 56 + 4 + part2Size;
		break;
	}
		
	case ItemType_RotateAnim:
		return 60;
		break;
	case ItemType_RotateAnimExpression:
	{
		filestream.seekg(segstart + std::streampos(56));
		uint32_t part2Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		return 4 + 56 + 4 + part2Size;
		break;
	}
	case ItemType_ScaleAnim:
		return 72;
		break;
	case ItemType_ScaleAnimExpression:
	{
		filestream.seekg(segstart + std::streampos(16));
		uint32_t part2Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		return 4 + 16 + 4 + part2Size;
		break;
	}
	case ItemType_Life:
		return 44;
		break;
	case ItemType_LifeExpression:
	{
		filestream.seekg(segstart + std::streampos(20));
		uint32_t part2Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		return 4 + 20 + 4 + part2Size;
		break;
	}
	case ItemType_UVSequence: 
	{
		filestream.seekg(segstart + std::streampos(32));
		uint32_t pathsize{ 0 };
		filestream.read((char*)&pathsize, 4);
		return 4 + 32 + 4 + (pathsize * 2);
		break;
	}
	case ItemType_UVSequenceExpression:
	{
		filestream.seekg(segstart + std::streampos(32));
		uint32_t secondPSize{ 0 };
		filestream.read((char*)&secondPSize, 4);
		return 4 + 32 + 4 + secondPSize;
		break;
	}
	case ItemType_UVScroll:
		return 0;
		break;
	case ItemType_TextureUnit:
	{
		filestream.seekg(segstart + std::streampos(476));
		uint32_t part2Size{ 0 };
		uint32_t part3Size{ 0 };
		uint32_t part4Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		filestream.read((char*)&part3Size, 4);
		filestream.read((char*)&part4Size, 4);
		return 4 + 476 + 12 + (part2Size * 2) + (part3Size * 2) + (part4Size * 2);
		break;
	}
	case ItemType_EmitterShape2D:
		return 40;
		break;
	case ItemType_EmitterShape2DExpression:
		return 0;
		break;
	case ItemType_EmitterShape3D:
		return 80;
		break;
	case ItemType_EmitterShape3DExpression: 
	{
		filestream.seekg(segstart + std::streampos(36));
		uint32_t secondPSize{ 0 };
		filestream.read((char*)&secondPSize, 4);
		return 4 + 36 + 4 + secondPSize;
		break;
	}		
	case ItemType_AlphaCorrection:
		return 24;
		break;
	case ItemType_TypeStrainRibbon:
	{
		filestream.seekg(segstart + std::streampos(164));
		uint32_t boneSize{ 0 };
		filestream.read((char*)&boneSize, 4);
		return 4 + 164 + 4 + boneSize;
		break;
	}
	case ItemType_TypeStrainRibbonExpression:
	{
		filestream.seekg(segstart + std::streampos(60));
		uint32_t secondPSize{ 0 };
		filestream.read((char*)&secondPSize, 4);
		return 4 + 60 + 4 + secondPSize;
		break;
	}
	case ItemType_TypeLightning3D:
		return 250;
		break;
	case ItemType_ShaderSettings:
		return 84;
		break;
	case ItemType_ShaderSettingsExpression:
	{
		filestream.seekg(segstart + std::streampos(12));
		uint32_t secondPSize{ 0 };
		filestream.read((char*)&secondPSize, 4);
		return 4 + 12 + 4 + secondPSize;
		break;
	}
	case ItemType_Distortion:
		return 16;
		break;
	case ItemType_RenderTarget:
	{
		filestream.seekg(segstart + std::streampos(8));
		uint32_t secondPSize{ 0 };
		filestream.read((char*)&secondPSize, 4);
		return 4 + 8 + 4 + (secondPSize * 2);
		break;
	}
	case ItemType_PtLife:
		return 20;
		break;
	case ItemType_PtBehavior: 
	{
		filestream.seekg(segstart + std::streampos(8));
		uint32_t scrSize{ 0 };
		uint32_t varCount{ 0 };
		filestream.read((char*)&scrSize, 4);
		filestream.read((char*)&varCount, 4);
		//get size of all var
		uint32_t varSize{ 0 };
		for (int i = 0; i < varCount; i++) {
			filestream.seekg(segstart + std::streampos(16 + scrSize + varSize));
			uint32_t tmp{ 0 };
			filestream.read((char*)&tmp, 4);
			tmp += 4;
			varSize += tmp;
		}
		return 4 + 16 + scrSize + varSize;
		break;
	}
	case ItemType_PtBehaviorClip:
		return 0;
		break;
	case ItemType_PlayEfx:
		return 0;
		break;
	case ItemType_FadeByAngle:
		return 36;
		break;
	case ItemType_FadeByAngleExpression:
	{
		filestream.seekg(segstart + std::streampos(16));
		uint32_t secondPSize{ 0 };
		filestream.read((char*)&secondPSize, 4);
		return 4 + 16 + 4 + secondPSize;
		break;
	}
	case ItemType_FadeByEmitterAngle:
		return 0x20;
		break;
	case ItemType_FadeByDepth:
		return 24;
		break;
	case ItemType_FadeByDepthExpression:
	{
		filestream.seekg(segstart + std::streampos(24));
		uint32_t secondPSize{ 0 };
		filestream.read((char*)&secondPSize, 4);
		return 4 + 24 + 4 + secondPSize;
		break;
	}
	case ItemType_FadeByOcclusion:
		return 24;
		break;
	case ItemType_FadeByOcclusionExpression:
		return 0;
		break;
	case ItemType_FakeDoF:
		return 0x1c;
		break;
	case ItemType_LuminanceBleed:
		return 0x20;
		break;
	case ItemType_TypeNodeBillboard:
		return 0xf8;
		break;
	case ItemType_TypeNodeBillboardExpression:
	{
		filestream.seekg(segstart + std::streampos(180));
		uint32_t secondPSize{ 0 };
		filestream.read((char*)&secondPSize, 4);
		return 4 + 180 + 4 + secondPSize;
		break;
	}
	case ItemType_UnitCulling:
		return 48;
		break;
	case ItemType_FluidEmitter2D:
		return 0x28;
		break;
	case ItemType_FluidSimulator2D:
	{
		filestream.seekg(segstart + std::streampos(188));
		uint32_t part1Size{ 0 };
		uint32_t part2Size{ 0 };
		uint32_t part3Size{ 0 };
		uint32_t part4Size{ 0 };
		uint32_t part5Size{ 0 };
		uint32_t part6Size{ 0 };
		filestream.read((char*)&part1Size, 4);
		filestream.read((char*)&part2Size, 4);
		filestream.read((char*)&part3Size, 4);
		filestream.read((char*)&part4Size, 4);
		filestream.read((char*)&part5Size, 4);
		filestream.read((char*)&part6Size, 4);
		return 4 + 188 + 24 + part1Size + part2Size + part3Size + part4Size + part5Size + part6Size;
		break;
	}
	case ItemType_PlayEmitter:
		return 0;
		break;
	case ItemType_PtTransform3D:
		return 44;
		break;
	case ItemType_PtTransform3DClip:
	{
		filestream.seekg(segstart + std::streampos(32));
		uint32_t part2Size{ 0 };
		uint32_t part3Size{ 0 };
		uint32_t part4Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		filestream.read((char*)&part3Size, 4);
		filestream.read((char*)&part4Size, 4);
		return 4 + 32 + 12 + part2Size + part3Size + part4Size;
		break;
	}
	case ItemType_PtTransform2D:
		return 0x1c;
		break;
	case ItemType_PtTransform2DClip:
		return 0;
		break;
	case ItemType_PtVelocity3D:
		return 0x18;
		break;
	case ItemType_PtVelocity3DClip:
	{
		filestream.seekg(segstart + std::streampos(32));
		uint32_t part2Size{ 0 };
		uint32_t part3Size{ 0 };
		uint32_t part4Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		filestream.read((char*)&part3Size, 4);
		filestream.read((char*)&part4Size, 4);
		return 4 + 32 + 12 + part2Size + part3Size + part4Size;
		break;
	}
	case ItemType_PtVelocity2D:
		return 0;
		break;
	case ItemType_PtVelocity2DClip:
		return 0;
		break;
	case ItemType_PtColliderAction:
		return 24;
		break;
	case ItemType_PtCollision:
		return 64;
		break;
	case ItemType_PtColor:
		return 20;
		break;
	case ItemType_PtColorClip:
	{
		filestream.seekg(segstart + std::streampos(32));
		uint32_t part2Size{ 0 };
		uint32_t part3Size{ 0 };
		uint32_t part4Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		filestream.read((char*)&part3Size, 4);
		filestream.read((char*)&part4Size, 4);
		return 4 + 32 + 12 + part2Size + part3Size + part4Size;
		break;
	}
	case ItemType_PtUvSequence:
		return 24;
		break;
	case ItemType_PtUvSequenceClip:
	{
		filestream.seekg(segstart + std::streampos(32));
		uint32_t part2Size{ 0 };
		uint32_t part3Size{ 0 };
		uint32_t part4Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		filestream.read((char*)&part3Size, 4);
		filestream.read((char*)&part4Size, 4);
		return 4 + 32 + 12 + part2Size + part3Size + part4Size;
		break;
	}
	case ItemType_MeshEmitter:
	{
		filestream.seekg(segstart + std::streampos(84));
		uint32_t mapPathSize{ 0 };
		uint32_t meshPathSize{ 0 };
		uint32_t mdfPathSize{ 0 };
		uint32_t mskPathSize{ 0 };
		filestream.read((char*)&mapPathSize, 4);
		filestream.read((char*)&meshPathSize, 4);
		filestream.read((char*)&mdfPathSize, 4);
		filestream.read((char*)&mskPathSize, 4);
		return 4 + 84 + 16 + (mapPathSize * 2) + (meshPathSize * 2) + (mdfPathSize * 2) + (mskPathSize * 2);
		break;
	}
	case ItemType_MeshEmitterClip:
	{
		filestream.seekg(segstart + std::streampos(32));
		uint32_t part2Size{ 0 };
		uint32_t part3Size{ 0 };
		uint32_t part4Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		filestream.read((char*)&part3Size, 4);
		filestream.read((char*)&part4Size, 4);
		return 4 + 32 + 12 + part2Size + part3Size + part4Size;
		break;
	}
	case ItemType_MeshEmitterExpression:
	{
		filestream.seekg(segstart + std::streampos(72));
		uint32_t secondPSize{ 0 };
		filestream.read((char*)&secondPSize, 4);
		return 4 + 72 + 4 + secondPSize;
		break;
	}
	case ItemType_VectorFieldParameter:
		return 76;
		break;
	case ItemType_VectorFieldParameterClip:
	{
		filestream.seekg(segstart + std::streampos(32));
		uint32_t part2Size{ 0 };
		uint32_t part3Size{ 0 };
		uint32_t part4Size{ 0 };
		filestream.read((char*)&part2Size, 4);
		filestream.read((char*)&part3Size, 4);
		filestream.read((char*)&part4Size, 4);
		return 4 + 32 + 12 + part2Size + part3Size + part4Size;
		break;
	}
	case ItemType_VectorFieldParameterExpression:
	{
		filestream.seekg(segstart + std::streampos(52));
		uint32_t secondPSize{ 0 };
		filestream.read((char*)&secondPSize, 4);
		return 4 + 52 + 4 + secondPSize;
		break;
	}
	case ItemType_DepthOperator:
		return 20;
		break;
	case ItemType_ShapeOperator:
		return 0;
		break;
	case ItemType_ShapeOperatorExpression:
		return 0;
		break;
	case ItemType_WindInfluence3D:
		return 0;
		break;
	case ItemType_TypeGpuBillboard:
		return 56;
		break;
	case ItemType_TypeGpuBillboardExpression:
	{
		filestream.seekg(segstart + std::streampos(36));
		uint32_t secondPSize{ 0 };
		filestream.read((char*)&secondPSize, 4);
		return 4 + 36 + 4 + secondPSize;
		break;
	}
	case ItemType_TypeGpuRibbonFollow:
		return 60;
		break;
	case ItemType_EmitterPriority:
		return 0x08;
		break;
	case ItemType_DrawOverlay:
		return 0x14;
		break;
	case ItemType_VectorField:
		return 0;
		break;
	case ItemType_VolumeField:
		return 0;
		break;
	case ItemType_AngularVelocity3D:
		return 88;
		break;
	case ItemType_PtAngularVelocity3D:
		return 0;
		break;
	case ItemType_PtAngularVelocity3DExpression:
		return 0;
		break;
	case ItemType_AngularVelocity2D:
		return 0x44;
		break;
	case ItemType_PtAngularVelocity2D:
		return 0;
		break;
	case ItemType_PtAngularVelocity2DExpression:
		return 0;
		break;
	case ItemType_IgnorePlayerColor:
		return 0x08;
		break;
	case ItemType_ProceduralDistortion:
		return 0x20;
		break;
	case ItemType_ProceduralDistortionClip:
		return 0;
		break;
	case ItemType_ItemNum:
		return 0;
		break;
	default:
		return 0;
	}
}

std::string FileManager::getSName(uint32_t id) {
	switch (id) {
	case ItemType_Unknown:
		return "Unknown";
		break;
	case ItemType_Spawn:
		return "Spawn";
		break;
	case ItemType_SpawnExpression:
		return "Spawn Expression";
		break;
	case ItemType_Transform2D:
		return "Transform 2D";
		break;
	case ItemType_Transform2DModifier:
		return "Transform 2D Modifier";
		break;
	case ItemType_Transform2DClip:
		return "Transform 2D Clip";
		break;
	case ItemType_Transform2DExpression:
		return "Transform 2D Expression";
		break;
	case ItemType_Transform3D:
		return "Transform 3D";
		break;
	case ItemType_Transform3DModifier:
		return "Transform 3D Modifier";
		break;
	case ItemType_Transform3DClip:
		return "Transform 3D Clip";
		break;
	case ItemType_Transform3DExpression:
		return "Transform 3D Expression";
		break;
	case ItemType_ParentOptions:
		return "Parent Options";
		break;
	case ItemType_FixRandomGenerator:
		return "Fix Random Generator";
		break;
	case ItemType_TypeBillboard2D:
		return "Type Billboard 2D";
		break;
	case ItemType_TypeBillboard2DExpression:
		return "Type Billboard 2D Expression";
		break;
	case ItemType_TypeBillboard3D:
		return "Type Billboard 3D";
		break;
	case ItemType_TypeBillboard3DExpression:
		return "Type Billboard 3D Expression";
		break;
	case ItemType_TypeMesh:
		return "Type Mesh";
		break;
	case ItemType_TypeMeshClip:
		return "Type Mesh Clip";
		break;
	case ItemType_TypeMeshExpression:
		return "Type Mesh Expression";
		break;
	case ItemType_TypeRibbonFollow:
		return "Type Ribbon Follow";
		break;
	case ItemType_TypeRibbonLength:
		return "Type Ribbon Length";
		break;
	case ItemType_TypeRibbonChain:
		return "Type Ribbon Chain";
		break;
	case ItemType_TypeRibbonFixEnd:
		return "Type Ribbon Fix End";
		break;
	case ItemType_TypeRibbonLightweight:
		return "Type Ribbon Lightweight";
		break;
	case ItemType_TypeRibbonFollowExpression:
		return "Type Ribbon Follow Expression";
		break;
	case ItemType_TypeRibbonLengthExpression:
		return "Type Ribbon Length Expression";
		break;
	case ItemType_TypeRibbonChainExpression:
		return "Type Ribbon Chain Expression";
		break;
	case ItemType_TypeRibbonFixEndExpression:
		return "Type Ribbon Fix End Expression";
		break;
	case ItemType_TypePolygon:
		return "Type Polygon";
		break;
	case ItemType_TypePolygonClip:
		return "Type Polygon Clip";
		break;
	case ItemType_TypePolygonExpression:
		return "Type Polygon Expression";
		break;
	case ItemType_TypeRibbonTrail:
		return "Type Ribbon Trail";
		break;
	case ItemType_TypePolygonTrail:
		return "Type Polygon Trail";
		break;
	case ItemType_TypeNoDraw:
		return "Type No Draw";
		break;
	case ItemType_TypeNoDrawExpression:
		return "Type No Draw Expression";
		break;
	case ItemType_Velocity2D:
		return "Velocity 2D";
		break;
	case ItemType_Velocity2DExpression:
		return "Velocity 2D Expression";
		break;
	case ItemType_Velocity3D:
		return "Velocity 3D";
		break;
	case ItemType_Velocity3DExpression:
		return "Velocity 3D Expression";
		break;
	case ItemType_RotateAnim:
		return "Rotate Anim";
		break;
	case ItemType_RotateAnimExpression:
		return "Rotate Anim Expression";
		break;
	case ItemType_ScaleAnim:
		return "Scale Anim";
		break;
	case ItemType_ScaleAnimExpression:
		return "Scale Anim Expression";
		break;
	case ItemType_Life:
		return "Life";
		break;
	case ItemType_LifeExpression:
		return "Life Expression";
		break;
	case ItemType_UVSequence:
		return "UV Sequence";
		break;
	case ItemType_UVSequenceExpression:
		return "UV Sequence Expression";
		break;
	case ItemType_UVScroll:
		return "UV Scroll";
		break;
	case ItemType_TextureUnit:
		return "Texture Unit";
		break;
	case ItemType_EmitterShape2D:
		return "Emitter Shape 2D";
		break;
	case ItemType_EmitterShape2DExpression:
		return "Emitter Shape 2D Expression";
		break;
	case ItemType_EmitterShape3D:
		return "Emitter Shape 3D";
		break;
	case ItemType_EmitterShape3DExpression:
		return "Emitter Shape 3D Expression";
		break;
	case ItemType_AlphaCorrection:
		return "Alpha Correction";
		break;
	case ItemType_TypeStrainRibbon:
		return "Type Strain Ribbon";
		break;
	case ItemType_TypeStrainRibbonExpression:
		return "Type Strain Ribbon Expression";
		break;
	case ItemType_TypeLightning3D:
		return "Type Lightning 3D";
		break;
	case ItemType_ShaderSettings:
		return "Shader Settings";
		break;
	case ItemType_ShaderSettingsExpression:
		return "Shader Settings Expression";
		break;
	case ItemType_Distortion:
		return "Distortion";
		break;
	case ItemType_RenderTarget:
		return "Render Target";
		break;
	case ItemType_PtLife:
		return "Pt Life";
		break;
	case ItemType_PtBehavior:
		return "Pt Behavior";
		break;
	case ItemType_PtBehaviorClip:
		return "Pt Behavior Clip";
		break;
	case ItemType_PlayEfx:
		return "Play Efx";
		break;
	case ItemType_FadeByAngle:
		return "Fade By Angle";
		break;
	case ItemType_FadeByAngleExpression:
		return "Fade By Angle Expression";
		break;
	case ItemType_FadeByEmitterAngle:
		return "Fade By Emitter Angle";
		break;
	case ItemType_FadeByDepth:
		return "Fade By Depth";
		break;
	case ItemType_FadeByDepthExpression:
		return "Fade By Depth Expression";
		break;
	case ItemType_FadeByOcclusion:
		return "Fade By Occlusion";
		break;
	case ItemType_FadeByOcclusionExpression:
		return "Fade By Occlusion Expression";
		break;
	case ItemType_FakeDoF:
		return "Fake Do F";
		break;
	case ItemType_LuminanceBleed:
		return "Luminance Bleed";
		break;
	case ItemType_TypeNodeBillboard:
		return "Type Node Billboard";
		break;
	case ItemType_TypeNodeBillboardExpression:
		return "Type Node Billboard Expression";
		break;
	case ItemType_UnitCulling:
		return "Unit Culling";
		break;
	case ItemType_FluidEmitter2D:
		return "Fluid Emitter 2D";
		break;
	case ItemType_FluidSimulator2D:
		return "Fluid Simulator 2D";
		break;
	case ItemType_PlayEmitter:
		return "Play Emitter";
		break;
	case ItemType_PtTransform3D:
		return "Pt Transform 3D";
		break;
	case ItemType_PtTransform3DClip:
		return "Pt Transform 3D Clip";
		break;
	case ItemType_PtTransform2D:
		return "Pt Transform 2D";
		break;
	case ItemType_PtTransform2DClip:
		return "Pt Transform 2D Clip";
		break;
	case ItemType_PtVelocity3D:
		return "Pt Velocity 3D";
		break;
	case ItemType_PtVelocity3DClip:
		return "Pt Velocity 3D Clip";
		break;
	case ItemType_PtVelocity2D:
		return "Pt Velocity 2D";
		break;
	case ItemType_PtVelocity2DClip:
		return "Pt Velocity 2D Clip";
		break;
	case ItemType_PtColliderAction:
		return "Pt Collider Action";
		break;
	case ItemType_PtCollision:
		return "Pt Collision";
		break;
	case ItemType_PtColor:
		return "Pt Color";
		break;
	case ItemType_PtColorClip:
		return "Pt Color Clip";
		break;
	case ItemType_PtUvSequence:
		return "Pt Uv Sequence";
		break;
	case ItemType_PtUvSequenceClip:
		return "Pt Uv Sequence Clip";
		break;
	case ItemType_MeshEmitter:
		return "Mesh Emitter";
		break;
	case ItemType_MeshEmitterClip:
		return "Mesh Emitter Clip";
		break;
	case ItemType_MeshEmitterExpression:
		return "Mesh Emitter Expression";
		break;
	case ItemType_VectorFieldParameter:
		return "Vector Field Parameter";
		break;
	case ItemType_VectorFieldParameterClip:
		return "Vector Field Parameter Clip";
		break;
	case ItemType_VectorFieldParameterExpression:
		return "Vector Field Parameter Expression";
		break;
	case ItemType_DepthOperator:
		return "Depth Operator";
		break;
	case ItemType_ShapeOperator:
		return "Shape Operator";
		break;
	case ItemType_ShapeOperatorExpression:
		return "Shape Operator Expression";
		break;
	case ItemType_WindInfluence3D:
		return "Wind Influence 3D";
		break;
	case ItemType_TypeGpuBillboard:
		return "Type Gpu Billboard";
		break;
	case ItemType_TypeGpuBillboardExpression:
		return "Type Gpu Billboard Expression";
		break;
	case ItemType_TypeGpuRibbonFollow:
		return "Type Gpu Ribbon Follow";
		break;
	case ItemType_EmitterPriority:
		return "Emitter Priority";
		break;
	case ItemType_DrawOverlay:
		return "Draw Overlay";
		break;
	case ItemType_VectorField:
		return "Vector Field";
		break;
	case ItemType_VolumeField:
		return "Volume Field";
		break;
	case ItemType_AngularVelocity3D:
		return "Angular Velocity 3D";
		break;
	case ItemType_PtAngularVelocity3D:
		return "Pt Angular Velocity 3D";
		break;
	case ItemType_PtAngularVelocity3DExpression:
		return "Pt Angular Velocity 3D Expression";
		break;
	case ItemType_AngularVelocity2D:
		return "Angular Velocity 2D";
		break;
	case ItemType_PtAngularVelocity2D:
		return "Pt Angular Velocity 2D";
		break;
	case ItemType_PtAngularVelocity2DExpression:
		return "Pt Angular Velocity 2D Expression";
		break;
	case ItemType_IgnorePlayerColor:
		return "Ignore Player Color";
		break;
	case ItemType_ProceduralDistortion:
		return "Procedural Distortion";
		break;
	case ItemType_ProceduralDistortionClip:
		return "Procedural Distortion Clip";
		break;
	case ItemType_ItemNum:
		return "Item Num";
		break;
	default:
		return "Unknown!";
	}
}


