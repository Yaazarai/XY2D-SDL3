#include ".\SDL3-XY2D\xy2d_engine.hpp"
using namespace xy2d;

class point_sweep {
public:
	inline static SDL_GPUGraphicsPipeline* scene_pipeline;
	inline static SDL_GPUComputePipeline* pointsweep_pipeline;
	inline static SDL_GPUGraphicsPipeline* present_pipeline;
	inline static xy2d_image emission, absorption;
	inline static xy2d_image fluences, radiance, transmit;
	inline static xy2d_buffer vertexBuffer, transferBuffer;
	inline static xy2d_sprite sceneSpriteA;
	inline static xy2d_sprite sceneSpriteB;
	
	inline static glm::ivec2 mouse_position1;
	inline static glm::ivec2 mouse_position2;
	inline static glm::vec2 extent;
	
	inline static struct UBO {
		glm::vec2 renderSize, circlePosA, circlePosB;
		glm::float32 circleRadius;
	} sceneUBO;
	
	inline static void preRenderFunction(SDL_GPUCommandBuffer* cmdbuffer) {
		SDL_GPURenderPass* renderPassA = xy2d_renderer::RenderPassBegin(cmdbuffer, { emission.texture, absorption.texture }, { 0.0, 0.0, 0.0, 1.0 }, SDL_GPU_LOADOP_CLEAR);
		glm::mat4 cameraData = xy2d_renderer::CameraTransform(extent, glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0));
		xy2d_renderer::BindVertexUniforms(cmdbuffer, &cameraData, sizeof(cameraData));
		xy2d_renderer::BindFragmentUniforms(cmdbuffer, &sceneUBO, sizeof(sceneUBO));
		xy2d_renderer::PipeVertices(renderPassA, scene_pipeline, vertexBuffer);
		xy2d_renderer::DrawVertices(renderPassA, sceneSpriteA.batchIndex * std::size(sceneSpriteA.vertices), 6, 0, 1);
		xy2d_renderer::RenderPassEnd(renderPassA);
	}
	
	inline static void preRenderPointSweep(SDL_GPUCommandBuffer* cmdbuffer) {
		SDL_GPUComputePass* computePass = xy2d_renderer::ComputePassBegin(cmdbuffer, { radiance.texture, transmit.texture, fluences.texture }, {});
		//xy2d_renderer::BindComputeTextures(computePass, { emission.texture, absorption.texture });
		int32_t xx = fluences.width / 32;
		int32_t yy = fluences.height / 32;
		xy2d_renderer::BindComputeDispatch(computePass, pointsweep_pipeline, glm::ivec3(xx, yy, 1));
		xy2d_renderer::ComputePassEnd(computePass);
	}
	
	inline static void presentFunction(SDL_GPUCommandBuffer* cmdbuffer, SDL_GPUTexture* swapImage, SDL_GPURenderPass* renderpass, glm::uint32_t& swapImageWidth, glm::uint32_t& swapImageHeight) {
		SDL_GPURenderPass* renderPassB = xy2d_renderer::RenderPassBegin(cmdbuffer, { swapImage }, { 0.0, 0.0, 0.0, 1.0 }, SDL_GPU_LOADOP_CLEAR);
		glm::mat4x4 cameraData = xy2d_renderer::CameraTransform(glm::vec2(xy2d_gamestate::windowSize), glm::vec2(0.0, 0.0), glm::vec2(1.0, 1.0), 0.0f /*0.78539816339744830961566084581988f*/);
		xy2d_renderer::BindVertexUniforms(cmdbuffer, &cameraData, sizeof(cameraData));
		xy2d_renderer::BindFragmentTextures(renderPassB, { absorption.texture });
		xy2d_renderer::PipeVertices(renderPassB, present_pipeline, vertexBuffer);
		xy2d_renderer::DrawVertices(renderPassB, sceneSpriteB.batchIndex * std::size(sceneSpriteB.vertices), 6, 0, 1);
		xy2d_renderer::RenderPassEnd(renderPassB);
	}
	
	inline static void onResizeSceneTarget(SDL_Event* event) {
		if (event->type != SDL_EVENT_WINDOW_RESIZED) return;
		xy2d_renderer::ResizeEvent(event, &absorption);
		xy2d_renderer::ResizeEvent(event, &emission);
		
		extent = glm::vec2(absorption.width, absorption.height);
		glm::vec2 xyscale = glm::vec2(xy2d_gamestate::windowSize) / extent;
		sceneSpriteA.Size(extent);
		sceneSpriteB.Size(extent).Scale(xyscale);
		
		xy2d_sprite_manager::BatchVerticesStageBuffer(&transferBuffer);
		xy2d_gamestate::TransferBuffer(&transferBuffer, &vertexBuffer);
	}
	
	inline static void Initialize() {
		extent = glm::vec2(256.0f, 256.0f);
		sceneUBO.renderSize = extent;
		sceneUBO.circleRadius = 16.0F;
		sceneUBO.circlePosA = glm::vec2(0.0, 0.0);
		sceneUBO.circlePosB = glm::vec2(0.0, 0.0);
		
		// User-Scene input:
		emission = xy2d_gamestate::CreateTexture(SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT, extent.x, extent.y);
		absorption = xy2d_gamestate::CreateTexture(SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT, extent.x, extent.y);
		// Single-Column N^2 directional samples: N pixels, N directions:
		radiance = xy2d_gamestate::CreateTexture(SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT, extent.x, extent.y);
		transmit = xy2d_gamestate::CreateTexture(SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT, extent.x, extent.y);
		// Output accumulated fluence:
		fluences = xy2d_gamestate::CreateTexture(SDL_GPU_TEXTUREFORMAT_R11G11B10_UFLOAT, extent.x, extent.y);
		
		// Render pipelines: User-Scene -> Point-Sweep -> Present-Screen:
		scene_pipeline = xy2d_gamestate::GraphicsPipeline("Shaders/default_output_vert.spv", 1, "Shaders/circle_sdf_frag.spv", 0, 1, 0, 2);
		pointsweep_pipeline = xy2d_gamestate::ComputePipeline("Shaders/point_sweep_gi_comp.spv", 3, 0, 0, 0, glm::ivec3(32, 32, 1));
		present_pipeline = xy2d_gamestate::GraphicsPipeline("Shaders/default_output_vert.spv", 1, "Shaders/texture_output_frag.spv", 1, 0, 0, 1);
		
		point_sweep::sceneSpriteA = xy2d_sprite::CreateSprite({0,0,extent.x,extent.y}, {0.0,0.0,1.0,1.0}, {1.0,1.0}, {0,0}, 0.0, 0.0);
		point_sweep::sceneSpriteB = xy2d_sprite::CreateSprite({0,0,extent.x,extent.y}, {0.0,0.0,1.0,1.0}, {1.0,1.0}, {0,0}, 0.0, 0.0);
		xy2d_sprite_manager::BatchSpriteList({ point_sweep::sceneSpriteA, point_sweep::sceneSpriteB });
		
		size_t spriteVertexSize = xy2d_sprite_manager::BatchVerticesSize();
		vertexBuffer = xy2d_gamestate::CreateBuffer(spriteVertexSize, xy2d_buffertype::STORAGE);
		transferBuffer = xy2d_gamestate::CreateBuffer(spriteVertexSize, xy2d_buffertype::TRANSFER_GPU);
		
		xy2d_sprite_manager::BatchVerticesStageBuffer(&transferBuffer);
		xy2d_gamestate::TransferBuffer(&transferBuffer, &vertexBuffer);
		
		xy2d_gamestate::onGameEvent.hook(xy2d_event_callback(point_sweep::gameEvent));
		xy2d_gamestate::onGamePreRender.hook(xy2d_prerender_callback(point_sweep::preRenderFunction));
		xy2d_gamestate::onGamePreRender.hook(xy2d_prerender_callback(point_sweep::preRenderPointSweep));
		xy2d_gamestate::onGameRender.hook(xy2d_render_callback(point_sweep::presentFunction));
		xy2d_gamestate::onGameEvent.hook(xy2d_event_callback(point_sweep::onResizeSceneTarget));
	}
	
	inline static void gameEvent(SDL_Event* event) {
		if (event->type == SDL_EVENT_MOUSE_BUTTON_UP) {
			if (event->button.button == SDL_BUTTON_LEFT && !event->button.down) {
				mouse_position1 = xy2d_gamestate::MousePosition();
				
				glm::vec2 scale = glm::vec2(sceneSpriteB.xyscale.x, sceneSpriteB.xyscale.y);
				sceneUBO.circlePosA = glm::ivec2(glm::vec2(mouse_position1) * (1.0f / scale));
				std::cout << "MOUSE [1] CLICK: " << sceneUBO.circlePosA[0] << " : " << sceneUBO.circlePosA[1] << std::endl;
			}
			
			if (event->button.button == SDL_BUTTON_RIGHT && !event->button.down) {
				mouse_position2 = xy2d_gamestate::MousePosition();
				
				glm::vec2 scale = glm::vec2(sceneSpriteB.xyscale.x, sceneSpriteB.xyscale.y);
				sceneUBO.circlePosB = glm::ivec2(glm::vec2(mouse_position2) * (1.0f / scale));
				std::cout << "MOUSE [2] CLICK: " << sceneUBO.circlePosB[0] << " : " << sceneUBO.circlePosB[1] << std::endl;
			}
		}
		
		sceneUBO.renderSize = extent;
		sceneUBO.circleRadius = 2.0F;
	}
};

void xy2d_gamestate::GameWindowInit() {
	xy2d_gamestate::GameWindowSize(1024, 1024);
	xy2d_gamestate::onGameStart.hook(xy2d_default_callback(point_sweep::Initialize));
}

/*
	Point Sweeping is an O(N^3) operation. Accumulate N directional samples holographically
	sweeping radiance for all columns from Left -> Right of all N x N pixels across the scene.
	
	TODO:
		1. Write first compute shader -> CA GI shader as a follow up.
			1.2 Output each shader/test visualization by presenting to screen directly.
*/