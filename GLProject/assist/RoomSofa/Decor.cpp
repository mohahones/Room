#pragma once

#include "../Shared/RenderUtils.cpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace scene
{
	struct TexturedVertex
	{
		glm::vec3 position;
		glm::vec2 texCoord;
	};

	class Decor
	{ 
	private:
		std::unique_ptr<PolygonWithTexture> shadowQuad;
		std::unique_ptr<PolygonWithTexture> metalMesh;
		std::unique_ptr<PolygonWithTexture> shadeMesh;
		std::unique_ptr<PolygonWithTexture> shelfMesh;
		std::array<std::unique_ptr<PolygonWithTexture>, 4> bookMeshes;
		std::unique_ptr<PolygonWithTexture> bookTrimMesh;
		std::unique_ptr<PolygonWithTexture> posterMesh;
		GLuint metalTexture = 0;
		GLuint shadeTexture = 0;
		GLuint shadowTexture = 0;
		GLuint posterTexture = 0;

		static constexpr float ShelfX = 13.0f;
		static constexpr float ShelfZ = -12.9f;
		static constexpr float ShelfFloorY = -1.5f;
		static constexpr float ShelfWidth = 5.8f;
		static constexpr float ShelfDepth = 1.5f;
		static constexpr float ShelfHeight = 6.2f;
		static constexpr float ShelfPanelWidth = 0.18f;
		static constexpr float ShelfBoardHeight = 0.12f;
		static constexpr float FloorLampX = -6.35f;
		static constexpr float FloorLampZ = -13.3f;
		static constexpr float FloorLampFloorY = -1.5f;
		const std::array<glm::vec3, 4> bookColors = {
			glm::vec3(0.78f, 0.09f, 0.12f),
			glm::vec3(0.92f, 0.58f, 0.13f),
			glm::vec3(0.10f, 0.46f, 0.23f),
			glm::vec3(0.9f, 0.9f, 0.83f)};

	public:
		void initialize()
		{
			shadowQuad = createTexturedQuad();
			shadowTexture = createContactShadowTexture();
			metalTexture = loadTexture("./textures/blue_metal_plate_diff_1k.jpg");
			shadeTexture = loadTexture("./textures/denim_fabric_04_diff_1k.jpg");
			posterTexture = loadTexture("./textures/poster.jpg");
			glBindTexture(GL_TEXTURE_2D, posterTexture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			initializePoster();

			std::vector<TexturedVertex> metalVertices;
			appendFrustum(metalVertices, 0.34f, 0.30f, 0.0f, 0.12f, 24, 2.0f);
			appendFrustum(metalVertices, 0.055f, 0.045f, 0.11f, 1.98f, 16, 1.0f, false);
			appendFrustum(metalVertices, 0.09f, 0.09f, 1.89f, 2.03f, 16, 1.0f);
			appendFrustum(metalVertices, 0.485f, 0.485f, 1.95f, 2.02f, 24, 2.0f);
			appendFrustum(metalVertices, 0.285f, 0.285f, 2.64f, 2.71f, 24, 2.0f);
			metalMesh = createTriangleMesh(metalVertices);

			std::vector<TexturedVertex> shadeVertices;
			appendFrustum(shadeVertices, 0.45f, 0.26f, 1.98f, 2.68f, 20, 3.0f);
			shadeMesh = createTriangleMesh(shadeVertices);

			std::vector<TexturedVertex> shelfVertices;
			const float shelfCenterY = ShelfHeight * 0.5f;
			appendBox(shelfVertices,
					  glm::vec3(-ShelfWidth * 0.5f + ShelfPanelWidth * 0.5f, shelfCenterY, 0.0f),
					  glm::vec3(ShelfPanelWidth, ShelfHeight, ShelfDepth));
			appendBox(shelfVertices,
					  glm::vec3(ShelfWidth * 0.5f - ShelfPanelWidth * 0.5f, shelfCenterY, 0.0f),
					  glm::vec3(ShelfPanelWidth, ShelfHeight, ShelfDepth));
			appendBox(shelfVertices,
					  glm::vec3(0.0f, shelfCenterY, -ShelfDepth * 0.5f - 0.05f),
					  glm::vec3(ShelfWidth - ShelfPanelWidth * 2.0f, ShelfHeight, 0.1f));
			const std::array<float, 6> shelfLevels = {
				0.08f, 1.30f, 2.52f, 3.74f, 4.96f,
				ShelfHeight - ShelfBoardHeight * 0.5f};
			for (float y : shelfLevels)
				appendBox(shelfVertices, glm::vec3(0.0f, y, 0.0f),
						  glm::vec3(ShelfWidth, ShelfBoardHeight, ShelfDepth));
			shelfMesh = createTriangleMesh(shelfVertices);

			std::array<std::vector<TexturedVertex>, 4> bookVertices;
			std::vector<TexturedVertex> bookTrimVertices;
			const auto addBook = [&](size_t color, float x, float shelfY, float width, float height)
			{
				appendBook(bookVertices[color], bookTrimVertices,
						   glm::vec3(x, shelfY + ShelfBoardHeight * 0.5f + height * 0.5f, 0.42f),
						   glm::vec3(width, height, 0.64f));
			};
			constexpr size_t BooksPerShelf = 32;
			constexpr float BookGap = 0.012f;
			constexpr float MinimumBookWidth = 0.14f;
			constexpr float BookWidthVariation = 0.01f;
			constexpr float InnerHalfWidth = (ShelfWidth - ShelfPanelWidth * 2.0f) * 0.5f;
			float totalBookWidth = 0.0f;
			for (size_t i = 0; i < BooksPerShelf; ++i)
				totalBookWidth += MinimumBookWidth +
								  BookWidthVariation * static_cast<float>(i % 4);
			const float rowWidth = totalBookWidth + BookGap * (BooksPerShelf - 1);
			const float rowStart = -InnerHalfWidth + (InnerHalfWidth * 2.0f - rowWidth) * 0.5f;

			for (size_t row = 0; row < 5; ++row)
			{
				float bookLeft = rowStart;
				for (size_t i = 0; i < BooksPerShelf; ++i)
				{
					const float bookWidth = MinimumBookWidth +
											BookWidthVariation * static_cast<float>(i % 4);
					const float bookHeight = 0.84f + 0.035f * static_cast<float>(i % 5);
					addBook((i + row * 2) % bookVertices.size(),
							bookLeft + bookWidth * 0.5f, shelfLevels[row], bookWidth, bookHeight);
					bookLeft += bookWidth + BookGap;
				}
			}

			for (size_t i = 0; i < bookMeshes.size(); ++i)
				bookMeshes[i] = createTriangleMesh(bookVertices[i], bookColors[i]);
			bookTrimMesh = createTriangleMesh(
				bookTrimVertices, glm::vec3(0.9f, 0.62f, 0.18f));
		}

		void render(Shader &shader)
		{
			if (!shadowQuad)
				throw std::logic_error("Decor polygons were not initialized.");

			drawContactShadows(shader);
			drawPoster(shader);
			drawFloorShelf(shader);
			drawFloorLamp(shader);
		}

		void cleanup()
		{
			if (shadowQuad)
				shadowQuad->deleteBuffers();
			shadowQuad.reset();
			deleteMesh(metalMesh);
			deleteMesh(shadeMesh);
			deleteMesh(shelfMesh);
			deleteMesh(bookTrimMesh);
			for (auto &bookMesh : bookMeshes)
				deleteMesh(bookMesh);
			if (shadowTexture)
				glDeleteTextures(1, &shadowTexture);
			if (metalTexture)
				glDeleteTextures(1, &metalTexture);
			if (shadeTexture)
				glDeleteTextures(1, &shadeTexture);
			if (posterTexture)
				glDeleteTextures(1, &posterTexture);
			metalTexture = 0;
			shadeTexture = 0;
			shadowTexture = 0;
			posterTexture = 0;
			deleteMesh(posterMesh);
		}

	private:
		void initializePoster()
		{
			constexpr float PosterLeft = -2.05f;
			constexpr float PosterRight = 2.05f;
			constexpr float PosterBottom = 0.85f;
			constexpr float PosterTop = 5.65f;
			constexpr float PosterWidth = PosterRight - PosterLeft;
			constexpr float PosterHeight = PosterTop - PosterBottom;
			constexpr float PosterImageAspect = 614.0f / 773.0f;
			const float visibleImageHeight = PosterWidth / PosterHeight;
			const float imageVInset =
				(1.0f - PosterImageAspect / visibleImageHeight) * 0.5f;
			const float z = -13.88f;
			const std::array<float, 14> rightEdge = {
				PosterRight, PosterRight, 1.94f, 2.14f, 1.96f, 2.12f,
				1.93f, 2.13f, 1.95f, 2.10f, 1.94f, 2.13f,
				PosterRight, PosterRight};

			std::vector<TexturedVertex> paperVertices;
			paperVertices.reserve((rightEdge.size() - 1) * 6);
			for (size_t i = 0; i + 1 < rightEdge.size(); ++i)
			{
				const float bottomY = PosterBottom + PosterHeight *
														 static_cast<float>(i) / static_cast<float>(rightEdge.size() - 1);
				const float topY = PosterBottom + PosterHeight *
													  static_cast<float>(i + 1) / static_cast<float>(rightEdge.size() - 1);
				const TexturedVertex bottomLeft = makeVertex(
					glm::vec3(PosterLeft, bottomY, z),
					glm::vec2(0.0f,
							  imageVInset + (1.0f - 2.0f * imageVInset) *
												(bottomY - PosterBottom) / PosterHeight));
				const TexturedVertex bottomRight = makeVertex(
					glm::vec3(rightEdge[i], bottomY, z),
					glm::vec2(std::clamp((rightEdge[i] - PosterLeft) / PosterWidth,
										 0.0f, 1.0f),
							  imageVInset + (1.0f - 2.0f * imageVInset) *
												(bottomY - PosterBottom) / PosterHeight));
				const TexturedVertex topRight = makeVertex(
					glm::vec3(rightEdge[i + 1], topY, z),
					glm::vec2(std::clamp((rightEdge[i + 1] - PosterLeft) / PosterWidth,
										 0.0f, 1.0f),
							  imageVInset + (1.0f - 2.0f * imageVInset) *
												(topY - PosterBottom) / PosterHeight));
				const TexturedVertex topLeft = makeVertex(
					glm::vec3(PosterLeft, topY, z),
					glm::vec2(0.0f,
							  imageVInset + (1.0f - 2.0f * imageVInset) *
												(topY - PosterBottom) / PosterHeight));
				appendTriangle(paperVertices, bottomLeft, bottomRight, topRight);
				appendTriangle(paperVertices, bottomLeft, topRight, topLeft);
			}
			posterMesh = createTriangleMesh(paperVertices);
		}

		void drawPoster(Shader &shader)
		{
			if (!posterTexture || !posterMesh)
				throw std::logic_error("Poster was not initialized.");

			shader.setBool("useTexture", true);
			shader.setVec2("textureScale", glm::vec2(1.0f));
			posterMesh->setTexture(posterTexture);
			posterMesh->transformation(glm::mat4(1.0f));
			glActiveTexture(GL_TEXTURE0);
			posterMesh->draw(shader);
		}

		void drawContactShadows(Shader &shader)
		{
			if (!shadowTexture || !shadowQuad)
				throw std::logic_error("Decor contact shadow was not initialized.");

			glDepthMask(GL_FALSE);
			drawHorizontalTexture(shader, *shadowQuad, shadowTexture,
								  glm::vec3(ShelfX, ShelfFloorY + 0.004f, ShelfZ),
								  glm::vec2(ShelfWidth + 0.72f, ShelfDepth + 0.62f));
			drawHorizontalTexture(shader, *shadowQuad, shadowTexture,
								  glm::vec3(FloorLampX, FloorLampFloorY + 0.004f, FloorLampZ),
								  glm::vec2(1.52f, 1.52f));
			glDepthMask(GL_TRUE);
		}

		void drawFloorShelf(Shader &shader)
		{
			const glm::mat4 model = glm::translate(glm::mat4(1.0f),
												   glm::vec3(ShelfX, ShelfFloorY, ShelfZ));
			drawMesh(shader, *shelfMesh, metalTexture, model);
			for (size_t i = 0; i < bookMeshes.size(); ++i)
				drawMesh(shader, *bookMeshes[i], 0, model);
			drawMesh(shader, *bookTrimMesh, 0, model);
		}

		void drawFloorLamp(Shader &shader)
		{
			const glm::mat4 model = glm::translate(glm::mat4(1.0f),
												   glm::vec3(FloorLampX, FloorLampFloorY, FloorLampZ));
			drawMesh(shader, *metalMesh, metalTexture, model);
			drawMesh(shader, *shadeMesh, shadeTexture, model);
		}

		static std::unique_ptr<PolygonWithTexture> createTriangleMesh(
			const std::vector<TexturedVertex> &vertices,
			const glm::vec3 &color = glm::vec3(1.0f))
		{
			if (vertices.empty())
				throw std::logic_error("Cannot initialize an empty textured mesh.");

			std::vector<glm::vec3> positions;
			std::vector<glm::vec2> texCoords;
			positions.reserve(vertices.size());
			texCoords.reserve(vertices.size());
			for (const TexturedVertex &vertex : vertices)
			{
				positions.push_back(vertex.position);
				texCoords.push_back(vertex.texCoord);
			}
			return std::make_unique<PolygonWithTexture>(
				std::move(positions), std::move(texCoords), color, GL_TRIANGLES);
		}

		static void drawMesh(Shader &shader, PolygonWithTexture &mesh, GLuint texture,
							 const glm::mat4 &model)
		{
			shader.setBool("useTexture", texture != 0);
			shader.setVec2("textureScale", glm::vec2(1.0f));
			mesh.setTexture(texture);
			mesh.transformation(model);
			glActiveTexture(GL_TEXTURE0);
			mesh.draw(shader);
		}

		static void deleteMesh(std::unique_ptr<PolygonWithTexture> &mesh)
		{
			if (mesh)
			{
				mesh->deleteBuffers();
				mesh.reset();
			}
		}

		static TexturedVertex makeVertex(const glm::vec3 &position,
										 const glm::vec2 &texCoord)
		{
			return {position, texCoord};
		}

		static void appendTriangle(std::vector<TexturedVertex> &vertices,
								   const TexturedVertex &a, const TexturedVertex &b,
								   const TexturedVertex &c)
		{
			vertices.push_back(a);
			vertices.push_back(b);
			vertices.push_back(c);
		}

		static void appendFace(std::vector<TexturedVertex> &vertices,
							   const glm::vec3 &bottomLeft, const glm::vec3 &bottomRight,
							   const glm::vec3 &topRight, const glm::vec3 &topLeft,
							   const glm::vec2 &uvSize)
		{
			const TexturedVertex a = makeVertex(bottomLeft, glm::vec2(0.0f));
			const TexturedVertex b = makeVertex(bottomRight,
												glm::vec2(uvSize.x, 0.0f));
			const TexturedVertex c = makeVertex(topRight, uvSize);
			const TexturedVertex d = makeVertex(topLeft,
												glm::vec2(0.0f, uvSize.y));
			appendTriangle(vertices, a, b, c);
			appendTriangle(vertices, a, c, d);
		}

		static void appendBox(std::vector<TexturedVertex> &vertices,
							  const glm::vec3 &center, const glm::vec3 &size)
		{
			const glm::vec3 half = size * 0.5f;
			const float left = center.x - half.x;
			const float right = center.x + half.x;
			const float bottom = center.y - half.y;
			const float top = center.y + half.y;
			const float back = center.z - half.z;
			const float front = center.z + half.z;

			appendFace(vertices,
					   glm::vec3(left, bottom, front), glm::vec3(right, bottom, front),
					   glm::vec3(right, top, front), glm::vec3(left, top, front),
					   glm::vec2(size.x, size.y));
			appendFace(vertices,
					   glm::vec3(left, bottom, back), glm::vec3(right, bottom, back),
					   glm::vec3(right, top, back), glm::vec3(left, top, back),
					   glm::vec2(size.x, size.y));
			appendFace(vertices,
					   glm::vec3(left, bottom, back), glm::vec3(left, bottom, front),
					   glm::vec3(left, top, front), glm::vec3(left, top, back),
					   glm::vec2(size.z, size.y));
			appendFace(vertices,
					   glm::vec3(right, bottom, front), glm::vec3(right, bottom, back),
					   glm::vec3(right, top, back), glm::vec3(right, top, front),
					   glm::vec2(size.z, size.y));
			appendFace(vertices,
					   glm::vec3(left, top, back), glm::vec3(right, top, back),
					   glm::vec3(right, top, front), glm::vec3(left, top, front),
					   glm::vec2(size.x, size.z));
			appendFace(vertices,
					   glm::vec3(left, bottom, front), glm::vec3(right, bottom, front),
					   glm::vec3(right, bottom, back), glm::vec3(left, bottom, back),
					   glm::vec2(size.x, size.z));
		}

		static void appendBook(std::vector<TexturedVertex> &vertices,
							   std::vector<TexturedVertex> &trimVertices, const glm::vec3 &center,
							   const glm::vec3 &size)
		{
			appendBox(vertices, center, size);
			const float front = center.z + size.z * 0.5f + 0.006f;
			appendBox(trimVertices,
					  glm::vec3(center.x, center.y + size.y * 0.34f, front),
					  glm::vec3(size.x * 0.72f, 0.018f, 0.012f));
			appendBox(trimVertices,
					  glm::vec3(center.x, center.y - size.y * 0.34f, front),
					  glm::vec3(size.x * 0.72f, 0.018f, 0.012f));
			appendBox(trimVertices,
					  glm::vec3(center.x - size.x * 0.36f, center.y, front + 0.006f),
					  glm::vec3(0.012f, size.y * 0.84f, 0.012f));
		}

		static GLuint createContactShadowTexture()
		{
			constexpr int ShadowSize = 128;
			std::vector<unsigned char> pixels(ShadowSize * ShadowSize * 4);
			for (int y = 0; y < ShadowSize; ++y)
				for (int x = 0; x < ShadowSize; ++x)
				{
					const float dx = (static_cast<float>(x) + 0.5f) / ShadowSize * 2.0f - 1.0f;
					const float dy = (static_cast<float>(y) + 0.5f) / ShadowSize * 2.0f - 1.0f;
					const float radius = std::sqrt(dx * dx + dy * dy);
					const float fade = std::clamp((1.0f - radius) / 0.7f, 0.0f, 1.0f);
					const float smoothFade = fade * fade * (3.0f - 2.0f * fade);
					const unsigned char alpha = static_cast<unsigned char>(
						88.0f * smoothFade * std::exp(-radius * radius * 1.35f));
					const size_t index = static_cast<size_t>((y * ShadowSize + x) * 4);
					pixels[index] = 15;
					pixels[index + 1] = 19;
					pixels[index + 2] = 27;
					pixels[index + 3] = alpha;
				}

			GLuint texture = 0;
			glGenTextures(1, &texture);
			if (!texture)
				throw std::runtime_error("Failed to allocate the decor shadow texture.");
			glBindTexture(GL_TEXTURE_2D, texture);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ShadowSize, ShadowSize, 0,
						 GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			if (glGetError() != GL_NO_ERROR)
			{
				glDeleteTextures(1, &texture);
				throw std::runtime_error("OpenGL failed to upload the decor shadow texture.");
			}
			return texture;
		}

		static void appendCap(std::vector<TexturedVertex> &vertices, float radius,
							  float y, int segments, bool top)
		{
			const TexturedVertex center = makeVertex(
				glm::vec3(0.0f, y, 0.0f), glm::vec2(0.5f));
			for (int i = 0; i < segments; ++i)
			{
				const float angle0 = 2.0f * Pi * static_cast<float>(i) / segments;
				const float angle1 = 2.0f * Pi * static_cast<float>(i + 1) / segments;
				const glm::vec3 point0(radius * std::cos(angle0), y,
									   radius * std::sin(angle0));
				const glm::vec3 point1(radius * std::cos(angle1), y,
									   radius * std::sin(angle1));
				const TexturedVertex ring0 = makeVertex(point0,
														glm::vec2(0.5f + 0.5f * std::cos(angle0),
																  0.5f + 0.5f * std::sin(angle0)));
				const TexturedVertex ring1 = makeVertex(point1,
														glm::vec2(0.5f + 0.5f * std::cos(angle1),
																  0.5f + 0.5f * std::sin(angle1)));
				if (top)
					appendTriangle(vertices, center, ring0, ring1);
				else
					appendTriangle(vertices, center, ring1, ring0);
			}
		}

		static void appendFrustum(std::vector<TexturedVertex> &vertices,
								  float bottomRadius, float topRadius, float bottomY, float topY,
								  int segments, float horizontalRepeats, bool caps = true)
		{
			const float height = topY - bottomY;
			const float verticalRepeats = height / 0.7f;
			for (int i = 0; i < segments; ++i)
			{
				const float angle0 = 2.0f * Pi * static_cast<float>(i) / segments;
				const float angle1 = 2.0f * Pi * static_cast<float>(i + 1) / segments;
				const float u0 = horizontalRepeats * static_cast<float>(i) / segments;
				const float u1 = horizontalRepeats * static_cast<float>(i + 1) / segments;

				const TexturedVertex bottom0 = makeVertex(
					glm::vec3(bottomRadius * std::cos(angle0), bottomY,
							  bottomRadius * std::sin(angle0)),
					glm::vec2(u0, 0.0f));
				const TexturedVertex bottom1 = makeVertex(
					glm::vec3(bottomRadius * std::cos(angle1), bottomY,
							  bottomRadius * std::sin(angle1)),
					glm::vec2(u1, 0.0f));
				const TexturedVertex top0 = makeVertex(
					glm::vec3(topRadius * std::cos(angle0), topY,
							  topRadius * std::sin(angle0)),
					glm::vec2(u0, verticalRepeats));
				const TexturedVertex top1 = makeVertex(
					glm::vec3(topRadius * std::cos(angle1), topY,
							  topRadius * std::sin(angle1)),
					glm::vec2(u1, verticalRepeats));
				appendTriangle(vertices, bottom0, top0, top1);
				appendTriangle(vertices, bottom0, top1, bottom1);
			}
			if (caps)
			{
				appendCap(vertices, bottomRadius, bottomY, segments, false);
				appendCap(vertices, topRadius, topY, segments, true);
			}
		}

	};
}
