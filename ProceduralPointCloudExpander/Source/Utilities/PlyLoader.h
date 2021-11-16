#pragma once
#include "stdafx.h"
#include "RendererCore/pointCloud.h"

class PlyLoader
{
public:
	static PAG::pointCloud* cargarModelo(std::string _filename);
};