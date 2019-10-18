#pragma once

#include <glm/vec3.hpp>

class Color : public glm::vec3 {
public:
    Color() : glm::vec3 (0.0) {}
    Color(float r_, float g_, float b_) : glm::vec3(r_, g_, b_) {}

    static Color Black() {return Color(0.0, 0.0, 0.0);}
	static Color Blue() {return Color(0.0, 0.0, 1.0);}
	static Color Green() {return Color(0.0, 1.0, 0.0);}
	static Color Red() {return Color(1.0, 0.0, 0.0);}
	static Color White() {return Color(1.0, 1.0, 1.0);}
};