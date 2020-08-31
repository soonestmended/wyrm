#pragma once

#include "Color.hpp"
#include "Image.hpp"
#include "Utils.hpp"
#include <cmath>
#include <string>

enum TextureWrapSetting {CLAMP, REPEAT};

template <typename T>
class Texture {
	public:
		Texture() : wrap (CLAMP) {}
		virtual T eval(Real u, Real v, Real w = 0) = 0;
		virtual T eval(const Vec2& uv) {
			return eval(uv[0], uv[1]);
		}
		virtual T eval(const Vec3& uvw) {
			return eval(uvw[0], uvw[1], uvw[2]);
		}

		void setWrapClamp() {wrap = CLAMP;}
		void setWrapRepeat() {wrap = REPEAT;}
		TextureWrapSetting wrap;
};

template <typename T>
class ConstantTexture : public Texture <T> {
	public:
		using Texture <T>::eval;
		ConstantTexture(const T& t) : value (t){}
		T eval(Real u, Real v, Real w = 0) {
			return value;
		}
		const T value;
};

class ImageTexture : public Texture <Color> {
	public:
		using Texture <Color>::eval;
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


	Image image;
};
