//
// Created by Jake Pappas on 8/19/2025.
//

#include "Triangle.h"

Triangle::Triangle(vec4 p1, vec4 p2, vec4 p3, mat4 projection, mat4 rotation, int screenWidth, int screenHeight){
    this->p1 = p1;
    this->p2 = p2;
    this->p3 = p3;
    this->projection = projection;
    this->rotation = rotation;
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;

    this->p2d1 = ApplyTransformations(p1);
    this->p2d2 = ApplyTransformations(p2);
    this->p2d3 = ApplyTransformations(p3);

}

vec2 Triangle::ApplyTransformations(vec4 p){
    vec4 projected = projection * rotation * p;
    vec3 ndc = vec3(projected) / projected.w;
    int x = (ndc.x + 1.0f) * 0.5f * screenWidth;
    int y = (1.0f - (ndc.y + 1.0f) * 0.5f) * screenHeight;
    return vec2(x,y);
}

void Triangle::Render() {
    drawLines(p2d1,p2d2,p2d3);
}

void Triangle::ColorPixel(int x, int y){
    drawHere.push_back(vec2(x,y));
}

void Triangle::drawLine(vec2 p1, vec2 p2) {
    int x0 = p1.x;
    int y0 = p1.y;
    int x1 = p2.x;
    int y1 = p2.y;

    bool steep = abs(y1 - y0) > abs(x1 - x0);

    if (steep) {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }

    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = abs(y1 - y0);
    int error = dx / 2;
    int ystep = (y0 < y1) ? 1 : -1;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        if (steep) {
            if (y >= 0 && y <= 128 &&
                x >= 0 && x <= 128) {
                ColorPixel(y,x);
            }
        } else {
            if (x >= 0 && x <= 128 &&
                y >= 0 && y <= 128) {
                ColorPixel(x,y);
            }
        }

        error -= dy;
        if (error < 0) {
            y += ystep;
            error += dx;
        }
    }
}

void Triangle::drawLines(vec2 p1, vec2 p2, vec2 p3) {
    drawLine(p1,p2);
    drawLine(p1,p3);
    drawLine(p2,p3);
}



