#pragma once
#ifndef __XY2D_GAMESTATE
#define __XY2D_GAMESTATE
	#include ".\xy2d_engine.hpp"
	
	namespace XY2D_NAMESPACE {
		struct xy2d_vertex { public: glm::vec3 xyz; glm::vec2 txcoord; };
		enum xy2d_buffertype { STORAGE, TRANSFER_GPU, TRANSFER_CPU, COMPUTE };
		struct xy2d_buffer { public: uint32_t byteLength; xy2d_buffertype type; void* buffer; };
		struct xy2d_image { public: uint32_t width, height, mips; SDL_GPUTextureFormat format; SDL_GPUSampleCount msaa; SDL_GPUTexture* texture; };
		
		typedef xy2d_callback<> xy2d_default_callback;
		typedef xy2d_callback<SDL_Event*> xy2d_event_callback;
		typedef xy2d_callback<SDL_GPUCommandBuffer*> xy2d_prerender_callback;
		typedef xy2d_callback<SDL_GPUCommandBuffer*, SDL_GPUTexture*, SDL_GPURenderPass*, glm::uint32_t&, glm::uint32_t&> xy2d_render_callback;
		
		class xy2d_gamestate {
		private:
			inline static std::vector<xy2d_buffer> bufferMemory;
			inline static std::vector<xy2d_image> imageMemory;
			inline static std::vector<SDL_GPUGraphicsPipeline*> graphicsPipelines;
			inline static std::vector<SDL_GPUComputePipeline*> computePipelines;
			
		public:
			inline static std::vector<std::string> eventlog;
			inline static glm::ivec2 windowSize = glm::ivec2(640, 480);
			inline static glm::ivec2 minWindowSize = glm::ivec2(640, 480);
			inline static SDL_Window* window;
			inline static SDL_GPUDevice* device;
			
			inline static xy2d::xy2d_invoker<> onGameQuit;
			inline static xy2d::xy2d_invoker<> onGameStart;
			inline static xy2d::xy2d_invoker<SDL_Event*> onGameEvent;
			inline static xy2d::xy2d_invoker<> onGameLoop;
			inline static xy2d::xy2d_invoker<SDL_GPUCommandBuffer*> onGamePreRender;
			inline static xy2d::xy2d_invoker<SDL_GPUCommandBuffer*, SDL_GPUTexture*, SDL_GPURenderPass*, glm::uint32_t&, glm::uint32_t&> onGameRender;
			
			inline static void GameWindowInit();
			
			inline static glm::vec2 MousePosition() {
				glm::vec2 mxy;
				SDL_GetMouseState(&mxy.x, &mxy.y);
				return mxy;
			}
			
			inline static void GameWindowSize(int32_t width, int32_t height) {
				windowSize = glm::ivec2(std::max(width, minWindowSize.x), std::max(height, minWindowSize.y));
				if (window != nullptr) SDL_SetWindowSize(window, windowSize.x, windowSize.y);
			}
			
			inline static SDL_AppResult LogEvent(SDL_AppResult result, std::vector<std::string> list) {
				#if XY2D_VALIDATION
				std::stringstream stream;
				for(std::string ss : list) stream << ss;
				eventlog.push_back(stream.str());
				std::cout << "xy2d: " << stream.str() << std::endl;
				#endif
				return result;
			}
			
			inline static void GameQuit(void* gamestate, SDL_AppResult result) {
				onGameQuit.invoke();
				
				for(size_t i = 0; i < graphicsPipelines.size(); i++) SDL_ReleaseGPUGraphicsPipeline(xy2d::xy2d_gamestate::device, graphicsPipelines[i]);
				for(size_t i = 0; i < computePipelines.size(); i++) SDL_ReleaseGPUComputePipeline(xy2d::xy2d_gamestate::device, computePipelines[i]);
				for(size_t i = 0; i < imageMemory.size(); i++) SDL_ReleaseGPUTexture(xy2d::xy2d_gamestate::device, imageMemory[i].texture);
				for(size_t i = 0; i < bufferMemory.size(); i++)
					if (bufferMemory[i].type & (xy2d_buffertype::TRANSFER_GPU | xy2d_buffertype::TRANSFER_CPU) != 0)
						SDL_ReleaseGPUTransferBuffer(xy2d::xy2d_gamestate::device, (SDL_GPUTransferBuffer*) bufferMemory[i].buffer);
					else SDL_ReleaseGPUBuffer(xy2d::xy2d_gamestate::device, (SDL_GPUBuffer*) bufferMemory[i].buffer);
				
				SDL_DestroyGPUDevice(xy2d::xy2d_gamestate::device);
				SDL_DestroyWindow(xy2d::xy2d_gamestate::window);
			}
			
			inline static SDL_AppResult GameEvent(void* gamestate, SDL_Event* event) {
				SDL_AppResult result = (event->type == SDL_EVENT_QUIT || event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
				if (event->type == SDL_EVENT_WINDOW_RESIZED)
					windowSize = glm::ivec2(std::max(event->window.data1, minWindowSize[0]), std::max(event->window.data2, minWindowSize[1]));
				onGameEvent.invoke(event);
				return result;
			}
			
			inline static SDL_AppResult GameStart(void** gamestate, int argc, char* argv[]) {
				SDL_SetAppMetadata("xy2d engine (SDL3)", "1.0", nullptr);
				
				if (!SDL_Init(SDL_INIT_VIDEO))
					return LogEvent(SDL_APP_FAILURE, { "Couldn't initialize SDL: ", SDL_GetError() });
				
				GameWindowInit();
				minWindowSize = glm::ivec2(std::max(640, minWindowSize.x), std::max(480, minWindowSize.y));
				windowSize = glm::ivec2(std::max(windowSize.x, minWindowSize.x), std::max(windowSize.y, minWindowSize.y));
				
				if ((window = SDL_CreateWindow("xy2d engine", windowSize.x, windowSize.y, SDL_WINDOW_RESIZABLE | SDL_WINDOW_INPUT_FOCUS)) == nullptr)
					return LogEvent(SDL_APP_FAILURE, { "Couldn't create window: ", SDL_GetError() });
				
				if ((device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, false, "vulkan")) == nullptr)
					return LogEvent(SDL_APP_FAILURE, { "Couldn't create vulkan GPU device: ", SDL_GetError() });
				
				if (SDL_ClaimWindowForGPUDevice(device, window) == false)
					return LogEvent(SDL_APP_FAILURE, { "Couldn't claim window for GPU device context: ", SDL_GetError() });
				
				SDL_WindowSupportsGPUPresentMode(device, window, SDL_GPU_PRESENTMODE_MAILBOX);
				SDL_SetGPUSwapchainParameters(device, window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR_LINEAR, SDL_GPU_PRESENTMODE_VSYNC);
				SDL_SetGPUAllowedFramesInFlight(device, 1);
				SDL_SetWindowMinimumSize(window, minWindowSize.x, minWindowSize.y);
				onGameStart.invoke();
				return SDL_APP_CONTINUE; 
			}
			
			inline static SDL_AppResult GameLoop(void* gamestate) {
				onGameLoop.invoke();
				
				for(xy2d_prerender_callback cb : onGamePreRender.callbacks) {
					SDL_GPUCommandBuffer* cmdbuffer = SDL_AcquireGPUCommandBuffer(device);
					cb.invoke(cmdbuffer);
					SDL_GPUFence* fencePreRender = SDL_SubmitGPUCommandBufferAndAcquireFence(cmdbuffer);
					SDL_WaitForGPUFences(device, true, &fencePreRender, 1);
				}
				
				SDL_GPUCommandBuffer* cmdbuffer = SDL_AcquireGPUCommandBuffer(device);
					uint32_t swapImageWidth = 0, swapImageHeight = 0;
					SDL_GPUTexture* swapImage = nullptr;
					if (SDL_WaitAndAcquireGPUSwapchainTexture(cmdbuffer, window, &swapImage, &swapImageWidth, &swapImageHeight))
						onGameRender.invoke(cmdbuffer, swapImage, nullptr, swapImageWidth, swapImageHeight);
				SDL_GPUFence* fenceRender = SDL_SubmitGPUCommandBufferAndAcquireFence(cmdbuffer);
				SDL_WaitForGPUFences(device, true, &fenceRender, 1);
				
				return SDL_APP_CONTINUE;
			}
			
			inline static SDL_GPUGraphicsPipeline* GraphicsPipeline(std::string vertexPath, uint32_t vertexStorageBuffers, std::string fragmentPath, uint32_t fragmentStorageTextures, uint32_t fragmentStorageBuffers, uint32_t fragmentSamplers, uint32_t colorTargetCount = 1) {
				size_t vertexSize = 0;
				void* vcode = SDL_LoadFile(vertexPath.c_str(), &vertexSize);
				SDL_GPUShaderCreateInfo vertexInfo = { .code = (uint8_t*) vcode, .code_size = vertexSize, .entrypoint = "main", .format = SDL_GPU_SHADERFORMAT_SPIRV, .stage = SDL_GPU_SHADERSTAGE_VERTEX, .num_uniform_buffers = vertexStorageBuffers};
				SDL_GPUShader* vertexShader = SDL_CreateGPUShader(xy2d_gamestate::device, &vertexInfo);
				SDL_free(vcode);
				
				size_t fragmentSize = 0;
				void* fcode = SDL_LoadFile(fragmentPath.c_str(), &fragmentSize);
				SDL_GPUShaderCreateInfo fragmentInfo = { .code = (uint8_t*) fcode, .code_size = fragmentSize, .entrypoint = "main", .format = SDL_GPU_SHADERFORMAT_SPIRV, .stage = SDL_GPU_SHADERSTAGE_FRAGMENT, .num_storage_textures = fragmentStorageTextures, .num_uniform_buffers = fragmentStorageBuffers, .num_samplers = fragmentSamplers };
				SDL_GPUShader* fragmentShader = SDL_CreateGPUShader(xy2d_gamestate::device, &fragmentInfo);
				SDL_free(fcode);
				
				SDL_GPUVertexBufferDescription vertexDescriptions[1];
				vertexDescriptions[0] = { .slot = 0, .instance_step_rate = 0, .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX, .pitch = sizeof(xy2d_vertex) };
				
				SDL_GPUVertexAttribute vertexAttributes[2];
				vertexAttributes[0] = { .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3, .location = 0, .offset = offsetof(xy2d_vertex, xyz), .buffer_slot = 0 };
				vertexAttributes[1] = { .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2, .location = 1, .offset = offsetof(xy2d_vertex, txcoord), .buffer_slot = 0 };
				
				std::vector<SDL_GPUColorTargetDescription> colorDescr(colorTargetCount);
				for(size_t i = 0; i < std::max(colorTargetCount, 1U); i++) {
					colorDescr[i].blend_state.enable_blend = true;
					colorDescr[i].blend_state.color_blend_op = colorDescr[i].blend_state.alpha_blend_op = SDL_GPU_BLENDOP_ADD;
					colorDescr[i].blend_state.src_color_blendfactor = colorDescr[i].blend_state.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
					colorDescr[i].blend_state.dst_color_blendfactor = colorDescr[i].blend_state.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
					colorDescr[i].format = SDL_GetGPUSwapchainTextureFormat(xy2d_gamestate::device, xy2d_gamestate::window);
				};
				
				SDL_GPUGraphicsPipelineCreateInfo pipelineInfo {
					.vertex_shader = vertexShader,
					.fragment_shader = fragmentShader,
					.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
					.vertex_input_state.num_vertex_buffers = std::size(vertexDescriptions),
					.vertex_input_state.vertex_buffer_descriptions = vertexDescriptions,
					.vertex_input_state.num_vertex_attributes = std::size(vertexAttributes),
					.vertex_input_state.vertex_attributes = vertexAttributes,
					.target_info.num_color_targets = std::max(colorTargetCount, 1U),
					.target_info.color_target_descriptions = colorDescr.data(),
				};
				
				graphicsPipelines.push_back(SDL_CreateGPUGraphicsPipeline(xy2d_gamestate::device, &pipelineInfo));
				SDL_ReleaseGPUShader(xy2d_gamestate::device, vertexShader);
				SDL_ReleaseGPUShader(xy2d_gamestate::device, fragmentShader);
				return graphicsPipelines.back();
			}
			
			inline static SDL_GPUComputePipeline* ComputePipeline(std::string computePath, uint32_t storageTextures, uint32_t samplerTextureCount, uint32_t storageBuffers, uint32_t uniformBufferCount, glm::ivec3 threadsXYZ) {
				size_t computeSize;
				void* computeCode = SDL_LoadFile(computePath.c_str(), &computeSize);
				
				SDL_GPUComputePipelineCreateInfo computeInfo = {
					.code = (uint8_t*) computeCode,
					.code_size = computeSize,
					.entrypoint = "main",
					.format = SDL_GPU_SHADERFORMAT_SPIRV,
					.num_readwrite_storage_buffers = storageBuffers,
					.num_readwrite_storage_textures = storageTextures,
					.num_uniform_buffers = uniformBufferCount,
					.num_samplers = samplerTextureCount,
					.threadcount_x = static_cast<Uint32>(threadsXYZ.x),
					.threadcount_y = static_cast<Uint32>(threadsXYZ.y),
					.threadcount_z = static_cast<Uint32>(threadsXYZ.z)
				};
				
				computePipelines.push_back(SDL_CreateGPUComputePipeline(xy2d_gamestate::device, &computeInfo));
				SDL_free(computeCode);
				return computePipelines.back();
			}
			
			inline static xy2d_image CreateTexture(SDL_GPUTextureFormat format, uint32_t width, uint32_t height,  uint32_t mips = 1, SDL_GPUSampleCount msaa = SDL_GPU_SAMPLECOUNT_1) {
				SDL_GPUTextureCreateInfo textureInfo = {
					.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_SIMULTANEOUS_READ_WRITE | SDL_GPU_TEXTUREUSAGE_GRAPHICS_STORAGE_READ | SDL_GPU_TEXTUREUSAGE_SAMPLER,
					.type = SDL_GPU_TEXTURETYPE_2D, .layer_count_or_depth = 1U, .format = format, .num_levels = mips, .sample_count = msaa, .width = width, .height = height
				};
				imageMemory.push_back(xy2d_image(width, height, mips, format, msaa, SDL_CreateGPUTexture(xy2d_gamestate::device, &textureInfo)));
				return imageMemory.back();
			}
			
			inline static xy2d_buffer CreateBuffer(uint32_t byteLength, xy2d_buffertype type) {
				switch(type) {
					case xy2d_buffertype::STORAGE: {
						SDL_GPUBufferCreateInfo bufferInfo = { .size = byteLength, .usage = SDL_GPU_BUFFERUSAGE_VERTEX | SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ };
						bufferMemory.push_back(xy2d_buffer(byteLength, xy2d_buffertype::STORAGE, SDL_CreateGPUBuffer(xy2d_gamestate::device, &bufferInfo)));
					} break;
					case xy2d_buffertype::TRANSFER_GPU: {
						SDL_GPUTransferBufferCreateInfo bufferInfo = { .size = byteLength, .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD };
						bufferMemory.push_back(xy2d_buffer(byteLength, xy2d_buffertype::TRANSFER_GPU, SDL_CreateGPUTransferBuffer(xy2d_gamestate::device, &bufferInfo)));
					} break;
					case xy2d_buffertype::TRANSFER_CPU: {
						SDL_GPUTransferBufferCreateInfo bufferInfo = { .size = byteLength, .usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD };
						bufferMemory.push_back(xy2d_buffer(byteLength, xy2d_buffertype::TRANSFER_CPU, SDL_CreateGPUTransferBuffer(xy2d_gamestate::device, &bufferInfo)));
					} break;
					case xy2d_buffertype::COMPUTE: {
						SDL_GPUBufferCreateInfo bufferInfo = { .size = byteLength, .usage = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_READ | SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE };
						bufferMemory.push_back(xy2d_buffer(byteLength, xy2d_buffertype::COMPUTE, SDL_CreateGPUBuffer(xy2d_gamestate::device, &bufferInfo)));
					} break;
				}
				return bufferMemory.back();
			}
			
			inline static bool StageBuffer(xy2d_buffer* transferBuffer, void* pointerData, size_t sizeOfData) {
				void* indices = (void*) SDL_MapGPUTransferBuffer(xy2d_gamestate::device, (SDL_GPUTransferBuffer*) transferBuffer->buffer, false);
					SDL_memcpy(indices, pointerData, sizeOfData);
				SDL_UnmapGPUTransferBuffer(xy2d_gamestate::device, (SDL_GPUTransferBuffer*) transferBuffer->buffer);
				return (indices != nullptr);
			}
			
			inline static bool TransferBuffer(xy2d_buffer* transferBuffer, xy2d_buffer* destinationBuffer) {
				SDL_GPUCommandBuffer* cmdBuffer = SDL_AcquireGPUCommandBuffer(xy2d_gamestate::device);
					SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);
						SDL_GPUTransferBufferLocation location = { .transfer_buffer = (SDL_GPUTransferBuffer*) transferBuffer->buffer, .offset = 0 };
						SDL_GPUBufferRegion region = { .buffer = (SDL_GPUBuffer*) destinationBuffer->buffer, .size = destinationBuffer->byteLength, .offset = 0 };
						SDL_UploadToGPUBuffer(copyPass, &location, &region, true);
					SDL_EndGPUCopyPass(copyPass);
				return SDL_SubmitGPUCommandBuffer(cmdBuffer);
			}
		};
	}
	
	static void SDL_AppQuit(void* gamestate, SDL_AppResult result) { xy2d::xy2d_gamestate::GameQuit(gamestate, result); }
	static SDL_AppResult SDL_AppEvent(void* gamestate, SDL_Event* event) { return xy2d::xy2d_gamestate::GameEvent(gamestate, event); }
	static SDL_AppResult SDL_AppInit(void** gamestate, int argc, char* argv[]) { return xy2d::xy2d_gamestate::GameStart(gamestate, argc, argv); }
	static SDL_AppResult SDL_AppIterate(void* gamestate) { return xy2d::xy2d_gamestate::GameLoop(gamestate); }
#endif