#pragma once

#include "../Shared/RenderUtils.cpp"

#include <learnopengl/model.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace scene
{
	class Sofa
	{
	public:
		void initialize()
		{
			const std::string modelPath = "./models/sofa/sofa.obj";
			const std::string materialPath = "./models/sofa/sofa.mtl";
			std::ifstream modelFile(modelPath, std::ios::binary);
			std::ifstream materialFile(materialPath, std::ios::binary);
			if (!modelFile || !materialFile)
				throw std::runtime_error("Unable to open the sofa OBJ or its associated MTL file.");

			Assimp::Importer materialImporter;
			const aiScene* importedScene = materialImporter.ReadFile(modelPath,
				aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
				aiProcess_CalcTangentSpace);
			if (!importedScene || (importedScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) ||
				!importedScene->mRootNode)
				throw std::runtime_error("Unable to parse sofa model and materials: " +
					std::string(materialImporter.GetErrorString()));

			materialColors.clear();
			materialNames.clear();
			collectMaterialColors(importedScene->mRootNode, importedScene);
			if (materialColors.empty() || materialNames.size() != materialColors.size())
				throw std::runtime_error("The sofa model contains no materials or meshes.");

			model = std::make_unique<Model>(modelPath);
			if (model->meshes.empty() || model->meshes.size() != materialColors.size())
				throw std::runtime_error("The sofa model did not load all expected material meshes.");

			glm::vec3 minimum(std::numeric_limits<float>::max());
			glm::vec3 maximum(std::numeric_limits<float>::lowest());
			for (const Mesh& mesh : model->meshes)
				for (const Vertex& vertex : mesh.vertices)
				{
					minimum = glm::min(minimum, vertex.Position);
					maximum = glm::max(maximum, vertex.Position);
				}

			const glm::vec3 extent = maximum - minimum;
			const float largestDimension = std::max(extent.x, std::max(extent.y, extent.z));
			if (!std::isfinite(largestDimension) || largestDimension <= 0.0f)
				throw std::runtime_error("The sofa model has invalid vertex bounds.");

			const float uniformScale = 8.0f / largestDimension;
			const glm::vec3 modelAnchor((minimum.x + maximum.x) * 0.5f, minimum.y,
				(minimum.z + maximum.z) * 0.5f);
			modelMatrix =
				glm::translate(glm::mat4(1.0f), glm::vec3(-11.0f, -1.5f, -9.5f)) *
				glm::scale(glm::mat4(1.0f), glm::vec3(uniformScale)) *
				glm::translate(glm::mat4(1.0f), -modelAnchor);
		}

		void render(Shader& shader)
		{
			if (!model || model->meshes.size() != materialColors.size() ||
				model->meshes.size() != materialNames.size())
				throw std::logic_error("Sofa model materials were not initialized.");

			shader.setVec2("textureScale", glm::vec2(1.0f));
			shader.setMat4("model", modelMatrix);
			for (size_t i = 0; i < model->meshes.size(); ++i)
			{
				const Mesh& mesh = model->meshes[i];
				const bool hasDiffuseTexture = std::any_of(mesh.textures.begin(), mesh.textures.end(),
					[](const Texture& texture) { return texture.type == "texture_diffuse"; });
				shader.setBool("useTexture", hasDiffuseTexture);
				shader.setVec3("objectColor", materialColors[i]);
				shader.setBool("useSofaBodyTones", materialNames[i] == "m_sofa");
				model->meshes[i].Draw(shader);
			}
			shader.setBool("useSofaBodyTones", false);
			glActiveTexture(GL_TEXTURE0);
		}

		void cleanup()
		{
			model.reset();
			materialColors.clear();
			materialNames.clear();
		}

	private:
		void collectMaterialColors(const aiNode* node, const aiScene* importedScene)
		{
			for (unsigned int i = 0; i < node->mNumMeshes; ++i)
			{
				const aiMesh* mesh = importedScene->mMeshes[node->mMeshes[i]];
				const aiMaterial* material = importedScene->mMaterials[mesh->mMaterialIndex];
				aiColor3D diffuse(1.0f, 1.0f, 1.0f);
				material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
				aiString materialName;
				material->Get(AI_MATKEY_NAME, materialName);
				materialNames.emplace_back(materialName.C_Str());

				if (materialName == aiString("m_sofa"))
					materialColors.emplace_back(0.91f, 0.86f, 0.75f);
				else if (materialName == aiString("m_sofa_bottom"))
					materialColors.emplace_back(0.68f, 0.61f, 0.51f);
				else if (materialName == aiString("Material.002"))
					materialColors.emplace_back(0.77f, 0.68f, 0.91f);
				else if (materialName == aiString("Material.003"))
					materialColors.emplace_back(0.94f, 0.65f, 0.68f);
				else if (materialName == aiString("Material.004"))
					materialColors.emplace_back(0.94f, 0.77f, 0.60f);
				else
					materialColors.emplace_back(diffuse.r, diffuse.g, diffuse.b);
			}

			for (unsigned int i = 0; i < node->mNumChildren; ++i)
				collectMaterialColors(node->mChildren[i], importedScene);
		}

		std::unique_ptr<Model> model;
		std::vector<glm::vec3> materialColors;
		std::vector<std::string> materialNames;
		glm::mat4 modelMatrix{ 1.0f };
	};
}
