#include "libs/glad_loader_docs.h"
#include <GLFW/glfw3.h>
#include <learnopengl/shader_m.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>
#include "assist/Shared/RenderUtils.cpp"
#include "assist/RoomSofa/Room.cpp"
#include "assist/RoomSofa/Sofa.cpp"
#include "assist/RoomSofa/Decor.cpp"
#include "assist/TableCards/Table.cpp"
#include "assist/TableCards/Cards.cpp"
#include "assist/Animation/Animation.cpp"
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <system_error>

namespace
{
	constexpr int WindowWidth = 1280;
	constexpr int WindowHeight = 720;

	GLFWwindow* window = nullptr;
	int framebufferWidth = WindowWidth;
	int framebufferHeight = WindowHeight;
	bool oneKeyDown = false;
	bool twoKeyDown = false;
	bool threeKeyDown = false;
	bool gKeyDown = false;
	bool playerView = false;
	bool roomView = true;
	float cameraBlend = 0.0f;
	float roomBlend = 0.0f;
	float lastFrame = 0.0f;

	void processInput(scene::Animation& animation)
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		const bool onePressed = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
		if (onePressed && !oneKeyDown)
		{
			playerView = false;
			roomView = false;
		}
		oneKeyDown = onePressed;

		const bool twoPressed = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
		if (twoPressed && !twoKeyDown)
		{
			playerView = true;
			roomView = false;
		}
		twoKeyDown = twoPressed;

		const bool threePressed = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;
		if (threePressed && !threeKeyDown)
		{
			playerView = false;
			roomView = true;
		}
		threeKeyDown = threePressed;

		const bool gPressed = glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS;
		if (gPressed && !gKeyDown)
			animation.playRandomCard();
		gKeyDown = gPressed;
	}

	void resizeCallback(GLFWwindow*, int width, int height)
	{
		framebufferWidth = std::max(width, 1);
		framebufferHeight = std::max(height, 1);
		glViewport(0, 0, framebufferWidth, framebufferHeight);
	}

	void cleanupModules(scene::Room& room, scene::Table& table,
		scene::Sofa& sofa, scene::Decor& decor, scene::Cards& cards)
	{
		cards.cleanup();
		decor.cleanup();
		sofa.cleanup();
		table.cleanup();
		room.cleanup();
	}
}

int main(int, char* argv[])
{
	std::error_code pathError;
	const std::filesystem::path executablePath =
		std::filesystem::absolute(argv[0], pathError);
	if (pathError)
	{
		std::cerr << "Failed to resolve the executable path: " << pathError.message() << '\n';
		return 1;
	}
	std::filesystem::current_path(executablePath.parent_path(), pathError);
	if (pathError)
	{
		std::cerr << "Failed to set the runtime asset directory: " << pathError.message() << '\n';
		return 1;
	}

	if (!glfwInit())
	{
		std::cerr << "Failed to initialize GLFW.\n";
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(WindowWidth, WindowHeight,
		"Card Table | 1: overhead  2: player view  3: room view  G: play", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Failed to create the OpenGL window.\n";
		glfwTerminate();
		return 1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, resizeCallback);
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		std::cerr << "Failed to load OpenGL functions.\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
	framebufferWidth = std::max(framebufferWidth, 1);
	framebufferHeight = std::max(framebufferHeight, 1);
	glViewport(0, 0, framebufferWidth, framebufferHeight);

	Shader shader("./shaders/vs/L4.vs", "./shaders/fs/L4.fs");
	GLint linkStatus = GL_FALSE;
	if (shader.ID)
		glGetProgramiv(shader.ID, GL_LINK_STATUS, &linkStatus);
	if (!shader.ID || linkStatus != GL_TRUE)
	{
		std::cerr << "Failed to create a usable OpenGL shader program.\n";
		if (shader.ID)
			glDeleteProgram(shader.ID);
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}
	shader.use();
	shader.setInt("texture_diffuse1", 0);
	shader.setBool("useLighting", false);
	shader.setBool("useSofaBodyTones", false);

	scene::Room room;
	scene::Table table;
	scene::Sofa sofa;
	scene::Decor decor;
	scene::Cards cards;
	scene::Animation animation;

	try
	{
		stbi_set_flip_vertically_on_load(true);
		room.initialize();
		table.initialize();
		sofa.initialize();
		decor.initialize();
		cards.initialize();
	}
	catch (const std::exception& error)
	{
		std::cerr << "Initialization failed: " << error.what() << '\n';
		cleanupModules(room, table, sofa, decor, cards);
		glDeleteProgram(shader.ID);
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	try
	{
		while (!glfwWindowShouldClose(window))
		{
			const float now = static_cast<float>(glfwGetTime());
			const float deltaTime = std::min(now - lastFrame, 0.05f);
			lastFrame = now;
			processInput(animation);
			const float cameraStep = std::min(1.0f, deltaTime * 2.8f);
			animation.update(deltaTime);
			cameraBlend += ((playerView ? 1.0f : 0.0f) - cameraBlend) *
				cameraStep;
			roomBlend += ((roomView ? 1.0f : 0.0f) - roomBlend) * cameraStep;

			glClearColor(0.035f, 0.04f, 0.055f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			shader.use();

			const float aspect = static_cast<float>(framebufferWidth) / framebufferHeight;
			constexpr float overheadHalfHeight = 6.5f;
			const glm::mat4 overhead = glm::ortho(
				-overheadHalfHeight * aspect, overheadHalfHeight * aspect,
				-overheadHalfHeight, overheadHalfHeight, 0.1f, 60.0f);
			const glm::mat4 perspectiveProjection =
				glm::perspective(glm::radians(55.0f), aspect, 0.1f, 60.0f);
			const float overheadWeight = (1.0f - roomBlend) * (1.0f - cameraBlend);
			const float playerWeight = (1.0f - roomBlend) * cameraBlend;
			const float roomWeight = roomBlend;
			const float perspectiveWeight = playerWeight + roomWeight;
			shader.setMat4("projection",
				overhead * overheadWeight + perspectiveProjection * perspectiveWeight);

			const glm::vec3 overheadEye(0.0f, 12.0f, scene::TableDepthOffset);
			const glm::vec3 playerEye(0.0f, 3.6f, 7.4f + scene::TableDepthOffset);
			const glm::vec3 roomEye(0.0f, 12.0f, 18.0f);
			const glm::vec3 overheadTarget(0.0f, 0.0f, scene::TableDepthOffset);
			const glm::vec3 playerTarget(0.0f, 0.8f, -1.0f + scene::TableDepthOffset);
			const glm::vec3 roomTarget(0.0f, 1.0f, -4.5f);
			const glm::vec3 overheadUp(0.0f, 0.0f, -1.0f);
			const glm::vec3 playerUp(0.0f, 1.0f, 0.0f);
			const glm::vec3 eye = overheadEye * overheadWeight +
				playerEye * playerWeight + roomEye * roomWeight;
			const glm::vec3 target = overheadTarget * overheadWeight +
				playerTarget * playerWeight + roomTarget * roomWeight;
			shader.setVec3("viewPosition", eye);
			const glm::vec3 up = glm::normalize(
				overheadUp * overheadWeight + playerUp * perspectiveWeight);
			shader.setMat4("view", glm::lookAt(eye, target, up));

			room.render(shader);
			table.render(shader);
			sofa.render(shader);
			decor.render(shader);
			animation.render(shader, cards, perspectiveWeight);

			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}
	catch (const std::exception& error)
	{
		std::cerr << "Rendering stopped: " << error.what() << '\n';
		cleanupModules(room, table, sofa, decor, cards);
		glDeleteProgram(shader.ID);
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	cleanupModules(room, table, sofa, decor, cards);
	glDeleteProgram(shader.ID);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
