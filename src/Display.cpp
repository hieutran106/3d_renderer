#include "Display.h"
#include "SDLHelper.h"

enum cull_method cull_method;
enum render_method render_method;

SDL_Window * window = nullptr;
SDL_Renderer * renderer = nullptr;
uint32_t * color_buffer = nullptr;
SDL_Texture * color_buffer_texture = nullptr;
TTF_Font * font = nullptr;

// Logical size
int window_width = 1024;
int window_height = 768;
// Physical pixels
int renderW = 0;
int renderH = 0;
float scale = 0;

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
	}
	SDL_Surface * textSurface = TTF_RenderText_Blended(font, renderMethodText, 0, white);
	SDL_Texture * textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_DestroySurface(textSurface);

	float textWidth = (float)textTexture->w;
	float textHeight = (float)textTexture->h;

	SDL_FRect destRect = {10.0f, 10.0f, textWidth, textHeight};
	SDL_RenderTexture(renderer, textTexture, nullptr, &destRect);
	SDL_DestroyTexture(textTexture);

	// Render culling method
	const char * cullMethodText = "Cull (C/D key): NONE";
	if(cull_method == CULL_BACKFACE)
	{
		cullMethodText = "Cull (C/D key): CULL_BACKFACE";
	}

	textSurface = TTF_RenderText_Blended(font, cullMethodText, 0, white);
	textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	SDL_DestroySurface(textSurface);

	textWidth = (float)textTexture->w;
	textHeight = (float)textTexture->h;

	destRect = {10.0f, 30.0f, textWidth, textHeight};
	SDL_RenderTexture(renderer, textTexture, nullptr, &destRect);
	SDL_DestroyTexture(textTexture);
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
void destroy_window()
{
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	TTF_Quit();
	SDL_Quit();
}
