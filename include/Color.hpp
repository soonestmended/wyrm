#pragma once

#include <iostream>
#include <glm/vec3.hpp>

#include "common.hpp"

class Color : public Vec3 {
	public:
		Color() : Vec3 (0.0) {}
		Color(Real x_) : Vec3(x_) {}
		Color(Real r_, Real g_, Real b_) : Vec3(r_, g_, b_) {}
		Color(const Vec3& foo) : Vec3(foo) {}
		Color(const Color& foo) : Vec3(foo.x, foo.y, foo.z) {}

		bool isBlack() const {return !(x > 0.0f || y > 0.0f || z > 0.0f);}
		Real luminance() const {return 0.2126*x + 0.7152*y + 0.0722*z;}
		static Color Black() {return Color(0.0, 0.0, 0.0);}
		static Color Blue() {return Color(0.0, 0.0, 1.0);}
		static Color Green() {return Color(0.0, 1.0, 0.0);}
		static Color Grey() {return Color(0.5, 0.5, 0.5);}
		static Color Red() {return Color(1.0, 0.0, 0.0);}
		static Color White() {return Color(1.0, 1.0, 1.0);}

		static Color abs(const Color& c) {return Color{std::abs(c.x), std::abs(c.y), std::abs(c.z)};}

		bool allComponentsLessThanEqualTo(const Color& c) const {
			return (x <= c.x && y <= c.y && z <= c.z);
		}

		Color& operator+=(const Color& c) {
			x += c.x;
			y += c.y;
		z += c.z;
		return *this;
	}

	Color& operator-=(const Color& c) {
		x -= c.x;
		y -= c.y;
		z -= c.z;
		return *this;
	}

	Color& operator*=(const Color& c) {
		x *= c.x;
		y *= c.y;
		z *= c.z;
		return *this;
	}

	Color& operator*=(const Real f) {
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	Color& operator/=(const Color& c) {
		x /= c.x;
		y /= c.y;
		z /= c.z;
		return *this;
	}

	Color& operator/=(const Real f) {
		x /= f;
		y /= f;
		z /= f;
		return *this;
	}

	friend Color operator+(const Color& lhs, const Color& rhs) {
		Color ans = lhs;
		ans += rhs;
		return ans;
	}

	friend Color operator-(const Color& lhs, const Color& rhs) {
		Color ans = lhs;
		ans -= rhs;
		return ans;
	}

	friend Color operator*(const Color& lhs, const Color& rhs) {
		return Color(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
	}

	friend Color operator*(const Color& lhs, const Real rhs) {
		Color ans = lhs;
		ans *= rhs;
		return ans;
	}

	friend Color operator*(const Real lhs, const Color& rhs) {
		Color ans = rhs;
		ans *= lhs;
		return ans;
	}

	friend Color operator/(const Color& lhs, const Color& rhs) {
		Color ans = lhs;
		ans /= rhs;
		return ans;
	}

	friend Color operator/(const Color& lhs, const Real rhs) {
		Color ans = lhs;
		ans /= rhs;
		return ans;
	}

	friend std::ostream& operator<<(std::ostream& os, const Color& c) {
		os << "<" << c.x << ", " << c.y << ", " << c.z << ">";
		return os;
	}
};
