	#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <filesystem>

#include "../math/math.hpp"

namespace cs_std::graphics
{
	/// <summary>
	/// Defines attribute flags for a mesh
	/// Based off the minimum required attributes for a gltf compliant renderer
	/// </summary>
	enum class Attribute: uint32_t
	{
		POSITION = 0,
		NORMAL,
		TANGENT,
		TEXCOORD_0,
		TEXCOORD_1,
		COLOR_0,
		JOINTS_0,
		WEIGHTS_0,
		UNKNOWN,
		// Must be last
		ATTRIBUTE_COUNT
	};

	// Specifies the data for a single attribute
	struct shader_attribute
	{
		std::vector<float> data;
		Attribute attribute;

		// Comparator for sorting
		bool operator<(const shader_attribute& other) const {
			return attribute < other.attribute;
		}
	};

	// Specifies the data for a mesh
	struct mesh
	{
		std::vector<shader_attribute> attributes;
		std::vector<uint32_t> indices;

		// Constructor
		mesh() = default;
		mesh(const std::vector<shader_attribute>& attributes, const std::vector<uint32_t>& indices)
			: attributes(attributes), indices(indices) {}
		// Destructor
		~mesh() = default;
		// Copy
		mesh(const mesh& other) = default;
		mesh& operator=(const mesh& other) = default;
		// Move
		mesh(mesh&& other) noexcept = default;
		mesh& operator=(mesh&& other) noexcept = default;

		const shader_attribute& get_attribute(Attribute attribute) const
		{
			for (const shader_attribute& attr : this->attributes)
			{
				if (attr.attribute == attribute) return attr;
			}
			return this->attributes[0];
		}
		shader_attribute& get_attribute(Attribute attribute)
		{
			for (shader_attribute& attr : this->attributes)
			{
				if (attr.attribute == attribute) return attr;
			}
			return this->attributes[0];
		}
		bool has_attribute(Attribute attribute) const
		{
			for (const shader_attribute& attr : this->attributes)
			{
				if (attr.attribute == attribute) return true;
			}
			return false;
		}
		// Insert in-place according to Attribute index
		mesh& add_attribute(Attribute attribute, const std::vector<float>& data)
		{
			this->attributes.push_back({ data, attribute });
			std::sort(this->attributes.begin(), this->attributes.end());
			return *this;
		}
	};

	// Attributes about the mesh such as it's transform, textures, etc
	struct mesh_attributes
	{
		// Somtimes metallic and roughness are combined into a single texture
		// We will store both paths in that case, SSO will make sure theres not much overhead
		std::filesystem::path diffuse, normal, metallic, roughness, metallicRoughness, emissive, occlusion;
		math::mat4 transform;
		bool isDoubleSided, isTransparent;

		// Constructor
		mesh_attributes() = default;
		// Destructor
		~mesh_attributes() = default;
		// Copy
		mesh_attributes(const mesh_attributes& other) = default;
		mesh_attributes& operator=(const mesh_attributes& other) = default;
		// Move
		mesh_attributes(mesh_attributes&& other) noexcept = default;
		mesh_attributes& operator=(mesh_attributes&& other) noexcept = default;

		mesh_attributes& set_diffuse(const std::filesystem::path& diffuse) { this->diffuse = diffuse; return *this; }
		mesh_attributes& set_normal(const std::filesystem::path& normal) { this->normal = normal; return *this; }
		mesh_attributes& set_metallic(const std::filesystem::path& metallic) { this->metallic = metallic; return *this; }
		mesh_attributes& set_roughness(const std::filesystem::path& roughness) { this->roughness = roughness; return *this; }
		mesh_attributes& set_metallic_roughness(const std::filesystem::path& metallicRoughness) { this->metallicRoughness = metallicRoughness; return *this; }
		mesh_attributes& set_emissive(const std::filesystem::path& emissive) { this->emissive = emissive; return *this; }
		mesh_attributes& set_occlusion(const std::filesystem::path& occlusion) { this->occlusion = occlusion; return *this; }
		mesh_attributes& set_transform(const math::mat4& transform) { this->transform = transform; return *this; }
		mesh_attributes& set_double_sided(bool isDoubleSided) { this->isDoubleSided = isDoubleSided; return *this; }
		mesh_attributes& set_transparent(bool isTransparent) { this->isTransparent = isTransparent; return *this; }
	};

	struct model
	{
		// Mesh data
		std::vector<mesh> meshes;
		// Mesh attributes
		std::vector<mesh_attributes> meshAttributes;

		// Constructor
		model() = default;
		model(const std::vector<mesh>& meshes, const std::vector<mesh_attributes>& meshTexturePaths) : meshes(meshes), meshAttributes(meshAttributes) {}
		// Destructor
		~model() = default;
		// Copy
		model(const model& other) = default;
		model& operator=(const model& other) = default;
		// Move
		model(model&& other) noexcept = default;
		model& operator=(model&& other) noexcept = default;

		void add_mesh(const mesh& mesh, const mesh_attributes& meshAttributes)
		{
			this->meshes.push_back(mesh);
			this->meshAttributes.push_back(meshAttributes);
		}
	};
}