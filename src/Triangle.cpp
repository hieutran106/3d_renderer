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

void draw_texel(int x, int y, const uint32_t * texture, vec4_t point_a, vec4_t point_b, vec4_t point_c, tex2_t a_uv, tex2_t b_uv, tex2_t c_uv)
{
	vec2_t point_p = {static_cast<float>(x), static_cast<float>(y)};
	vec2_t a = vec2_from_vec4(point_a);
	vec2_t b = vec2_from_vec4(point_b);
	vec2_t c = vec2_from_vec4(point_c);
	vec3_t weights = barycentric_weights(a, b, c, point_p);

	float alpha = weights.x;
	float beta = weights.y;
	float gamma = weights.z;

	// Variables to store the interpolated values of U, V, and also 1/w for the current pixel
	float interpolated_u;
	float interpolated_v;
	float interpolated_reciprocal_w;

	// Perform the interpolation of all U/w and V/w values using barycentric weights and a factor of 1/w
	interpolated_u = (a_uv.u / point_a.w) * alpha + (b_uv.u / point_b.w) * beta + (c_uv.u / point_c.w) * gamma;
	interpolated_v = (a_uv.v / point_a.w) * alpha + (b_uv.v / point_b.w) * beta + (c_uv.v / point_c.w) * gamma;

	// Also interpolate the value of 1/w for the current pixel
	interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

	// Now we can divide back both interpolated values by 1/w
	interpolated_u /= interpolated_reciprocal_w;
	interpolated_v /= interpolated_reciprocal_w;

	// Map the UV coordinate to the full texture width and height
	// tex_x can go out of range whenever interpolated_u is not guaranteed to stay in [0, 1)
	// Floating-point precision + Rounding behavior
	int tex_x = std::clamp(static_cast<int>(interpolated_u * texture_width), 0, texture_width - 1);
	int tex_y = std::clamp(static_cast<int>(interpolated_v * texture_height), 0, texture_height - 1);

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
	float z0,
	float w0,
	float u0,
	float v0,
	int x1,
	int y1,
	float z1,
	float w1,
	float u1,
	float v1,
	int x2,
	int y2,
	float z2,
	float w2,
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
		swap(&z0, &z1);
		swap(&w0, &w1);
		swap(&u0, &u1);
		swap(&v0, &v1);
	}
	if(y1 > y2)
	{
		swap(&y1, &y2);
		swap(&x1, &x2);
		swap(&z1, &z2);
		swap(&w1, &w2);
		swap(&u1, &u2);
		swap(&v1, &v2);
	}
	if(y0 > y1)
	{
		swap(&y0, &y1);
		swap(&x0, &x1);
		swap(&z0, &z1);
		swap(&w0, &w1);
		swap(&u0, &u1);
		swap(&v0, &v1);
	}

	// Create vector points and texture coords after we sort the vertices
	vec4_t point_a = {static_cast<float>(x0), static_cast<float>(y0), z0, w0};
	vec4_t point_b = {static_cast<float>(x1), static_cast<float>(y1), z1, w1};
	vec4_t point_c = {static_cast<float>(x2), static_cast<float>(y2), z2, w2};
	tex2_t a_uv = {u0, v0};
	tex2_t b_uv = {u1, v1};
	tex2_t c_uv = {u2, v2};

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
				draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
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
				draw_texel(x, y, texture, point_a, point_b, point_c, a_uv, b_uv, c_uv);
			}
		}
	}
}
