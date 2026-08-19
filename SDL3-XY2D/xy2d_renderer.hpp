#pragma once
#ifndef __XY2D_RENDERER
#define __XY2D_RENDERER
	#include ".\xy2d_engine.hpp"
	
	namespace XY2D_NAMESPACE {
		class xy2d_renderer {
		public:
			inline static glm::mat4 CameraTransform(glm::vec2 cameraSize, glm::vec2 cameraPosition, glm::vec2 cameraScale = glm::vec2(1.0f), glm::float32_t cameraTheta = 0.0f, glm::vec2 zNearFar = glm::vec2(1.0f, -1.0f)) {
				glm::mat4 projection = glm::ortho(0.0f, cameraSize.x, cameraSize.y, 0.0f, zNearFar.x, zNearFar.y);
				glm::vec2 cameraCenter = cameraSize / 2.0f;
				projection = glm::translate(projection, glm::vec3(cameraCenter, 0.0f));
				projection = glm::rotate(projection, cameraTheta, glm::vec3(0.0f, 0.0f, 1.0f));
				projection = glm::scale(projection, glm::vec3(cameraScale, 1.0f));
				projection = glm::translate(projection, glm::vec3(-cameraCenter, 0.0f));
				return projection;
			}
			
			inline static void ResizeEvent(SDL_Event* event, xy2d_image* renderer) {
				if (event->type != SDL_EVENT_WINDOW_RESIZED) return;
				
				uint32_t width = renderer->width;
				uint32_t height = width * static_cast<glm::float32_t>(xy2d_gamestate::windowSize.y) / static_cast<glm::float32_t>(xy2d_gamestate::windowSize.x);
				SDL_ReleaseGPUTexture(xy2d_gamestate::device, renderer->texture);
				*renderer = xy2d_gamestate::CreateTexture(SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM_SRGB, width, height);
				
				xy2d_gamestate::LogEvent(SDL_APP_SUCCESS, { "Resize Event: ", std::to_string(width), " : ", std::to_string(height) });
			}
			
			inline static SDL_GPURenderPass* RenderPassBegin(SDL_GPUCommandBuffer* cmdbuffer, const std::vector<SDL_GPUTexture*> textures, SDL_FColor clearColor, SDL_GPULoadOp loadOp) {
				std::vector<SDL_GPUColorTargetInfo> targets(textures.size());
					for(size_t i = 0; i < textures.size(); i++)
						targets[i] = { .load_op = loadOp, .store_op = SDL_GPU_STOREOP_STORE, .texture = textures[i], .clear_color = clearColor };
				
				return SDL_BeginGPURenderPass(cmdbuffer, targets.data(), targets.size(), nullptr);
			}
			
			inline static void RenderPassEnd(SDL_GPURenderPass* renderPass) {
				SDL_EndGPURenderPass(renderPass);
			}
			
			inline static void PipeVertices(SDL_GPURenderPass* renderPass, SDL_GPUGraphicsPipeline* graphics_pipeline, xy2d_buffer& vertexBuffer) {
				SDL_BindGPUGraphicsPipeline(renderPass, graphics_pipeline);
				SDL_GPUBufferBinding vertexBinding = { .buffer = (SDL_GPUBuffer*) vertexBuffer.buffer };
				SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBinding, 1);
			}
			
			inline static void DrawVertices(SDL_GPURenderPass* renderPass, uint32_t first_vrtx, uint32_t count_vrtx, uint32_t first_inst, uint32_t count_inst) {
				SDL_DrawGPUPrimitives(renderPass, count_vrtx, count_inst, first_vrtx, first_inst);
			}
			
			inline static void BindVertexUniforms(SDL_GPUCommandBuffer* cmdbuffer, void* uniformData, size_t uniformSize) {
				SDL_PushGPUVertexUniformData(cmdbuffer, 0U, uniformData, uniformSize);
			}
			
			inline static void BindFragmentUniforms(SDL_GPUCommandBuffer* cmdbuffer, void* uniformData, size_t uniformSize) {
				SDL_PushGPUFragmentUniformData(cmdbuffer, 0U, uniformData, uniformSize);
			}
			
			inline static void BindFragmentTextures(SDL_GPURenderPass* renderPass, const std::vector<SDL_GPUTexture*> textures) {
				SDL_BindGPUFragmentStorageTextures(renderPass, 0U, textures.data(), textures.size());
			}
			
			inline static SDL_GPUComputePass* ComputePassBegin(SDL_GPUCommandBuffer* cmdbuffer, const std::vector<SDL_GPUTexture*> writeTextures, const std::vector<SDL_GPUBuffer*> writeBuffers) {
				std::vector<SDL_GPUStorageTextureReadWriteBinding> textbindings(writeTextures.size());
				for(size_t i = 0; i < writeTextures.size(); i++)
					textbindings[i] = { .mip_level = 0, .layer = 0, .cycle = false, .texture = writeTextures[i] };
				
				std::vector<SDL_GPUStorageBufferReadWriteBinding> buffbindings(writeBuffers.size());
				for(size_t i = 0; i < writeBuffers.size(); i++)
					buffbindings[i] = { .cycle = false, .buffer = writeBuffers[i] };
				
				SDL_GPUComputePass* computePass = SDL_BeginGPUComputePass(cmdbuffer, textbindings.data(), textbindings.size(), buffbindings.data(), buffbindings.size());
				return computePass;
			}
			
			inline static void ComputePassEnd(SDL_GPUComputePass* computePass) {
				SDL_EndGPUComputePass(computePass);
			}
			
			inline static void BindComputeDispatch(SDL_GPUComputePass* computePass, SDL_GPUComputePipeline* pipeline, glm::ivec3 threads) {
				SDL_BindGPUComputePipeline(computePass, pipeline);
				SDL_DispatchGPUCompute(computePass, threads.x, threads.y, threads.z);
			}
			
			inline static void BindComputeUniforms(SDL_GPUCommandBuffer* cmdbuffer, void* uniformData, size_t uniformSize) {
				SDL_PushGPUComputeUniformData(cmdbuffer, 0U, uniformData, uniformSize);
			}
			
			inline static void BindComputeTextures(SDL_GPUComputePass* computePass, const std::vector<SDL_GPUTexture*> textures) {
				SDL_BindGPUComputeStorageTextures(computePass, 0U, textures.data(), textures.size());
			}
			
			inline static void BindComputeBuffers(SDL_GPUComputePass* computePass, const std::vector<SDL_GPUBuffer*> buffers) {
				SDL_BindGPUComputeStorageBuffers(computePass, 0U, buffers.data(), buffers.size());
			}
		};
	}
#endif