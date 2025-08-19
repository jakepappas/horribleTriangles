//
// Created by Jake Pappas on 8/19/2025.
//
#include "glm/glm.hpp"
#include "../util/color.h"
#include <vector>

#ifndef INC_8_BIT_TRIANGLE_H
#define INC_8_BIT_TRIANGLE_H

using std::vector, glm::mat4, glm::vec3, glm::vec2, glm::vec4;
class Triangle {
public:
    vec4 p1;
    vec4 p2;
    vec4 p3;
    mat4 projection;
    mat4 rotation;
    int screenWidth;
    int screenHeight;

    vec2 p2d1;
    vec2 p2d2;
    vec2 p2d3;

    vector<vec2> drawHere;

    Triangle(vec4 p1, vec4 p2, vec4 p3, mat4 projection, mat4 rotation, int screenWidth, int screenHeight);
    ~Triangle() = default;

    vec2 ApplyTransformations(vec4 p);
    void Render();
    void ColorPixel(int x, int y);

    void drawLine(vec2 p1, vec2 p2);
    void drawLines(vec2 p1, vec2 p2, vec2 p3);


};


#endif //INC_8_BIT_TRIANGLE_H
