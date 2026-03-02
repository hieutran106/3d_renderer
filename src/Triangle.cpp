#include "Triangle.h"

#include "Display.h"

void int_swap(int * a, int * b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat bottom
///////////////////////////////////////////////////////////////////////////////
//
//        (x0,y0)
//          / \
//         /   \
//        /     \
//       /       \
//      /         \
//  (x1,y1)------(x2,y2)
//
///////////////////////////////////////////////////////////////////////////////
void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
	float inv_slope_1 = (x1 - x0) / static_cast<float>(y1 - y0);
	float inv_slope_2 = (x2 - x0) / static_cast<float>(y2 - y0);
	// Loop all the scanlines from top to bottom
	for(int y = y0; y <= y1; y++)
	{
		int x_start = (y - y1) * inv_slope_1 + x1;
		int x_end = (y - y2) * inv_slope_2 + x2;
		draw_line(x_start, y, x_end, y, color);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled a triangle with a flat top
///////////////////////////////////////////////////////////////////////////////
//
//  (x0,y0)------(x1,y1)
//      \         /
//       \       /
//        \     /
//         \   /
//          \ /
//        (x2,y2)
//
///////////////////////////////////////////////////////////////////////////////
void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
	float inv_slope_1 = (x2 - x0) / static_cast<float>(y2 - y0);
	float inv_slope_2 = (x2 - x1) / static_cast<float>(y2 - y1);
	// Loop all the scanlines from top to bottom
	for(int y = y2; y >= y0; y--)
	{
		int x_start = (y - y0) * inv_slope_1 + x0;
		int x_end = (y - y1) * inv_slope_2 + x1;
		draw_line(x_start, y, x_end, y, color);
	}
}
// TODO: need to add comment
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
	// We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
	if(y0 > y1)
	{
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
	}
	if(y1 > y2)
	{
		int_swap(&y1, &y2);
		int_swap(&x1, &x2);
	}
	if(y0 > y1)
	{
		int_swap(&y0, &y1);
		int_swap(&x0, &x1);
	}

	if(y1 == y2)
	{
		fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
	}
	else if(y0 == y1)
	{
		fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
	}
	else
	{
		int My = y1;
		int Mx = (x2 - x0) * (y1 - y0) / static_cast<float>(y2 - y0) + x0;
		// Draw flat-bottom triangle
		fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);
		// Draw flat-top triangle
		fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
	}
}
