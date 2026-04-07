#include "Triangle.h"
#include "Display.h"
#include "Swap.h"
#include <cmath>

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
		swap(&y0, &y1);
		swap(&x0, &x1);
	}
	if(y1 > y2)
	{
		swap(&y1, &y2);
		swap(&x1, &x2);
	}
	if(y0 > y1)
	{
		swap(&y0, &y1);
		swap(&x0, &x1);
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
/*
 * Draw a textured triangle based on a texture array of colors.
 * We split the original triangle in two, half flat-bottom and half flat-top.
 */
///////////////////////////////////////////////////////////////////////////////
//
//        v0
//        /\
//       /  \
//      /    \
//     /      \
//   v1--------\
//     \_       \
//        \_     \
//           \_   \
//              \_ \
//                 \\
//                   \
//                    v2
//
///////////////////////////////////////////////////////////////////////////////
void draw_textured_triangle(
	int x0,
	int y0,
	float u0,
	float v0,
	int x1,
	int y1,
	float u1,
	float v1,
	int x2,
	int y2,
	float u2,
	float v2,
	const uint32_t * texture
)
{
	// We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
	if(y0 > y1)
	{
		swap(&y0, &y1);
		swap(&x0, &x1);
		swap(&u0, &u1);
		swap(&v0, &v1);
	}
	if(y1 > y2)
	{
		swap(&y1, &y2);
		swap(&x1, &x2);
		swap(&u1, &u2);
		swap(&v1, &v2);
	}
	if(y0 > y1)
	{
		swap(&y0, &y1);
		swap(&x0, &x1);
		swap(&u0, &u1);
		swap(&v0, &v1);
	}
	///////////////////////////////////////////////////////
	// Render the upper part of the triangle (flat-bottom)
	///////////////////////////////////////////////////////
	float inv_slope_1 = 0;
	float inv_slope_2 = 0;

	if(y1 - y0 != 0)
		inv_slope_1 = static_cast<float>(x1 - x0) / abs(y1 - y0);
	if(y2 - y0 != 0)
		inv_slope_2 = static_cast<float>(x2 - x0) / abs(y2 - y0);

	if(y1 - y0 != 0)
	{
		for(int y = y0; y <= y1; y++)
		{
			int x_start = x1 + (y - y1) * inv_slope_1;
			int x_end = x0 + (y - y0) * inv_slope_2;

			if(x_end < x_start)
			{
				swap(&x_start, &x_end); // swap if x_start is to the right of x_end
			}

			for(int x = x_start; x < x_end; x++)
			{
				// Draw our pixel with a custom color
				draw_pixel(x, y, (x % 2 == 0 && y % 2 == 0) ? 0xFFFF00FF : 0x00000000);
			}
		}
	}

	///////////////////////////////////////////////////////
	// Render the bottom part of the triangle (flat-top)
	///////////////////////////////////////////////////////
	inv_slope_1 = 0;
	inv_slope_2 = 0;

	if(y2 - y1 != 0)
		inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);
	if(y2 - y0 != 0)
		inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

	if(y2 - y1 != 0)
	{
		for(int y = y1; y <= y2; y++)
		{
			int x_start = x1 + (y - y1) * inv_slope_1;
			int x_end = x0 + (y - y0) * inv_slope_2;

			if(x_end < x_start)
			{
				swap(&x_start, &x_end); // swap if x_start is to the right of x_end
			}

			for(int x = x_start; x < x_end; x++)
			{
				/// Draw our pixel with a custom color
				draw_pixel(x, y, (x % 2 == 0 && y % 2 == 0) ? 0xFFFF00FF : 0x00000000);
			}
		}
	}
}
