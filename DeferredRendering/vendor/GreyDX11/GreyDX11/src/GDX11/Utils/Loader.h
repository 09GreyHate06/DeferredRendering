#pragma once
#include <string>

namespace GDX11::Utils
{
	std::string LoadText(const std::string& filename);

	struct ImageData
	{
		uint8_t* pixels;
		int width;
		int height;
		int nrComponents;
	};

	ImageData LoadImageFile(const std::string& filename, bool flipImageY, int reqComponents);

	void FreeImageData(ImageData* data);
}