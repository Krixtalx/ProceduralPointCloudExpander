#pragma once
#include "RendererCore/PointCloud.h"

class PlyLoader
{
private: 
	static std::string PLY_EXTENSION;
public:
	static PPCX::PointCloud* cargarModelo(const std::string& _filename);
};