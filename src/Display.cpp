#include "Display.h"

#include "Camera.h"
#include "SDLHelper.h"
#include "backends/imgui_impl_sdl3.h"
#include "backends/imgui_impl_sdlrenderer3.h"
#include "imgui.h"
#include <format>

enum cull_method cull_method;
enum render_method render_method;

static SDL_Window * window = nullptr;
SDL_Renderer * renderer = nullptr;
static uint32_t * color_buffer = nullptr;
float * z_buffer = nullptr;
static SDL_Texture * color_buffer_texture = nullptr;

static TouchControls touch_controls;
static AnimationConfig animConfig;

// Logical size
int window_width = 640;
int window_height = 480;
// Physical pixels
static int renderW = 0;
static int renderH = 0;
static float scale = 0;

// Triangle to render
int num_triangles_to_render = 0;
triangle_t triangles_to_render[MAX_TRIANGLE_PER_MESH];
Uint64 deltaTime = 0;

bool initialize_imgui(SDL_Renderer * renderer, SDL_Window * window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// ImGuiIO & io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	if(!ImGui_ImplSDL3_InitForSDLRenderer(window, renderer))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error initializing ImGui");
		return false;
	}
	if(!ImGui_ImplSDLRenderer3_Init(renderer))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error initializing ImGui ImGui_ImplSDLRenderer3_Init");
		return false;
	}
	return true;
}

bool initialize_window()
{
	SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");
	SDL_SetAppMetadata("3D renderer", "1.2.0", "com.ht.3d-renderer");

	if(!SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Error initializing SDL");
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

	color_buffer = new uint32_t[window_width * window_height];
	color_buffer_texture =
		SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, window_width, window_height);
	z_buffer = new float[window_width * window_height];

	if(!initialize_imgui(renderer, window))
	{
		return false;
	}
	SDL_Log(
		"Window initialized window_width=%d, window_height=%d, renderW=%d, renderH=%d",
		window_width,
		window_height,
		renderW,
		renderH
	);
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
	if(x < 0 || x >= window_width || y < 0 || y >= window_height)
	{
		return;
	}
	color_buffer[y * window_width + x] = color;
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

void create_imgui_touch_control_window()
{
	ImVec2 windowSize(300.0f, 80.0f);
	ImVec2 margin(0.0f, 0.0f);

	ImGui::SetNextWindowSize(windowSize);
	ImGui::SetNextWindowPos(ImVec2(window_width - windowSize.x - margin.x, window_height - windowSize.y - margin.y));

	ImGui::Begin("Inputs", nullptr);

	float buttonWidth = 60.0;
	ImGui::Button("Left", ImVec2(buttonWidth, 0.0f));
	touch_controls.left = ImGui::IsItemActive();
	ImGui::SameLine();

	ImGui::Button("Down", ImVec2(buttonWidth, 0.0f));
	touch_controls.down = ImGui::IsItemActive();
	ImGui::SameLine();
	ImGui::Button("Up", ImVec2(buttonWidth, 0.0f));
	touch_controls.up = ImGui::IsItemActive();
	ImGui::SameLine();
	ImGui::Button("Right", ImVec2(buttonWidth, 0.0f));
	touch_controls.right = ImGui::IsItemActive();

	ImGui::Button("W", ImVec2(buttonWidth, 0.0f));
	touch_controls.w = ImGui::IsItemActive();
	ImGui::SameLine();
	ImGui::Button("S", ImVec2(buttonWidth, 0.0f));
	touch_controls.s = ImGui::IsItemActive();

	ImGui::End();
}
void render_imgui()
{
	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	ImGuiIO & io = ImGui::GetIO();

	// ImGuiWindowFlags flags = ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize;
	// ImGui::SetNextWindowSize(ImVec2(400, 200));
	// ImGui::Begin("Debug Input", nullptr, flags);
	// ImGui::Text("Testing");
	// ImGui::Text("Mouse Position: %.1f, %.1f", io.MousePos.x, io.MousePos.y);
	// ImGui::Text("Display Size: %.1f, %.1f", io.DisplaySize.x, io.DisplaySize.y);
	// ImGui::Text("Framebuffer Scale: %.1f", io.DisplayFramebufferScale.x);
	// ImGui::End();

	create_imgui_touch_control_window();

	ImGui::Render();
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
}
////////////////////////////////////////////////////////////////////////
/// Process input
void handle_key_up(float deltaTimeMs)
{
	update_camera_forward_velocity(vec3_mul(get_camera_direction(), 10.0 * deltaTimeMs));
	update_camera_position(vec3_add(get_camera_position(), get_camera_forward_velocity()));
}

void handle_key_down(float deltaTimeMs)
{
	update_camera_forward_velocity(vec3_mul(get_camera_direction(), 10.0f * deltaTimeMs));
	update_camera_position(vec3_sub(get_camera_position(), get_camera_forward_velocity()));
}
void handle_key_left(float deltaTimeMs)
{
	rotate_camera_yaw(-1.0 * deltaTimeMs);
}

void handle_key_right(float deltaTimeMs)
{
	rotate_camera_yaw(1.0 * deltaTimeMs);
}
void handle_key_w(float deltaTimeMs)
{
	rotate_camera_pitch(3.0 * deltaTimeMs);
}
void handle_key_s(float deltaTimeMs)
{
	rotate_camera_pitch(-3.0 * deltaTimeMs);
}

void handle_touch_controls()
{
	float deltaTimeMs = static_cast<float>(deltaTime) / 1000.0f;
	if(touch_controls.up)
	{
		handle_key_up(deltaTimeMs);
	}
	if(touch_controls.down)
	{
		handle_key_down(deltaTimeMs);
	}
	if(touch_controls.left)
	{
		handle_key_left(deltaTimeMs);
	}
	if(touch_controls.right)
	{
		handle_key_right(deltaTimeMs);
	}
	if(touch_controls.w)
	{
		handle_key_w(deltaTimeMs);
	}
	if(touch_controls.s)
	{
		handle_key_s(deltaTimeMs);
	}
}
void toggle_rotation_x()
{
	animConfig.rotate_x = !animConfig.rotate_x;
}
void toggle_rotation_y()
{
	animConfig.rotate_y = !animConfig.rotate_y;
}
void toggle_rotation_z()
{
	animConfig.rotate_z = !animConfig.rotate_z;
}

AnimationConfig & get_amin_config()
{
	return animConfig;
}
////////////////////////////////////////////////////////////////////////
void clear_color_buffer(uint32_t color)
{
	for(int i = 0; i < window_width * window_height; i++)
	{
		color_buffer[i] = color;
	}
}
void clear_z_buffer()
{
	for(int i = 0; i < window_width * window_height; i++)
	{
		z_buffer[i] = 1.0f;
	}
}

void free_display_resource()
{
	delete[] color_buffer;
	delete[] z_buffer;
}

void destroy_window()
{
	destroy_imgui();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void destroy_imgui()
{
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();
}
