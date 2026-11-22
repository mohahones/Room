#pragma once

#include "../Shared/RenderUtils.cpp"

#include <array>
#include <memory>
#include <stdexcept>

namespace scene
{
	class Table
	{
	private:
		std::unique_ptr<PolygonWithTexture> sideQuad;
		std::unique_ptr<PolygonWithTexture> boardQuad;
		std::unique_ptr<PolygonWithTexture> middleZone;
		std::array<std::unique_ptr<Polygon>, 2> playerZones;
		GLuint woodTexture = 0;
		GLuint middleTexture = 0;

	public:
		void initialize()
		{
			sideQuad = createTexturedQuad();
			boardQuad = createTexturedQuad();
			middleZone = createTexturedQuad();
			playerZones[0] = createSolidQuad(glm::vec3(0.12f, 0.34f, 0.22f));
			playerZones[1] = createSolidQuad(glm::vec3(0.14f, 0.24f, 0.42f));
			woodTexture = loadTexture("./textures/oak_veneer_03_diff_1k.jpg");
			middleTexture = loadTexture("./textures/leather_red_03_coll1_1k (1).png");
		}

		void render(Shader& shader)
		{
			if (!sideQuad || !boardQuad || !middleZone || !playerZones[0] || !playerZones[1])
				throw std::logic_error("Table polygons were not initialized.");

			drawTableBody(shader);
			drawPlayZones(shader);
		}

		void cleanup()
		{
			if (sideQuad)
				sideQuad->deleteBuffers();
			if (boardQuad)
				boardQuad->deleteBuffers();
			if (middleZone)
				middleZone->deleteBuffers();
			for (auto& zone : playerZones)
			{
				if (zone)
					zone->deleteBuffers();
				zone.reset();
			}
			sideQuad.reset();
			boardQuad.reset();
			middleZone.reset();
			if (woodTexture)
				glDeleteTextures(1, &woodTexture);
			if (middleTexture)
				glDeleteTextures(1, &middleTexture);
			woodTexture = 0;
			middleTexture = 0;
		}

	private:
		void drawTableBody(Shader& shader)
		{
			const float width = 10.4f;
			const float depth = 6.8f;
			const float thickness = 0.75f;
			const float bottom = -thickness;
			const float halfWidth = width * 0.5f;
			const float halfDepth = depth * 0.5f;
			const float sideCenterY = -thickness * 0.5f;

			drawHorizontalTexture(shader, *boardQuad, woodTexture,
				glm::vec3(0.0f, 0.0f, TableDepthOffset), glm::vec2(width, depth));

			drawTexturedPlane(shader, *sideQuad, woodTexture,
				glm::translate(glm::mat4(1.0f),
					glm::vec3(0.0f, sideCenterY, halfDepth + TableDepthOffset)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(width, thickness, 1.0f)));
			drawTexturedPlane(shader, *sideQuad, woodTexture,
				glm::translate(glm::mat4(1.0f),
					glm::vec3(0.0f, sideCenterY, -halfDepth + TableDepthOffset)) *
				glm::rotate(glm::mat4(1.0f), Pi, glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(width, thickness, 1.0f)));
			drawTexturedPlane(shader, *sideQuad, woodTexture,
				glm::translate(glm::mat4(1.0f),
					glm::vec3(-halfWidth, sideCenterY, TableDepthOffset)) *
				glm::rotate(glm::mat4(1.0f), Pi * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(depth, thickness, 1.0f)));
			drawTexturedPlane(shader, *sideQuad, woodTexture,
				glm::translate(glm::mat4(1.0f),
					glm::vec3(halfWidth, sideCenterY, TableDepthOffset)) *
				glm::rotate(glm::mat4(1.0f), -Pi * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(depth, thickness, 1.0f)));

			const float legSize = 0.5f;
			const float legHeight = -1.5f - bottom;
			const float legCenterY = bottom - legHeight * 0.5f;
			for (float xSign : { -1.0f, 1.0f })
				for (float zSign : { -1.0f, 1.0f })
				{
					const float x = xSign * (halfWidth - legSize * 0.5f - 0.25f);
					const float z = zSign * (halfDepth - legSize * 0.5f - 0.25f) +
						TableDepthOffset;
					drawTexturedPlane(shader, *sideQuad, woodTexture,
						glm::translate(glm::mat4(1.0f),
							glm::vec3(x, legCenterY, z + legSize * 0.5f)) *
						glm::scale(glm::mat4(1.0f), glm::vec3(legSize, legHeight, 1.0f)));
					drawTexturedPlane(shader, *sideQuad, woodTexture,
						glm::translate(glm::mat4(1.0f),
							glm::vec3(x + xSign * legSize * 0.5f, legCenterY, z)) *
						glm::rotate(glm::mat4(1.0f), xSign * Pi * 0.5f,
							glm::vec3(0.0f, 1.0f, 0.0f)) *
						glm::scale(glm::mat4(1.0f), glm::vec3(legSize, legHeight, 1.0f)));
				}
		}

		void drawPlayZones(Shader& shader)
		{
			constexpr float playerZoneWidth = 4.5f;
			constexpr float zoneHeight = 1.45f;
			constexpr float middleZoneWidth = 5.8f;
			drawHorizontalSolid(shader, *playerZones[0],
				glm::vec3(0.0f, 0.025f, 1.82f + TableDepthOffset),
				glm::vec2(playerZoneWidth, zoneHeight));
			drawHorizontalTexture(shader, *middleZone, middleTexture,
				glm::vec3(0.0f, 0.03f, TableDepthOffset),
				glm::vec2(middleZoneWidth, zoneHeight));
			drawHorizontalSolid(shader, *playerZones[1],
				glm::vec3(0.0f, 0.025f, -1.82f + TableDepthOffset),
				glm::vec2(playerZoneWidth, zoneHeight));
		}
	};
}
