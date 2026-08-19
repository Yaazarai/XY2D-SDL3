#pragma once
#ifndef __XY2D_ENGINE
#define __XY2D_ENGINE

	#define GLM_FORCE_RADIANS
	#define GLM_FORCE_LEFT_HANDED
	#define GLM_FORCE_DEPTH_ZERO_TO_ONE
	#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
	#include <glm/glm.hpp>
	#include <glm/ext.hpp>
	
	#define SDL_MAIN_USE_CALLBACKS 1
	#include <SDL3/SDL.h>
	#include <SDL3/SDL_main.h>
	
	#ifdef _DEBUG
		#define XY2D_VALIDATION true
	#else
		#define XY2D_VALIDATION false
	#endif
	
	#ifndef XY2D_NAMESPACE
		#define XY2D_NAMESPACE xy2d
		namespace XY2D_NAMESPACE {}
	#endif
	
	#include <filesystem>
	#include <fstream>
	#include <iostream>
	#include <sstream>
	#include <string>
	#include <vector>
	#include <unordered_map>
	#include <algorithm>
	#include <functional>
	#include <utility>
	#include <chrono>
	#include <mutex>
	
	#pragma region XY2D_LIBRARY
		#include ".\SDL3-XY2D\xy2d_callbacks.hpp"
		#include ".\SDL3-XY2D\xy2d_gamestate.hpp"
		#include ".\SDL3-XY2D\xy2d_sprite.hpp"
		#include ".\SDL3-XY2D\xy2d_renderer.hpp"
	#pragma endregion
#endif