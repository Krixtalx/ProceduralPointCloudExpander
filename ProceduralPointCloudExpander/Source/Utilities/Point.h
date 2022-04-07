#pragma once
#include "stdafx.h"

struct PointModel {
	vec3 _point;
	unsigned	_rgb;

	/**
	*	@return RGB color packed as a single unsigned value.
	*/
	static unsigned getRGBColor(const vec3& rgb) { return packUnorm4x8(vec4(rgb, .0f) / 255.0f); }

	/**
	*	@return RGB color packed as a single unsigned value.
	*/
	vec4 getRGBVec3() const { return unpackUnorm4x8(_rgb) * 255.0f; }

	/**
	*	@brief Packs an RGB color into an unsigned value.
	*/
	void saveRGB(const vec3& rgb) { _rgb = this->getRGBColor(rgb); }

};