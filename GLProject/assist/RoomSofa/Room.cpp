#pragma once

#include "../Shared/RenderUtils.cpp"

#include <memory>
#include <stdexcept>

namespace scene
{
	class Room
	{
	public:
		void initialize()
		{
			quad = createTexturedQuad();
			floorTexture = loadTexture("./textures/wood_floor_diff_1k.jpg");
			wallTexture = loadTexture("./textures/plastered_wall_diff_1k.jpg");
		}

		void render(Shader& shader)
		{
			if (!quad)
				throw std::logic_error("Room polygon was not initialized.");

			drawTexturedPlane(shader, *quad, floorTexture,
				glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.5f, 1.0f)) *
				glm::rotate(glm::mat4(1.0f), -Pi * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(32.0f, 30.0f, 1.0f)),
				glm::vec2(16.0f, 15.0f));

			drawTexturedPlane(shader, *quad, wallTexture,
				glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.5f, -14.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(32.0f, 8.0f, 1.0f)),
				glm::vec2(16.0f, 4.0f));
			drawTexturedPlane(shader, *quad, wallTexture,
				glm::translate(glm::mat4(1.0f), glm::vec3(-16.0f, 2.5f, 1.0f)) *
				glm::rotate(glm::mat4(1.0f), -Pi * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(30.0f, 8.0f, 1.0f)),
				glm::vec2(15.0f, 4.0f));
			drawTexturedPlane(shader, *quad, wallTexture,
				glm::translate(glm::mat4(1.0f), glm::vec3(16.0f, 2.5f, 1.0f)) *
				glm::rotate(glm::mat4(1.0f), Pi * 0.5f, glm::vec3(0.0f, 1.0f, 0.0f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(30.0f, 8.0f, 1.0f)),
				glm::vec2(15.0f, 4.0f));
		}

		void cleanup()
		{
			if (quad)
				quad->deleteBuffers();
			quad.reset();
			if (floorTexture)
				glDeleteTextures(1, &floorTexture);
			if (wallTexture)
				glDeleteTextures(1, &wallTexture);
			floorTexture = 0;
			wallTexture = 0;
		}

	private:
		std::unique_ptr<PolygonWithTexture> quad;
		GLuint floorTexture = 0;
		GLuint wallTexture = 0;
	};
}
