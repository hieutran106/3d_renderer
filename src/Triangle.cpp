#include "Triangle.h"


void int_swap(int* a, int* b) {
    int tmp = *a;
    *a      = *b;
    *b      = tmp;
}
void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {}

void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {}
// TODO: need to add comment
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }
    if (y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
    }
    if (y0 > y1) {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }
    int My = y1;
    int Mx = (x2 - x0) * (y1 - y0) / static_cast<float>(y2 - y0) + x0;
    // Draw flat-bottom triangle
    fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);
    // Draw flat-top triangle
    fill_flat_top_triangle(x0, y0, Mx, My, x2, y2, color);
}
