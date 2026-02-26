#pragma once
#include "Vector.h"
#include <cstdint>

struct face_t {
    int a;
    int b;
    int c;
};


struct triangle_t {
    vec2_t points[3];
};

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color);
