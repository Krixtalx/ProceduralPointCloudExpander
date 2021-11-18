#pragma once
#include "RendererCore/pointCloud.h"

class PlyLoader
{
private: 
	static std::string PLY_EXTENSION;
public:
	static PPCX::pointCloud* cargarModelo(const std::string& _filename);
};