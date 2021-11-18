#pragma once
#include "RendererCore/pointCloud.h"

class PlyLoader
{
private: 
	static std::string PLY_EXTENSION;
public:
	static PAG::pointCloud* cargarModelo(const std::string& _filename);
};