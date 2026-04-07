#include "Triangle.h"
#include "Display.h"
#include "Swap.h"
#include <algorithm>
#include <assert.h>
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

void draw_texel(
	int x,
	int y,
	const uint32_t * texture,
	vec2_t point_a,
	vec2_t point_b,
	vec2_t point_c,
	float u0,
	float v0,
	float u1,
	float v1,
	float u2,
	float v2
)
{
	vec2_t point_p = {static_cast<float>(x), static_cast<float>(y)};
	vec3_t weights = barycentric_weights(point_a, point_b, point_c, point_p);

	float alpha = weights.x;
	float beta = weights.y;
	float gamma = weights.z;

	float interpolated_u = u0 * alpha + u1 * beta + u2 * gamma;
	float interpolated_v = v0 * alpha + v1 * beta + v2 * gamma;

	int tex_x = static_cast<int>(interpolated_u * texture_width);
	int tex_y = static_cast<int>(interpolated_v * texture_height);

	// tex_x can go out of range whenever interpolated_u is not guaranteed to stay in [0, 1)
	// Floating-point precision + Rounding behavior
	tex_x = std::clamp(tex_x, 0, texture_width - 1);
	tex_y = std::clamp(tex_y, 0, texture_height - 1);

	auto colorIndex = tex_y * texture_width + tex_x;
	draw_pixel(x, y, texture[colorIndex]);
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

	// Create vector points and texture coords after we sort the vertices
	vec2_t point_a = {static_cast<float>(x0), static_cast<float>(y0)};
	vec2_t point_b = {static_cast<float>(x1), static_cast<float>(y1)};
	vec2_t point_c = {static_cast<float>(x2), static_cast<float>(y2)};

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
				// Draw our pixel with the color that comes from the texture
				draw_texel(x, y, texture, point_a, point_b, point_c, u0, v0, u1, v1, u2, v2);
			}
		}
	}

	///////////////////////////////////////////////////////
	// Render the bottom part of the triangle (flat-top)
	///////////////////////////////////////////////////////
	inv_slope_1 = 0;
	inv_slope_2 = 0;

	if(y2 - y1 != 0)
		inv_slope_1 = static_cast<float>(x2 - x1) / abs(y2 - y1);
	if(y2 - y0 != 0)
		inv_slope_2 = static_cast<float>(x2 - x0) / abs(y2 - y0);

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
				// Draw our pixel with the color that comes from the texture
				draw_texel(x, y, texture, point_a, point_b, point_c, u0, v0, u1, v1, u2, v2);
			}
		}
	}
}
