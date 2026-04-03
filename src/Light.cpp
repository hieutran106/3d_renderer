#include "Light.h"

light_t light = {
	.direction = {0, 0, 1}
};

uint32_t light_apply_intensity(uint32_t color, float percent)
{
	uint32_t a = (color & 0xFF000000);
	uint32_t r = (color & 0x00FF0000) * percent;
	uint32_t g = (color & 0x0000FF00) * percent;
	uint32_t b = (color & 0x000000FF) * percent;

	uint32_t new_color = a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);
	return new_color;
}
