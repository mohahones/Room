#pragma once

#include "../Shared/RenderUtils.cpp"

#include <array>
#include <memory>
#include <stdexcept>
#include <string>

namespace scene
{
	class Cards
	{
	private:
		std::array<std::unique_ptr<PolygonWithTexture>, 5> cardQuads;
		std::array<GLuint, 5> cardTextures{};
		GLuint cardBackTexture = 0;

	public:
		void initialize()
		{
			static const std::array<std::string, 5> texturePaths = {
				"./textures/assest/p11.jpg",
				"./textures/assest/p12.jpg",
				"./textures/assest/p13.jpg",
				"./textures/assest/p14.jpg",
				"./textures/assest/p15.jpg"
			};
			for (size_t i = 0; i < cardQuads.size(); ++i)
			{
				cardQuads[i] = createTexturedQuad();
				cardTextures[i] = loadTexture(texturePaths[i]);
			}
			cardBackTexture = loadTexture("./textures/backgoround/p16.jpg");
		}

		void renderCard(Shader& shader, size_t index, const glm::mat4& model)
		{
			if (index >= cardQuads.size() || !cardQuads[index] ||
				!cardTextures[index] || !cardBackTexture)
				throw std::logic_error("A card polygon, face texture, or back texture is missing.");

			shader.setBool("useTexture", true);
			GLuint texture = cardBackTexture;
			cardQuads[index]->setTexture(texture);
			cardQuads[index]->transformation(
				model *
				glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.003f)) *
				glm::rotate(glm::mat4(1.0f), Pi, glm::vec3(0.0f, 1.0f, 0.0f)));
			glActiveTexture(GL_TEXTURE0);
			cardQuads[index]->draw(shader);

			texture = cardTextures[index];
			cardQuads[index]->setTexture(texture);
			cardQuads[index]->transformation(model);
			glActiveTexture(GL_TEXTURE0);
			cardQuads[index]->draw(shader);
		}

		void cleanup()
		{
			for (auto& quad : cardQuads)
			{
				if (quad)
					quad->deleteBuffers();
				quad.reset();
			}
			for (GLuint texture : cardTextures)
				if (texture)
					glDeleteTextures(1, &texture);
			if (cardBackTexture)
				glDeleteTextures(1, &cardBackTexture);
			cardTextures.fill(0);
			cardBackTexture = 0;
		}
	};
}
