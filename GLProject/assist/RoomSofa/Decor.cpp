#pragma once

#include "../Shared/RenderUtils.cpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <vector>

namespace scene
{
	struct LitTexturedVertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoord;
	};

	class LitTexturedMesh
	{
	public:
		void initialize(const std::vector<LitTexturedVertex>& vertices)
		{
			if (vertices.empty())
				throw std::logic_error("Cannot initialize an empty lit mesh.");

			glGenVertexArrays(1, &vao);
			glGenBuffers(1, &vbo);
			if (!vao || !vbo)
			{
				cleanup();
				throw std::runtime_error("Failed to allocate lamp mesh buffers.");
			}

			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER,
				static_cast<GLsizeiptr>(vertices.size() * sizeof(LitTexturedVertex)),
				vertices.data(), GL_STATIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LitTexturedVertex),
				reinterpret_cast<void*>(offsetof(LitTexturedVertex, position)));
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LitTexturedVertex),
				reinterpret_cast<void*>(offsetof(LitTexturedVertex, normal)));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(LitTexturedVertex),
				reinterpret_cast<void*>(offsetof(LitTexturedVertex, texCoord)));
			glEnableVertexAttribArray(2);
			glBindVertexArray(0);
			vertexCount = static_cast<GLsizei>(vertices.size());
		}

		void draw(Shader& shader, GLuint texture, const glm::mat4& model,
			const glm::vec3& color, float shininess, float specularStrength,
			float metallic) const
		{
			if (!vao || vertexCount <= 0)
				throw std::logic_error("Lit mesh was not initialized.");

			shader.setBool("useTexture", texture != 0);
			shader.setBool("useLighting", true);
			shader.setVec2("textureScale", glm::vec2(1.0f));
			shader.setVec3("objectColor", color);
			shader.setFloat("materialShininess", shininess);
			shader.setFloat("materialSpecularStrength", specularStrength);
			shader.setFloat("materialMetallic", metallic);
			shader.setMat4("model", model);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);
			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, vertexCount);
			glBindVertexArray(0);
		}

		void cleanup()
		{
			if (vbo)
				glDeleteBuffers(1, &vbo);
			if (vao)
				glDeleteVertexArrays(1, &vao);
			vbo = 0;
			vao = 0;
			vertexCount = 0;
		}

	private:
		GLuint vao = 0;
		GLuint vbo = 0;
		GLsizei vertexCount = 0;
	};

	class Decor
	{
	public:
		void initialize()
		{
			shadowQuad = createTexturedQuad();
			shadowTexture = createContactShadowTexture();
			metalTexture = loadTexture("./textures/blue_metal_plate_diff_1k.jpg");
			shadeTexture = loadTexture("./textures/denim_fabric_04_diff_1k.jpg");

			std::vector<LitTexturedVertex> metalVertices;
			appendFrustum(metalVertices, 0.34f, 0.30f, 0.0f, 0.12f, 64, 2.0f);
			appendFrustum(metalVertices, 0.055f, 0.045f, 0.11f, 1.98f, 32, 1.0f, false);
			appendFrustum(metalVertices, 0.09f, 0.09f, 1.89f, 2.03f, 48, 1.0f);
			appendFrustum(metalVertices, 0.485f, 0.485f, 1.95f, 2.02f, 64, 2.0f);
			appendFrustum(metalVertices, 0.285f, 0.285f, 2.64f, 2.71f, 64, 2.0f);
			metalMesh.initialize(metalVertices);

			std::vector<LitTexturedVertex> shadeVertices;
			appendFrustum(shadeVertices, 0.45f, 0.26f, 1.98f, 2.68f, 64, 3.0f);
			shadeMesh.initialize(shadeVertices);

			std::vector<LitTexturedVertex> shelfVertices;
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
				ShelfHeight - ShelfBoardHeight * 0.5f
			};
			for (float y : shelfLevels)
				appendBox(shelfVertices, glm::vec3(0.0f, y, 0.0f),
					glm::vec3(ShelfWidth, ShelfBoardHeight, ShelfDepth));
			shelfMesh.initialize(shelfVertices);

			std::array<std::vector<LitTexturedVertex>, 4> bookVertices;
			std::vector<LitTexturedVertex> bookTrimVertices;
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
				bookMeshes[i].initialize(bookVertices[i]);
			bookTrimMesh.initialize(bookTrimVertices);
		}

		void render(Shader& shader)
		{
			if (!shadowQuad)
				throw std::logic_error("Decor polygons were not initialized.");

			drawContactShadows(shader);
			drawFloorShelf(shader);
			drawFloorLamp(shader);
			shader.setBool("useLighting", false);
		}

		void cleanup()
		{
			if (shadowQuad)
				shadowQuad->deleteBuffers();
			shadowQuad.reset();
			metalMesh.cleanup();
			shadeMesh.cleanup();
			shelfMesh.cleanup();
			bookTrimMesh.cleanup();
			for (auto& bookMesh : bookMeshes)
				bookMesh.cleanup();
			if (shadowTexture)
				glDeleteTextures(1, &shadowTexture);
			if (metalTexture)
				glDeleteTextures(1, &metalTexture);
			if (shadeTexture)
				glDeleteTextures(1, &shadeTexture);
			metalTexture = 0;
			shadeTexture = 0;
			shadowTexture = 0;
		}

	private:
		void drawContactShadows(Shader& shader)
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

		void drawFloorShelf(Shader& shader)
		{
			const glm::mat4 model = glm::translate(glm::mat4(1.0f),
				glm::vec3(ShelfX, ShelfFloorY, ShelfZ));
			shelfMesh.draw(shader, metalTexture, model, glm::vec3(1.0f),
				48.0f, 0.5f, 0.82f);
			for (size_t i = 0; i < bookMeshes.size(); ++i)
				bookMeshes[i].draw(shader, 0, model, bookColors[i], 24.0f, 0.22f, 0.04f);
			bookTrimMesh.draw(shader, 0, model, glm::vec3(0.9f, 0.62f, 0.18f),
				42.0f, 0.38f, 0.72f);
		}

		void drawFloorLamp(Shader& shader)
		{
			const glm::mat4 model = glm::translate(glm::mat4(1.0f),
				glm::vec3(FloorLampX, FloorLampFloorY, FloorLampZ));
			metalMesh.draw(shader, metalTexture, model, glm::vec3(1.0f),
				56.0f, 0.56f, 0.9f);
			shadeMesh.draw(shader, shadeTexture, model, glm::vec3(1.0f),
				16.0f, 0.18f, 0.0f);
		}

		static LitTexturedVertex makeVertex(const glm::vec3& position,
			const glm::vec3& normal, const glm::vec2& texCoord)
		{
			return { position, normal, texCoord };
		}

		static void appendTriangle(std::vector<LitTexturedVertex>& vertices,
			const LitTexturedVertex& a, const LitTexturedVertex& b,
			const LitTexturedVertex& c)
		{
			vertices.push_back(a);
			vertices.push_back(b);
			vertices.push_back(c);
		}

		static void appendFace(std::vector<LitTexturedVertex>& vertices,
			const glm::vec3& bottomLeft, const glm::vec3& bottomRight,
			const glm::vec3& topRight, const glm::vec3& topLeft,
			const glm::vec3& normal, const glm::vec2& uvSize)
		{
			const LitTexturedVertex a = makeVertex(bottomLeft, normal, glm::vec2(0.0f));
			const LitTexturedVertex b = makeVertex(bottomRight, normal,
				glm::vec2(uvSize.x, 0.0f));
			const LitTexturedVertex c = makeVertex(topRight, normal, uvSize);
			const LitTexturedVertex d = makeVertex(topLeft, normal,
				glm::vec2(0.0f, uvSize.y));
			appendTriangle(vertices, a, b, c);
			appendTriangle(vertices, a, c, d);
		}

		static void appendBox(std::vector<LitTexturedVertex>& vertices,
			const glm::vec3& center, const glm::vec3& size)
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
				glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(size.x, size.y));
			appendFace(vertices,
				glm::vec3(left, bottom, back), glm::vec3(right, bottom, back),
				glm::vec3(right, top, back), glm::vec3(left, top, back),
				glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(size.x, size.y));
			appendFace(vertices,
				glm::vec3(left, bottom, back), glm::vec3(left, bottom, front),
				glm::vec3(left, top, front), glm::vec3(left, top, back),
				glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec2(size.z, size.y));
			appendFace(vertices,
				glm::vec3(right, bottom, front), glm::vec3(right, bottom, back),
				glm::vec3(right, top, back), glm::vec3(right, top, front),
				glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(size.z, size.y));
			appendFace(vertices,
				glm::vec3(left, top, back), glm::vec3(right, top, back),
				glm::vec3(right, top, front), glm::vec3(left, top, front),
				glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(size.x, size.z));
			appendFace(vertices,
				glm::vec3(left, bottom, front), glm::vec3(right, bottom, front),
				glm::vec3(right, bottom, back), glm::vec3(left, bottom, back),
				glm::vec3(0.0f, -1.0f, 0.0f), glm::vec2(size.x, size.z));
		}

		static void appendBook(std::vector<LitTexturedVertex>& vertices,
			std::vector<LitTexturedVertex>& trimVertices, const glm::vec3& center,
			const glm::vec3& size)
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

		static void appendCap(std::vector<LitTexturedVertex>& vertices, float radius,
			float y, int segments, bool top)
		{
			const glm::vec3 normal(0.0f, top ? 1.0f : -1.0f, 0.0f);
			const LitTexturedVertex center = makeVertex(glm::vec3(0.0f, y, 0.0f),
				normal, glm::vec2(0.5f));
			for (int i = 0; i < segments; ++i)
			{
				const float angle0 = 2.0f * Pi * static_cast<float>(i) / segments;
				const float angle1 = 2.0f * Pi * static_cast<float>(i + 1) / segments;
				const glm::vec3 point0(radius * std::cos(angle0), y,
					radius * std::sin(angle0));
				const glm::vec3 point1(radius * std::cos(angle1), y,
					radius * std::sin(angle1));
				const LitTexturedVertex ring0 = makeVertex(point0, normal,
					glm::vec2(0.5f + 0.5f * std::cos(angle0),
						0.5f + 0.5f * std::sin(angle0)));
				const LitTexturedVertex ring1 = makeVertex(point1, normal,
					glm::vec2(0.5f + 0.5f * std::cos(angle1),
						0.5f + 0.5f * std::sin(angle1)));
				if (top)
					appendTriangle(vertices, center, ring0, ring1);
				else
					appendTriangle(vertices, center, ring1, ring0);
			}
		}

		static void appendFrustum(std::vector<LitTexturedVertex>& vertices,
			float bottomRadius, float topRadius, float bottomY, float topY,
			int segments, float horizontalRepeats, bool caps = true)
		{
			const float height = topY - bottomY;
			const float normalY = (bottomRadius - topRadius) / height;
			const float verticalRepeats = height / 0.7f;
			for (int i = 0; i < segments; ++i)
			{
				const float angle0 = 2.0f * Pi * static_cast<float>(i) / segments;
				const float angle1 = 2.0f * Pi * static_cast<float>(i + 1) / segments;
				const glm::vec3 normal0 = glm::normalize(
					glm::vec3(std::cos(angle0), normalY, std::sin(angle0)));
				const glm::vec3 normal1 = glm::normalize(
					glm::vec3(std::cos(angle1), normalY, std::sin(angle1)));
				const float u0 = horizontalRepeats * static_cast<float>(i) / segments;
				const float u1 = horizontalRepeats * static_cast<float>(i + 1) / segments;

				const LitTexturedVertex bottom0 = makeVertex(
					glm::vec3(bottomRadius * std::cos(angle0), bottomY,
						bottomRadius * std::sin(angle0)), normal0, glm::vec2(u0, 0.0f));
				const LitTexturedVertex bottom1 = makeVertex(
					glm::vec3(bottomRadius * std::cos(angle1), bottomY,
						bottomRadius * std::sin(angle1)), normal1, glm::vec2(u1, 0.0f));
				const LitTexturedVertex top0 = makeVertex(
					glm::vec3(topRadius * std::cos(angle0), topY,
						topRadius * std::sin(angle0)), normal0,
					glm::vec2(u0, verticalRepeats));
				const LitTexturedVertex top1 = makeVertex(
					glm::vec3(topRadius * std::cos(angle1), topY,
						topRadius * std::sin(angle1)), normal1,
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

		std::unique_ptr<PolygonWithTexture> shadowQuad;
		LitTexturedMesh metalMesh;
		LitTexturedMesh shadeMesh;
		LitTexturedMesh shelfMesh;
		std::array<LitTexturedMesh, 4> bookMeshes;
		LitTexturedMesh bookTrimMesh;
		GLuint metalTexture = 0;
		GLuint shadeTexture = 0;
		GLuint shadowTexture = 0;
		const std::array<glm::vec3, 4> bookColors = {
			glm::vec3(0.78f, 0.09f, 0.12f),
			glm::vec3(0.92f, 0.58f, 0.13f),
			glm::vec3(0.10f, 0.46f, 0.23f),
			glm::vec3(0.9f, 0.9f, 0.83f)
		};
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
	};
}
