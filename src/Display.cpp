#include "Display.h"
#include "SDLHelper.h"
#include <format>

enum cull_method cull_method;
enum render_method render_method;

SDL_Window * window = nullptr;
SDL_Renderer * renderer = nullptr;
uint32_t * color_buffer = nullptr;
float * z_buffer = nullptr;
SDL_Texture * color_buffer_texture = nullptr;
TTF_Font * font = nullptr;

// Logical size
int window_width = 1024;
int window_height = 768;
// Physical pixels
int renderW = 0;
int renderH = 0;
float scale = 0;
// Triangle to render
int num_triangles_to_render = 0;
triangle_t triangles_to_render[MAX_TRIANGLE_PER_MESH];

bool initialize_window()
{
	if(!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error initializing SDL");
		return false;
	}
	if(!TTF_Init())
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error initializing SDL_TTF");
		return false;
	}

	font = TTF_OpenFont("../assets/fonts/arial.ttf", 16);
	if(!font)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error opening font");
		return false;
	}

	SDL_WindowFlags windowFlags = SDL_WINDOW_HIGH_PIXEL_DENSITY;
	windowFlags = windowFlags | SDL_WINDOW_RESIZABLE;
	window = SDL_CreateWindow("3d renderer", 1024, 768, windowFlags);
	if(!window)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error creating SDL Window");
		return false;
	}
	SDLHelper::SetRenderSizeInPixels(window, &renderW, &renderH, &scale);
	renderer = SDL_CreateRenderer(window, NULL);
	SDL_SetRenderLogicalPresentation(renderer, window_width, window_height, SDL_LOGICAL_PRESENTATION_STRETCH);
	return true;
}

void draw_grid()
{
	for(int y = 0; y < window_height; y++)
	{
		for(int x = 0; x < window_width; x++)
		{
			if(y % 10 == 0 || x % 10 == 0)
			{
				color_buffer[y * window_width + x] = 0xFF333333;
			}
		}
	}
}

void draw_pixel(int x, int y, uint32_t color)
{
	if(x >= 0 && x < window_width && y >= 0 && y < window_height)
	{
		color_buffer[y * window_width + x] = color;
	}
}
void draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
	int delta_x = (x1 - x0);
	int delta_y = (y1 - y0);

	int longest_side_length = (abs(delta_x) >= abs(delta_y)) ? abs(delta_x) : abs(delta_y);

	float x_inc = delta_x / (float)longest_side_length;
	float y_inc = delta_y / (float)longest_side_length;

	float current_x = x0;
	float current_y = y0;
	for(int i = 0; i <= longest_side_length; i++)
	{
		draw_pixel(round(current_x), round(current_y), color);
		current_x += x_inc;
		current_y += y_inc;
	}
}
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
	draw_line(x0, y0, x1, y1, color);
	draw_line(x1, y1, x2, y2, color);
	draw_line(x2, y2, x0, y0, color);
}

void draw_rect(int x, int y, int width, int height, uint32_t color)
{
	for(int i = 0; i < width; i++)
	{
		for(int j = 0; j < height; j++)
		{
			int current_x = x + i;
			int current_y = y + j;
			draw_pixel(current_x, current_y, color);
		}
	}
}

void render_color_buffer()
{
	SDL_UpdateTexture(color_buffer_texture, nullptr, color_buffer, window_width * sizeof(uint32_t));
	SDL_RenderTexture(renderer, color_buffer_texture, nullptr, nullptr);
}

static void render_text_line(const char * text, float x, float y, SDL_Color color)
{
	SDL_Surface * textSurface = TTF_RenderText_Blended(font, text, 0, color);
	SDL_Texture * textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_DestroySurface(textSurface);

	float textWidth = textTexture->w;
	float textHeight = textTexture->h;

	SDL_FRect destRect = {x, y, textWidth, textHeight};
	SDL_RenderTexture(renderer, textTexture, nullptr, &destRect);
	SDL_DestroyTexture(textTexture);
}

void render_text()
{
	SDL_Color white = {255, 255, 255, 255};
	const char * renderMethodText = nullptr;
	switch(render_method)
	{
		case RENDER_WIRE:
			renderMethodText = "Render: RENDER_WIRE";
			break;
		case RENDER_WIRE_VERTEX:
			renderMethodText = "Render: RENDER_WIRE_VERTEX";
			break;
		case RENDER_FILL_TRIANGLE:
			renderMethodText = "Render: RENDER_FILL_TRIANGLE";
			break;
		case RENDER_FILL_TRIANGLE_WIRE:
			renderMethodText = "Render: RENDER_FILL_TRIANGLE_WIRE";
			break;
		case RENDER_TEXTURED:
			renderMethodText = "Render: RENDER_TEXTURED";
			break;
		case RENDER_TEXTURED_WIRE:
			renderMethodText = "Render: RENDER_TEXTURED_WIRE";
			break;
	}
	render_text_line(renderMethodText, 10.0f, 10.0f, white);

	// Render culling method
	const char * cullMethodText = "Cull face: Disabled";
	if(cull_method == CULL_BACKFACE)
	{
		cullMethodText = "Cull face: Enabled";
	}

	render_text_line(cullMethodText, 10.0f, 30.0f, white);

	////////////////////////////////////////////////////////
	// Render triangle to render count
	std::string triangleCountText = std::format("Triangle to Render: {}", num_triangles_to_render);
	render_text_line(triangleCountText.c_str(), 10.0f, 50.0f, white);
}

void clear_color_buffer(uint32_t color)
{
	for(int y = 0; y < window_height; y++)
	{
		for(int x = 0; x < window_width; x++)
		{
			color_buffer[y * window_width + x] = color;
		}
	}
}
void clear_z_buffer()
{
	for(int y = 0; y < window_height; y++)
	{
		for(int x = 0; x < window_width; x++)
		{
			z_buffer[y * window_width + x] = 1.0f;
		}
	}
}
void destroy_window()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();
}
