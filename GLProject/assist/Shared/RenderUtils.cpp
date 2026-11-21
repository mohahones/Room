#pragma once

#include "../../Polygon.cpp"
#include "../../PolygonWithTexture.cpp"
#include <learnopengl/shader_m.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace scene
{
	inline constexpr float Pi = 3.14159265f;
	inline constexpr float TableDepthOffset = -1.2f;

	inline std::unique_ptr<PolygonWithTexture> createTexturedQuad()
	{
		const std::vector<glm::vec3> positions = {
			glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.5f, -0.5f, 0.0f),
			glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(-0.5f, 0.5f, 0.0f)
		};
		const std::vector<glm::vec2> texcoords = {
			glm::vec2(0.0f, 0.0f), glm::vec2(1.0f, 0.0f),
			glm::vec2(1.0f, 1.0f), glm::vec2(0.0f, 1.0f)
		};
		return std::make_unique<PolygonWithTexture>(positions, texcoords);
	}

	inline std::unique_ptr<Polygon> createSolidQuad(const glm::vec3& color)
	{
		const std::vector<glm::vec3> positions = {
			glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0.5f, -0.5f, 0.0f),
			glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(-0.5f, 0.5f, 0.0f)
		};
		return std::make_unique<Polygon>(positions, color);
	}

	inline GLuint loadTexture(const std::string& path)
	{
		int width = 0;
		int height = 0;
		int channels = 0;
		unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, 0);
		if (!pixels)
		{
			const char* reason = stbi_failure_reason();
			throw std::runtime_error("Unable to load texture '" + path + "': " +
				(reason ? reason : "unknown image error"));
		}
		if (width <= 0 || height <= 0)
		{
			stbi_image_free(pixels);
			throw std::runtime_error("Texture has invalid dimensions: '" + path + "'.");
		}

		GLenum format = 0;
		GLenum internalFormat = 0;
		switch (channels)
		{
		case 1: format = GL_RED; internalFormat = GL_R8; break;
		case 3: format = GL_RGB; internalFormat = GL_RGB8; break;
		case 4: format = GL_RGBA; internalFormat = GL_RGBA8; break;
		default:
			stbi_image_free(pixels);
			throw std::runtime_error("Unsupported channel count in texture '" + path + "'.");
		}

		GLuint texture = 0;
		glGenTextures(1, &texture);
		if (!texture)
		{
			stbi_image_free(pixels);
			throw std::runtime_error("Failed to allocate an OpenGL texture for '" + path + "'.");
		}

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format,
			GL_UNSIGNED_BYTE, pixels);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(pixels);

		if (glGetError() != GL_NO_ERROR)
		{
			glDeleteTextures(1, &texture);
			throw std::runtime_error("OpenGL failed to upload texture '" + path + "'.");
		}
		return texture;
	}

	inline void drawTexturedPlane(Shader& shader, PolygonWithTexture& polygon, GLuint texture,
		const glm::mat4& model, const glm::vec2& uvScale = glm::vec2(1.0f))
	{
		if (!texture)
			throw std::logic_error("Cannot draw a textured polygon without a valid texture.");
		shader.setBool("useTexture", true);
		shader.setVec2("textureScale", uvScale);
		polygon.setTexture(texture);
		polygon.transformation(model);
		glActiveTexture(GL_TEXTURE0);
		polygon.draw(shader);
	}

	inline void drawHorizontalTexture(Shader& shader, PolygonWithTexture& polygon, GLuint texture,
		const glm::vec3& center, const glm::vec2& size,
		const glm::vec2& uvScale = glm::vec2(1.0f))
	{
		const glm::mat4 model = glm::translate(glm::mat4(1.0f), center) *
			glm::rotate(glm::mat4(1.0f), -Pi * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
		drawTexturedPlane(shader, polygon, texture, model, uvScale);
	}

	inline void drawHorizontalSolid(Shader& shader, Polygon& polygon,
		const glm::vec3& center, const glm::vec2& size)
	{
		const glm::mat4 model = glm::translate(glm::mat4(1.0f), center) *
			glm::rotate(glm::mat4(1.0f), -Pi * 0.5f, glm::vec3(1.0f, 0.0f, 0.0f)) *
			glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));
		shader.setBool("useTexture", false);
		polygon.transformation(model);
		polygon.draw(shader);
	}
}
