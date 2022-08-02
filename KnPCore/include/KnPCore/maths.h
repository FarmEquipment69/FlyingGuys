#pragma once
#include "IEVector.hpp"

using namespace IEVector;

namespace KnP {
	namespace math
	{
		bool WorldToScreen(const v3& pos, v4& clipCoords, v4& NDC, v3& screen, float matrix[16], const int& windowWidth, const int& windowHeight);
	}
}