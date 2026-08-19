#pragma once
#ifndef __XY2D_SPRITE
#define __XY2D_SPRITE
	#include ".\xy2d_engine.hpp"
	
	namespace XY2D_NAMESPACE {
		class xy2d_sprite {
		public:
			xy2d_vertex vertices[6] = {
				{ glm::vec3(0.0, 0.0, 0.0), glm::vec2(0.0, 0.0) },
				{ glm::vec3(1.0, 0.0, 0.0), glm::vec2(1.0, 0.0) },
				{ glm::vec3(1.0, 1.0, 0.0), glm::vec2(1.0, 1.0) },
				{ glm::vec3(0.0, 0.0, 0.0), glm::vec2(0.0, 0.0) },
				{ glm::vec3(1.0, 1.0, 0.0), glm::vec2(1.0, 1.0) },
				{ glm::vec3(0.0, 1.0, 0.0), glm::vec2(0.0, 1.0) },
			};
			
			glm::vec4 xywh = glm::vec4(0.0, 0.0, 1.0, 1.0);
			glm::vec4 uvwh = glm::vec4(0.0, 0.0, 1.0, 1.0);
			glm::vec2 xyscale = glm::vec2(1.0, 1.0);
			glm::vec2 xyorigin = glm::vec2(0.0, 0.0);
			glm::float32_t depth = 0.0;
			glm::float32_t theta = 0.0;
			
			xy2d_sprite() {};
			xy2d_sprite(glm::vec4& xywh, glm::vec4& uvwh, glm::vec2& xyscale, glm::vec2& xyorigin, glm::float32_t& depth, glm::float32_t& theta)
				: xywh(xywh), uvwh(uvwh), xyscale(xyscale), xyorigin(xyorigin), depth(depth), theta(theta) { Update(); };
			
			void Update() {
				vertices[0] = { glm::vec3(xywh.x         , xywh.y         , depth), glm::vec2(uvwh.x         , uvwh.y         ) };
				vertices[1] = { glm::vec3(xywh.x + xywh.z, xywh.y         , depth), glm::vec2(uvwh.x + uvwh.z, uvwh.y         ) };
				vertices[4] = { glm::vec3(xywh.x + xywh.z, xywh.y + xywh.w, depth), glm::vec2(uvwh.x + uvwh.z, uvwh.y + uvwh.w) };
				vertices[5] = { glm::vec3(xywh.x         , xywh.y + xywh.w, depth), glm::vec2(uvwh.x         , uvwh.y + uvwh.w) };
				
				glm::mat2 rotmatrix = glm::mat2(glm::cos(theta), -glm::sin(theta), glm::sin(theta), glm::cos(theta));
				
				for(size_t i = 0, corner[4] = { 0, 1, 4, 5 }; i < std::size(corner); i++) {
					glm::vec2 xypos = glm::vec2(vertices[corner[i]].xyz);
					xypos -= glm::vec2(xywh);
					xypos = rotmatrix * ((xypos - glm::vec2(xyorigin.x, xyorigin.y)) * xyscale);
					xypos += glm::vec2(xywh);
					vertices[corner[i]].xyz = glm::vec3(xypos, depth);
				}
				
				vertices[2] = vertices[4];
				vertices[3] = vertices[0];
			}
			
			void Position(glm::vec4 xywh) { this->xywh = xywh; }
			void Origin(glm::vec2 xyorigin) { this->xyorigin = xyorigin; }
			void Scale(glm::vec2 xyscale) { this->xyscale = xyscale; }
			void Rotate(glm::float32 theta) { this->theta = theta; }
			void Texture(glm::vec4 uvwh) { this->uvwh = uvwh; }
			
			inline static xy2d_sprite CreateSprite(glm::vec4 xywh, glm::vec4 uvwh, glm::vec2 xyscale, glm::vec2 xyorigin, glm::float32_t depth, glm::float32_t theta) {
				return xy2d_sprite(xywh, uvwh, xyscale, xyorigin, depth, theta);
			}
			
			inline static glm::vec4 GetUVCoords(glm::vec4 xywh, glm::vec2 textsize) {
				return glm::vec4(xywh.x, xywh.y, xywh.x + xywh.z, xywh.y + xywh.w) / glm::vec4(textsize.x, textsize.y, textsize.x, textsize.y);
			}
		};
		
		class xy2d_sprite_manager {
		public:
			inline static std::vector<xy2d_sprite> spriteBatch;
			
			inline static void BatchDepthSort() {
				std::sort(spriteBatch.begin(), spriteBatch.end(), [](const xy2d_sprite& A, const xy2d_sprite& B) { return A.depth < B.depth; });
			}
			
			inline static size_t BatchVerticesSize() {
				return (spriteBatch.size() > 0)? std::size(spriteBatch[0].vertices) * sizeof(xy2d_vertex) * spriteBatch.size() : 0;
			}
			
			inline static bool BatchVerticesStageBuffer(xy2d_buffer* transferBuffer) {
				uint8_t* memory = (uint8_t*) SDL_MapGPUTransferBuffer(xy2d_gamestate::device, (SDL_GPUTransferBuffer*) transferBuffer->buffer, false);
				
				if (memory != nullptr) {
					for(size_t i = 0; i < spriteBatch.size(); i++) {
						spriteBatch[i].Update();
						size_t vertexSize = sizeof(spriteBatch[i].vertices);
						SDL_memcpy(memory + (i * vertexSize), spriteBatch[i].vertices, vertexSize);
					}
					
					SDL_UnmapGPUTransferBuffer(xy2d_gamestate::device, (SDL_GPUTransferBuffer*) transferBuffer->buffer);
				}
				
				return (memory != nullptr);
			}
			
			inline static xy2d_image SpriteSheetLoad(const char* file) {
				SDL_Surface* png = SDL_LoadPNG(file);
				xy2d_image image = {};
				
				if (png->format == SDL_PIXELFORMAT_RGBA32) {
					image = xy2d_gamestate::CreateTexture(SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM, png->w, png->h);
					
					size_t sizeofPixels = (png->h * png->pitch);
					xy2d_buffer transfer = xy2d::xy2d_gamestate::CreateBuffer(sizeofPixels, xy2d::xy2d_buffertype::TRANSFER_GPU);
					xy2d::xy2d_gamestate::StageBuffer(&transfer, png->pixels, sizeofPixels);
					
					SDL_GPUCommandBuffer* cmdBuffer = SDL_AcquireGPUCommandBuffer(xy2d_gamestate::device);
						SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);
							SDL_GPUTextureTransferInfo transferInfo = {
								.transfer_buffer = (SDL_GPUTransferBuffer*) transfer.buffer,
								.offset = 0,
								.pixels_per_row = 0,
            					.rows_per_layer = 0,
							};
							SDL_GPUTextureRegion transferRegion = {
								.texture = image.texture, .w = static_cast<uint32_t>(png->w), .h = static_cast<uint32_t>(png->h), .d = 1
							};
							SDL_UploadToGPUTexture(copyPass, &transferInfo, &transferRegion, false);
						SDL_EndGPUCopyPass(copyPass);
					SDL_SubmitGPUCommandBuffer(cmdBuffer);
					SDL_ReleaseGPUTransferBuffer(xy2d::xy2d_gamestate::device, (SDL_GPUTransferBuffer*) transfer.buffer);
				}
				
				SDL_DestroySurface(png);
				return image;
			}
		};
	}
#endif