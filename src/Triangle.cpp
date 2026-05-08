#include "Triangle.h"
#include "Display.h"
#include "Mesh.h"
#include "Swap.h"
#include <algorithm>
#include <assert.h>
#include <cmath>

namespace
{
	///////////////////////////////////////////////////////////////////////////////
	// Function to draw a solid pixel at position (x,y) using depth interpolation
	///////////////////////////////////////////////////////////////////////////////
	void draw_triangle_pixel(int x, int y, uint32_t color, vec4_t point_a, vec4_t point_b, vec4_t point_c)
	{
		// Create three vec2 to find the interpolation
		vec2_t p = {static_cast<float>(x), static_cast<float>(y)};
		vec2_t a = vec2_from_vec4(point_a);
		vec2_t b = vec2_from_vec4(point_b);
		vec2_t c = vec2_from_vec4(point_c);

		// Calculate the barycentric coordinates of our point 'p' inside the triangle
		vec3_t weights = barycentric_weights(a, b, c, p);

		float alpha = weights.x;
		float beta = weights.y;
		float gamma = weights.z;

		// Interpolate the value of 1/w for the current pixel
		float interpolated_reciprocal_w = (1 / point_a.w) * alpha + (1 / point_b.w) * beta + (1 / point_c.w) * gamma;

		// Adjust 1/w so the pixels that are closer to the camera have smaller values
		interpolated_reciprocal_w = 1.0 - interpolated_reciprocal_w;

		// Only draw the pixel if the depth value is less than the one previously stored in the z-buffer
		int zBufferIndex = (window_width * y) + x;
		if(zBufferIndex >= window_width * window_height)
		{
			return;
		}
		if(interpolated_reciprocal_w < z_buffer[zBufferIndex])
		{
			// Draw a pixel at position (x,y) with a solid color
			draw_pixel(x, y, color);

			// Update the z-buffer value with the 1/w of this current pixel
			z_buffer[zBufferIndex] = interpolated_reciprocal_w;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Draw a filled triangle with the flat-top/flat-bottom method
// We split the original triangle in two, half flat-bottom and half flat-top
///////////////////////////////////////////////////////////////////////////////
//
//          (x0,y0)
//            / \
//           /   \
//          /     \
//         /       \
//        /         \
//   (x1,y1)---------\
//       \_           \
//          \_         \
//             \_       \
//                \_     \
//                   \    \
//                     \_  \
//                        \_\
//                           \
//                         (x2,y2)
//
///////////////////////////////////////////////////////////////////////////////
void draw_filled_triangle(
	int x0,
	int y0,
	float z0,
	float w0,
	int x1,
	int y1,
	float z1,
	float w1,
	int x2,
	int y2,
	float z2,
	float w2,
	uint32_t color
)
{
	// We need to sort the vertices by y-coordinate ascending (y0 < y1 < y2)
	if(y0 > y1)
	{
		swap(&y0, &y1);
		swap(&x0, &x1);
		swap(&z0, &z1);
		swap(&w0, &w1);
	}
	if(y1 > y2)
	{
		swap(&y1, &y2);
		swap(&x1, &x2);
		swap(&z1, &z2);
		swap(&w1, &w2);
	}
	if(y0 > y1)
	{
		swap(&y0, &y1);
		swap(&x0, &x1);
		swap(&z0, &z1);
		swap(&w0, &w1);
	}

	// Create three vector points after we sort the vertices
	vec4_t point_a = {static_cast<float>(x0), static_cast<float>(y0), z0, w0};
	vec4_t point_b = {static_cast<float>(x1), static_cast<float>(y1), z1, w1};
	vec4_t point_c = {static_cast<float>(x2), static_cast<float>(y2), z2, w2};

	///////////////////////////////////////////////////////
	// Render the upper part of the triangle (flat-bottom)
	///////////////////////////////////////////////////////
	float inv_slope_1 = 0;
	float inv_slope_2 = 0;

	if(y1 - y0 != 0)
		inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
	if(y2 - y0 != 0)
		inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

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
				// Draw our pixel with a solid color
				draw_triangle_pixel(x, y, color, point_a, point_b, point_c);
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
				// Draw our pixel with a solid color
				draw_triangle_pixel(x, y, color, point_a, point_b, point_c);
			}
		}
	}
}

void draw_texel(
	int x,
	int y,
	const png_texture_t & texture,
	vec4_t point_a,
	vec4_t point_b,
	vec4_t point_c,
	tex2_t a_uv,
	tex2_t b_uv,
	tex2_t c_uv
)
{
	vec2_t point_p = {static_cast<float>(x), static_cast<float>(y)};
	vec2_t a = vec2_from_vec4(point_a);
	vec2_t b = vec2_from_vec4(point_b);
	vec2_t c = vec2_from_vec4(point_c);
	vec3_t weights = barycentric_weights(a, b, c, point_p);

	float alpha = weights.x;
	float beta = weights.y;
	float gamma = weights.z;

	if(std::isnan(alpha) || std::isinf(alpha))
	{
		return;
	}

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
	int texture_width = texture.width;
	int texture_height = texture.height;
	uint32_t * i32_texture = texture.texture;
	interpolated_v = 1.0 - interpolated_v;
	int tex_x = std::clamp(static_cast<int>(interpolated_u * texture_width), 0, texture_width - 1);
	int tex_y = std::clamp(static_cast<int>(interpolated_v * texture_height), 0, texture_height - 1);

	auto colorIndex = tex_y * texture_width + tex_x;

	// Adjust 1/w so the pixels that are closer to the camera have smaller values
	interpolated_reciprocal_w = 1.0 - interpolated_reciprocal_w;
	// Only draw the pixel if the depth value is less than the one previously drawn,
	// z_buffer value range from 0.0 (near) -> 1.0 (far)
	int zBufferIndex = y * window_width + x;
	if(zBufferIndex >= window_width * window_height)
	{
		return;
	}

	if(interpolated_reciprocal_w < z_buffer[zBufferIndex])
	{
		draw_pixel(x, y, i32_texture[colorIndex]);
		z_buffer[zBufferIndex] = interpolated_reciprocal_w;
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
	const png_texture_t & texture
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
