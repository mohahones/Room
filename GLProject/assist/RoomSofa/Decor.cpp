#pragma once

#include "../Shared/RenderUtils.cpp"

#include <array>
#include <cmath>
#include <vector>

namespace scene
{
	class Decor
	{
	public:
		void initialize()
		{
			const std::vector<glm::vec3> squareVertices = {
				glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.5f, -0.5f, 0.0f),
				glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(-0.5f, 0.5f, 0.0f)
			};
			const std::vector<glm::vec2> squareTexcoords = {
				glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f),
				glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f)
			};

			texturedQuad = new PolygonWithTexture(squareVertices, squareTexcoords);
			for (size_t i = 0; i < bookQuads.size(); ++i)
				bookQuads[i] = new Polygon(squareVertices, BookColors[i]);

			std::vector<glm::vec3> topVertices = {
				glm::vec3(0.0f, LampBaseTopY, 0.0f)
			};
			for (size_t i = 0; i <= LampBaseSegments; ++i)
			{
				const float angle =
					2.0f * Pi * static_cast<float>(i) /
					static_cast<float>(LampBaseSegments);
				topVertices.emplace_back(
					LampBaseRadius * std::cos(angle),
					LampBaseTopY,
					LampBaseRadius * std::sin(angle));
			}
			lampBaseTop = new Polygon(topVertices, LampBaseColor);

			for (size_t i = 0; i < lampBaseSides.size(); ++i)
			{
				const float angle0 =
					2.0f * Pi * static_cast<float>(i) /
					static_cast<float>(LampBaseSegments);
				const float angle1 =
					2.0f * Pi * static_cast<float>(i + 1) /
					static_cast<float>(LampBaseSegments);
				const std::vector<glm::vec3> sideVertices = {
					glm::vec3(LampBaseRadius * std::cos(angle0),
						LampBaseBottomY, LampBaseRadius * std::sin(angle0)),
					glm::vec3(LampBaseRadius * std::cos(angle1),
						LampBaseBottomY, LampBaseRadius * std::sin(angle1)),
					glm::vec3(LampBaseRadius * std::cos(angle1),
						LampBaseTopY, LampBaseRadius * std::sin(angle1)),
					glm::vec3(LampBaseRadius * std::cos(angle0),
						LampBaseTopY, LampBaseRadius * std::sin(angle0))
				};
				lampBaseSides[i] = new Polygon(sideVertices, LampBaseColor);
			}

			for (size_t i = 0; i < shadeSegments.size(); ++i)
			{
				const float angle0 = 2.0f * Pi * static_cast<float>(i) /
					static_cast<float>(shadeSegments.size());
				const float angle1 = 2.0f * Pi * static_cast<float>(i + 1) /
					static_cast<float>(shadeSegments.size());
				const float u0 = static_cast<float>(i) /
					static_cast<float>(shadeSegments.size());
				const float u1 = static_cast<float>(i + 1) /
					static_cast<float>(shadeSegments.size());
				const std::vector<glm::vec3> segmentVertices = {
					glm::vec3(ShadeBottomRadius * std::cos(angle0),
						ShadeBottomY, ShadeBottomRadius * std::sin(angle0)),
					glm::vec3(ShadeBottomRadius * std::cos(angle1),
						ShadeBottomY, ShadeBottomRadius * std::sin(angle1)),
					glm::vec3(ShadeTopRadius * std::cos(angle1),
						ShadeTopY, ShadeTopRadius * std::sin(angle1)),
					glm::vec3(ShadeTopRadius * std::cos(angle0),
						ShadeTopY, ShadeTopRadius * std::sin(angle0))
				};
				const std::vector<glm::vec2> segmentTexcoords = {
					glm::vec2(u0, 0.0f), glm::vec2(u1, 0.0f),
					glm::vec2(u1, 1.0f), glm::vec2(u0, 1.0f)
				};
				shadeSegments[i] = new PolygonWithTexture(segmentVertices,
					segmentTexcoords);
			}

			metalTexture = loadTexture("./textures/blue_metal_plate_diff_1k.jpg");
			shadeTexture = loadTexture("./textures/denim_fabric_04_diff_1k.jpg");
		}

		void render(Shader& shader)
		{
			if (!texturedQuad)
				throw std::logic_error("Decor polygons were not initialized.");

			drawBookshelf(shader);
			drawFloorLamp(shader);
		}

		void cleanup()
		{
			if (texturedQuad)
			{
				texturedQuad->deleteBuffers();
				delete texturedQuad;
				texturedQuad = nullptr;
			}
			for (PolygonWithTexture* shadeSegment : shadeSegments)
			{
				if (shadeSegment)
				{
					shadeSegment->deleteBuffers();
					delete shadeSegment;
					shadeSegment = nullptr;
				}
			}
			for (Polygon* bookQuad : bookQuads)
			{
				if (bookQuad)
				{
					bookQuad->deleteBuffers();
					delete bookQuad;
					bookQuad = nullptr;
				}
			}
			if (lampBaseTop)
			{
				lampBaseTop->deleteBuffers();
				delete lampBaseTop;
				lampBaseTop = nullptr;
			}
			for (Polygon* side : lampBaseSides)
			{
				if (side)
				{
					side->deleteBuffers();
					delete side;
					side = nullptr;
				}
			}
			if (metalTexture)
				glDeleteTextures(1, &metalTexture);
			if (shadeTexture)
				glDeleteTextures(1, &shadeTexture);
			metalTexture = 0;
			shadeTexture = 0;
		}

	private:
		void drawBookshelf(Shader& shader)
		{
			const glm::mat4 shelfBase =
				glm::translate(glm::mat4(1.0f),
					glm::vec3(ShelfX, ShelfFloorY, ShelfZ));

			// Upright back and side panels.
			drawTexturedPanel(shader, shelfBase, metalTexture,
				glm::vec3(0.0f, (ShelfHeight + ShelfBoardHeight) * 0.5f,
					-ShelfDepth * 0.5f),
				glm::vec3(ShelfWidth, ShelfHeight + ShelfBoardHeight, 1.0f),
				glm::vec3(0.0f));
			drawTexturedPanel(shader, shelfBase, metalTexture,
				glm::vec3(-ShelfWidth * 0.5f,
					(ShelfHeight + ShelfBoardHeight) * 0.5f, 0.0f),
				glm::vec3(ShelfDepth, ShelfHeight + ShelfBoardHeight, 1.0f),
				glm::vec3(0.0f, -Pi * 0.5f, 0.0f));
			drawTexturedPanel(shader, shelfBase, metalTexture,
				glm::vec3(ShelfWidth * 0.5f,
					(ShelfHeight + ShelfBoardHeight) * 0.5f, 0.0f),
				glm::vec3(ShelfDepth, ShelfHeight + ShelfBoardHeight, 1.0f),
				glm::vec3(0.0f, Pi * 0.5f, 0.0f));

			// Bottom and divider shelves.
			const std::array<float, 5> shelfLevels = {
				0.06f, 1.25f, 2.44f, 3.63f, 4.82f
			};
			for (float y : shelfLevels)
				drawHorizontalTexturedPanel(shader, shelfBase, metalTexture,
					glm::vec3(0.0f, y, 0.0f),
					glm::vec2(ShelfWidth, ShelfDepth));

			drawTopCap(shader, shelfBase, metalTexture);

			// Narrow front uprights make the frame read cleanly.
			for (float x : { -ShelfWidth * 0.5f + 0.11f,
				ShelfWidth * 0.5f - 0.11f })
				drawTexturedPanel(shader, shelfBase, metalTexture,
					glm::vec3(x, (ShelfHeight + ShelfBoardHeight) * 0.5f,
						ShelfDepth * 0.5f),
					glm::vec3(0.22f, ShelfHeight + ShelfBoardHeight, 1.0f),
					glm::vec3(0.0f));

			drawBooks(shader, shelfBase, shelfLevels);
		}

		void drawBooks(Shader& shader, const glm::mat4& shelfBase,
			const std::array<float, 5>& shelfLevels)
		{
			const float innerLeft = -ShelfWidth * 0.5f + 0.42f;
			const float innerRight = ShelfWidth * 0.5f - 0.42f;
			constexpr size_t BooksPerRow = 10;
			constexpr float BookGap = 0.085f;
			const float rowWidth = innerRight - innerLeft;
			const float bookWidth =
				(rowWidth - BookGap * static_cast<float>(BooksPerRow - 1)) /
				static_cast<float>(BooksPerRow);

			for (size_t row = 0; row < 5; ++row)
			{
				const float shelfY = shelfLevels[row] + ShelfBoardHeight * 0.5f;
				for (size_t i = 0; i < BooksPerRow; ++i)
				{
					const size_t variation = (i + row * 3) % 5;
					const float height = 0.76f + 0.075f *
						static_cast<float>((i + row) % 4);
					const float width = bookWidth *
						(0.88f + 0.025f * static_cast<float>(variation));
					const float x = innerLeft + bookWidth * (i + 0.5f) +
						BookGap * static_cast<float>(i);
					const float depth = 0.34f +
						0.04f * static_cast<float>((i + row) % 3);
					Polygon* bookQuad = bookQuads[(i + row) % bookQuads.size()];

					shader.setBool("useTexture", false);
					drawBookBox(shader, bookQuad, shelfBase,
						glm::vec3(x, shelfY + height * 0.5f,
							ShelfDepth * 0.42f),
						glm::vec3(width, height, depth));
				}
			}
		}

		void drawBookBox(Shader& shader, Polygon* bookQuad,
			const glm::mat4& shelfBase, const glm::vec3& center,
			const glm::vec3& size)
		{
			const glm::mat4 frontBackBase =
				shelfBase *
				glm::translate(glm::mat4(1.0f), center);

			bookQuad->transformation(
				frontBackBase *
				glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, size.z * 0.5f)) *
				glm::scale(glm::mat4(1.0f),
					glm::vec3(size.x, size.y, 1.0f)));
			bookQuad->draw(shader);

			bookQuad->transformation(
				frontBackBase *
				glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -size.z * 0.5f)) *
				glm::rotate(glm::mat4(1.0f), Pi, glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f),
					glm::vec3(size.x, size.y, 1.0f)));
			bookQuad->draw(shader);

			bookQuad->transformation(
				frontBackBase *
				glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, size.y * 0.5f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), -Pi * 0.5f,
					glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f),
					glm::vec3(size.x, size.z, 1.0f)));
			bookQuad->draw(shader);

			bookQuad->transformation(
				frontBackBase *
				glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -size.y * 0.5f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), Pi * 0.5f,
					glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f),
					glm::vec3(size.x, size.z, 1.0f)));
			bookQuad->draw(shader);

			bookQuad->transformation(
				frontBackBase *
				glm::translate(glm::mat4(1.0f), glm::vec3(-size.x * 0.5f, 0.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), -Pi * 0.5f,
					glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f),
					glm::vec3(size.z, size.y, 1.0f)));
			bookQuad->draw(shader);

			bookQuad->transformation(
				frontBackBase *
				glm::translate(glm::mat4(1.0f), glm::vec3(size.x * 0.5f, 0.0f, 0.0f)) *
				glm::rotate(glm::mat4(1.0f), Pi * 0.5f,
					glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f),
					glm::vec3(size.z, size.y, 1.0f)));
			bookQuad->draw(shader);
		}

		void drawTexturedPanel(Shader& shader, const glm::mat4& parent,
			GLuint texture, const glm::vec3& localPosition,
			const glm::vec3& scale, const glm::vec3& rotation)
		{
			glm::mat4 model = parent *
				glm::translate(glm::mat4(1.0f), localPosition);
			if (rotation.x != 0.0f)
				model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
			if (rotation.y != 0.0f)
				model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, scale);

			shader.setBool("useTexture", true);
			shader.setVec2("textureScale", glm::vec2(1.0f));
			texturedQuad->setTexture(texture);
			texturedQuad->transformation(model);
			glActiveTexture(GL_TEXTURE0);
			texturedQuad->draw(shader);
		}

		void drawHorizontalTexturedPanel(Shader& shader, const glm::mat4& parent,
			GLuint texture, const glm::vec3& localPosition, const glm::vec2& size)
		{
			shader.setBool("useTexture", true);
			shader.setVec2("textureScale", glm::vec2(1.0f));
			texturedQuad->setTexture(texture);
			texturedQuad->transformation(
				parent *
				glm::translate(glm::mat4(1.0f), localPosition) *
				glm::rotate(glm::mat4(1.0f), -Pi * 0.5f,
					glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f),
					glm::vec3(size.x, size.y, 1.0f)));
			glActiveTexture(GL_TEXTURE0);
			texturedQuad->draw(shader);
		}

		void drawTopCap(Shader& shader, const glm::mat4& shelfBase, GLuint texture)
		{
			constexpr float TopCapThickness = 0.28f;
			const float capCenterY = ShelfHeight;

			// Broad top and underside faces.
			drawHorizontalTexturedPanel(shader, shelfBase, texture,
				glm::vec3(0.0f, capCenterY + TopCapThickness * 0.5f,
					0.0f),
				glm::vec2(ShelfWidth, ShelfDepth));
			drawHorizontalTexturedPanel(shader, shelfBase, texture,
				glm::vec3(0.0f, capCenterY - TopCapThickness * 0.5f,
					0.0f),
				glm::vec2(ShelfWidth, ShelfDepth));

			// Clear front, back, left, and right faces make the cap visibly solid.
			drawTexturedPanel(shader, shelfBase, texture,
				glm::vec3(0.0f, capCenterY, ShelfDepth * 0.5f + 0.02f),
				glm::vec3(ShelfWidth, TopCapThickness, 1.0f),
				glm::vec3(0.0f));
			drawTexturedPanel(shader, shelfBase, texture,
				glm::vec3(0.0f, capCenterY, -ShelfDepth * 0.5f - 0.02f),
				glm::vec3(ShelfWidth, TopCapThickness, 1.0f),
				glm::vec3(0.0f));
			drawTexturedPanel(shader, shelfBase, texture,
				glm::vec3(-ShelfWidth * 0.5f - 0.02f, capCenterY, 0.0f),
				glm::vec3(ShelfDepth, TopCapThickness, 1.0f),
				glm::vec3(0.0f, -Pi * 0.5f, 0.0f));
			drawTexturedPanel(shader, shelfBase, texture,
				glm::vec3(ShelfWidth * 0.5f + 0.02f, capCenterY, 0.0f),
				glm::vec3(ShelfDepth, TopCapThickness, 1.0f),
				glm::vec3(0.0f, Pi * 0.5f, 0.0f));
		}

		void drawFloorLamp(Shader& shader)
		{
			const glm::mat4 lampBase =
				glm::translate(glm::mat4(1.0f),
					glm::vec3(FloorLampX, FloorLampFloorY, FloorLampZ));

			drawCircularLampBase(shader, lampBase);

			// Tall, thin stand and a small collar at the shade joint.
			drawTexturedPanel(shader, lampBase, metalTexture,
				glm::vec3(0.0f, 1.55f, 0.0f),
				glm::vec3(0.10f, 2.78f, 1.0f),
				glm::vec3(0.0f));
			drawHorizontalLampPart(shader, lampBase, metalTexture,
				glm::vec3(0.0f, 2.34f, 0.0f), glm::vec2(0.24f, 0.24f));

			drawShadeFrustum(shader, lampBase);
		}

		void drawCircularLampBase(Shader& shader, const glm::mat4& lampBase)
		{
			if (!lampBaseTop)
				throw std::logic_error("Lamp base top was not initialized.");

			shader.setBool("useTexture", false);
			lampBaseTop->transformation(lampBase);
			lampBaseTop->draw(shader);
			for (Polygon* side : lampBaseSides)
			{
				if (!side)
					throw std::logic_error("Lamp base side was not initialized.");
				side->transformation(lampBase);
				side->draw(shader);
			}
		}

		void drawShadeFrustum(Shader& shader, const glm::mat4& lampBase)
		{
			shader.setBool("useTexture", true);
			shader.setVec2("textureScale", glm::vec2(1.0f));
			for (PolygonWithTexture* shadeSegment : shadeSegments)
			{
				if (!shadeSegment)
					throw std::logic_error("Lamp shade segment was not initialized.");
				shadeSegment->setTexture(shadeTexture);
				shadeSegment->transformation(lampBase);
				glActiveTexture(GL_TEXTURE0);
				shadeSegment->draw(shader);
			}
		}

		void drawHorizontalLampPart(Shader& shader, const glm::mat4& parent,
			GLuint texture, const glm::vec3& localPosition, const glm::vec2& size)
		{
			shader.setBool("useTexture", true);
			shader.setVec2("textureScale", glm::vec2(1.0f));
			texturedQuad->setTexture(texture);
			texturedQuad->transformation(
				parent *
				glm::translate(glm::mat4(1.0f), localPosition) *
				glm::rotate(glm::mat4(1.0f), -Pi * 0.5f,
					glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f),
					glm::vec3(size.x, size.y, 1.0f)));
			glActiveTexture(GL_TEXTURE0);
			texturedQuad->draw(shader);
		}

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
		static constexpr float LampBaseRadius = 0.62f;
		static constexpr float LampBaseBottomY = 0.06f;
		static constexpr float LampBaseTopY = 0.16f;
		static constexpr size_t LampBaseSegments = 24;
		static constexpr float ShadeBottomY = 2.38f;
		static constexpr float ShadeTopY = 3.05f;
		static constexpr float ShadeBottomRadius = 0.82f;
		static constexpr float ShadeTopRadius = 0.48f;

		PolygonWithTexture* texturedQuad = nullptr;
		std::array<PolygonWithTexture*, 32> shadeSegments{};
		std::array<Polygon*, 6> bookQuads{};
		Polygon* lampBaseTop = nullptr;
		std::array<Polygon*, LampBaseSegments> lampBaseSides{};
		GLuint metalTexture = 0;
		GLuint shadeTexture = 0;
		const glm::vec3 LampBaseColor = glm::vec3(0.32f, 0.34f, 0.36f);
		const std::array<glm::vec3, 6> BookColors = {
			glm::vec3(0.72f, 0.16f, 0.20f),
			glm::vec3(0.86f, 0.52f, 0.24f),
			glm::vec3(0.18f, 0.44f, 0.32f),
			glm::vec3(0.48f, 0.42f, 0.68f),
			glm::vec3(0.72f, 0.68f, 0.50f),
			glm::vec3(0.82f, 0.72f, 0.66f)
		};
	};
}
