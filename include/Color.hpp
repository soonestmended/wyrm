#pragma once

#include <iostream>
#include <glm/vec3.hpp>

class Color : public glm::vec3 {
public:
    Color() : glm::vec3 (0.0) {}
	Color(float x_) : glm::vec3(x_) {}
    Color(float r_, float g_, float b_) : glm::vec3(r_, g_, b_) {}
	Color(const glm::vec3& foo) : glm::vec3(foo) {}
	Color(const Color& foo) : glm::vec3(foo.x, foo.y, foo.z) {}

	bool isBlack() const {return !(x > 0.0f || y > 0.0f || z > 0.0f);}

    static Color Black() {return Color(0.0, 0.0, 0.0);}
	static Color Blue() {return Color(0.0, 0.0, 1.0);}
	static Color Green() {return Color(0.0, 1.0, 0.0);}
	static Color Red() {return Color(1.0, 0.0, 0.0);}
	static Color White() {return Color(1.0, 1.0, 1.0);}

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
		y *= c.z;
		z *= c.z;
		return *this;
	}

	Color& operator*=(const float f) {
		x *= f;
		y *= f;
		z *= f;
		return *this;
	}

	Color& operator/=(const Color& c) {
		x /= c.x;
		y /= c.z;
		z /= c.z;
		return *this;
	}

	Color& operator/=(const float f) {
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

	friend Color operator*(const Color& lhs, const float rhs) {
		Color ans = lhs;
		ans *= rhs;
		return ans;
	}

	friend Color operator*(const float lhs, const Color& rhs) {
		Color ans = rhs;
		ans *= lhs;
		return ans;
	}

	friend Color operator/(const Color& lhs, const Color& rhs) {
		Color ans = lhs;
		ans /= rhs;
		return ans;
	}

	friend Color operator/(const Color& lhs, const float rhs) {
		Color ans = lhs;
		ans /= rhs;
		return ans;
	}

	friend std::ostream& operator<<(std::ostream& os, const Color& c) {
		os << "<" << c.x << ", " << c.y << ", " << c.z << ">";
		return os;
	}
};