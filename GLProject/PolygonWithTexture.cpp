#include <glad/glad.h>
#include <vector>
#include <stdexcept>
#include <learnopengl/shader_m.h>
#include <glm/glm.hpp>

class PolygonWithTexture
{
public:
	PolygonWithTexture(std::vector<glm::vec3> v, std::vector<glm::vec2> t,
		glm::vec3 c = glm::vec3(1.0f, 1.0f, 1.0f),
		GLenum drawMode = GL_TRIANGLE_FAN)
	{
		if (v.size() < 3 || v.size() != t.size())
			throw std::invalid_argument(
				"A textured polygon requires at least three vertices and matching texture coordinates.");

		vertices.reserve(v.size() * 5);
		for (size_t i = 0; i < v.size(); ++i)
		{
			vertices.insert(vertices.end(),
				{ v[i].x, v[i].y, v[i].z, t[i].x, t[i].y });
		}
		color = c;
		model = glm::mat4(1.0f);
		texture = 0;
		this->drawMode = drawMode;

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
			vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	void transformation(glm::mat4 t)
	{
		model = t;
	}

	void setTexture(GLuint &t)
	{
		texture = t;
	}

	void draw(Shader &shader)
	{
		shader.setVec3("objectColor", color);
		shader.setMat4("model", model);
		glBindTexture(GL_TEXTURE_2D, texture);
		glBindVertexArray(VAO);
		glDrawArrays(drawMode, 0, static_cast<GLsizei>(vertices.size() / 5));
	}

	void deleteBuffers()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		VAO = 0;
		VBO = 0;
	}

private:
	std::vector<float> vertices;
	glm::vec3 color;
	glm::mat4 model;
	GLuint texture;
	GLuint VAO = 0;
	GLuint VBO = 0;
	GLenum drawMode = GL_TRIANGLE_FAN;
	PolygonWithTexture();
};
