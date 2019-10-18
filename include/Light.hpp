#pragma once

class Light {
public:
    float power;
    Light() : power (0.) {}
    Light(float p_) : power(p_) {}
};