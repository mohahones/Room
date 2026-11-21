#include <glad/glad.h>
#include <stdexcept>
#include <utility>
#include <vector>
#include <learnopengl/shader_m.h>
#include <glm/glm.hpp>

class Polygon {
public:
	Polygon(std::vector<glm::vec3> v, glm::vec3 c)
		: vertices(std::move(v)), color(c), model(1.0f)
	{
		if (vertices.size() < 3)
			throw std::invalid_argument("A polygon requires at least three vertices.");

		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
			vertices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
		glEnableVertexAttribArray(0);
	}

	void transformation(glm::mat4 t) {
		model = t;
	}

	void draw(Shader& shader)
	{
		shader.setVec3("objectColor", color);
		shader.setMat4("model", model);
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(vertices.size()));
	}

	void deleteBuffers() {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		VAO = 0;
		VBO = 0;
	}

private:
	std::vector<glm::vec3> vertices;
	glm::vec3 color;
	glm::mat4 model;
	GLuint VAO = 0;
	GLuint VBO = 0;
	Polygon();
};
