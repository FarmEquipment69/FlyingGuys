#include "maths.h"

namespace KnP {

	namespace math
	{
		bool WorldToScreen(const v3& pos, v4& clipCoords, v4& NDC, v3& screen, float matrix[16], const int& windowWidth, const int& windowHeight)
		{
			clipCoords.x = pos.x * matrix[0] + pos.y * matrix[4] + pos.z * matrix[8] + matrix[12];
			clipCoords.y = pos.x * matrix[1] + pos.y * matrix[5] + pos.z * matrix[9] + matrix[13];
			clipCoords.z = pos.x * matrix[2] + pos.y * matrix[6] + pos.z * matrix[10] + matrix[14];
			clipCoords.w = pos.x * matrix[3] + pos.y * matrix[7] + pos.z * matrix[11] + matrix[15];

			if (clipCoords.w < 0.1f)
			{
				return false;
			}

			NDC.x = clipCoords.x / clipCoords.w;
			NDC.y = clipCoords.y / clipCoords.w;
			NDC.z = clipCoords.z / clipCoords.w;

			screen.x = (windowWidth / 2 * NDC.x) + (NDC.x + windowWidth / 2);
			screen.y = -(windowHeight / 2 * NDC.y) + (NDC.y + windowHeight / 2);

			return true;
		}
	}
}