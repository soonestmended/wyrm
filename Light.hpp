#pragma once

class Light {
public:
    float power;
    Light() : power (0.) {}
    Light(float _p) : power(_p) {}
};