#pragma once

#include "Color.hpp"
#include "Image.hpp"
#include "Utils.hpp"
#include <cmath>
#include <string>

enum TextureWrapSetting {CLAMP, REPEAT};

class Texture {
public:
	Texture() : wrap (CLAMP) {}
	virtual Color eval(Real u, Real v, Real w = 0) = 0;

	void setWrapClamp() {wrap = CLAMP;}
	void setWrapRepeat() {wrap = REPEAT;}
	TextureWrapSetting wrap;
};

class ImageTexture : public Texture {
public:	
	ImageTexture(const std::string& filename);
	Color eval(Real u, Real v, Real w = 0) {
		if (wrap == CLAMP) {
			u = utils::clamp(u, 0, 1);
			v = utils::clamp(v, 0, 1);
		}
		else if (wrap == REPEAT) {
			u = fmod(u, 1.0);
			v = fmod(v, 1.0);
		}
		int x = utils::clamp(u * image.width(), 0, image.width() - 1);
		int y = utils::clamp(v * image.height(), 0, image.height() - 1);
		return image(x, y);
	}

		Color eval(const Vec2& uv) {
			return eval(uv[0], uv[1]);
		}

	Image image;
};
