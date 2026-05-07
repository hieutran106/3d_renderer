#pragma once

#include "Logger.h"
#include <SDL3/SDL.h>
#include <string>
#include <vector>

namespace SDLHelper
{
	inline void SetRenderSizeInPixels(SDL_Window * window, int * renderW, int * renderH, float * scale)
	{
		int windowW, windowH;
		SDL_GetWindowSize(window, &windowW, &windowH);
		float density = SDL_GetWindowPixelDensity(window);
		*scale = density;
		SDL_LogDebug(MY_LOG_SDL, "SDL Window size: %d x %d, density: %.2f", windowW, windowH, density);

		SDL_GetWindowSizeInPixels(window, renderW, renderH);
		SDL_LogDebug(MY_LOG_SDL, "Physical pixels: %d x %d", *renderW, *renderH);
	}

	inline bool IsIOS()
	{
		const char * platform = SDL_GetPlatform();
		return SDL_strcmp(platform, "iOS") == 0;
	}

	inline void LogSDLWindowFlags(SDL_Window * window)
	{
		Uint64 flags = SDL_GetWindowFlags(window);
		std::string report = "Effective Window Flags for ID " + std::to_string(SDL_GetWindowID(window)) + ":\n";
		struct FlagInfo
		{
			Uint64 bit;
			std::string name;
		};

		std::vector<FlagInfo> checkList = {
			{SDL_WINDOW_FULLSCREEN,          "FULLSCREEN"           },
			{SDL_WINDOW_OPENGL,              "OPENGL"               },
			{SDL_WINDOW_OCCLUDED,            "OCCLUDED"             },
			{SDL_WINDOW_HIDDEN,              "HIDDEN"               },
			{SDL_WINDOW_BORDERLESS,          "BORDERLESS"           },
			{SDL_WINDOW_RESIZABLE,           "RESIZABLE"            },
			{SDL_WINDOW_MINIMIZED,           "MINIMIZED"            },
			{SDL_WINDOW_MAXIMIZED,           "MAXIMIZED"            },
			{SDL_WINDOW_MOUSE_GRABBED,       "MOUSE_GRABBED"        },
			{SDL_WINDOW_INPUT_FOCUS,         "INPUT_FOCUS"          },
			{SDL_WINDOW_MOUSE_FOCUS,         "MOUSE_FOCUS"          },
			{SDL_WINDOW_HIGH_PIXEL_DENSITY,  "HIGH_PIXEL_DENSITY"   },
			{SDL_WINDOW_MOUSE_RELATIVE_MODE, "MOUSE_RELATIVE_MODE"  },
			{SDL_WINDOW_VULKAN,              "VULKAN"               },
			{SDL_WINDOW_METAL,               "METAL"                },
			{SDL_WINDOW_BORDERLESS,          "SDL_WINDOW_BORDERLESS"}
		};
		for(const auto & item : checkList)
		{
			if(flags & item.bit)
			{
				report += "  [X] " + item.name + "\n";
			}
		}
		SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s", report.c_str());
	}

	inline std::string GetResourcePath(const char * filename)
	{
		const char * base_path = SDL_GetBasePath();
		SDL_Log("SDL_GetBasePath: %s", base_path);

		if(base_path)
		{
			std::string fullPath = std::string(base_path) + filename;
			return fullPath;
		}
		return std::string(filename);
	}
}
