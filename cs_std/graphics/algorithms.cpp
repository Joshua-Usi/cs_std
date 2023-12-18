#include "algorithms.hpp"

#include "mikktspace/mikktspace.h"

#include <array>

namespace cs_std::graphics
{
	namespace mikktspace
	{
		constexpr int get_vertices_per_face(const SMikkTSpaceContext* pContext, const int iFace)
		{
			// We only support triangles
			return 3;
		}
		int get_face_count(const SMikkTSpaceContext* pContext) {
			const mesh* pMesh = static_cast<mesh*>(pContext->m_pUserData);
			return pMesh->indices.size() / 3;
		}
		void get_position(const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert) {
			const mesh* pMesh = static_cast<mesh*>(pContext->m_pUserData);
			const int index = pMesh->indices[iFace * 3 + iVert];
			memcpy(fvPosOut, &pMesh->get_attribute(Attribute::POSITION).data[index * 3], sizeof(float) * 3);
		}
		void get_normal(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert) {
			const mesh* pMesh = static_cast<mesh*>(pContext->m_pUserData);
			const int index = pMesh->indices[iFace * 3 + iVert];
			memcpy(fvNormOut, &pMesh->get_attribute(Attribute::NORMAL).data[index * 3], sizeof(float) * 3);
		}
		void get_tex_coord(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert) {
			const mesh* pMesh = static_cast<mesh*>(pContext->m_pUserData);
			const int index = pMesh->indices[iFace * 3 + iVert];
			memcpy(fvTexcOut, &pMesh->get_attribute(Attribute::TEXCOORD_0).data[index * 2], sizeof(float) * 2);
		}
		void set_tangent_space_basic(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert) {
			mesh* pMesh = static_cast<mesh*>(pContext->m_pUserData);
			const int index = pMesh->indices[iFace * 3 + iVert];

			std::vector<float>& tangents = pMesh->get_attribute(Attribute::TANGENT).data;

			tangents[index * 4 + 0] = fvTangent[0];
			tangents[index * 4 + 1] = fvTangent[1];
			tangents[index * 4 + 2] = fvTangent[2];
			tangents[index * 4 + 3] = fSign;
		}
	}
	bounding_aabb::bounding_aabb(const std::vector<float>& vertices)
	{
		if (vertices.size() == 0)
		{
			min = max = math::vec3(0.0f);
			return;
		}

		min.x = min.y = min.z = std::numeric_limits<float>::max();
		max.x = max.y = max.z = std::numeric_limits<float>::lowest();

		for (size_t i = 0; i < vertices.size(); i += 3) {
			min.x = std::min(min.x, vertices[i]);
			min.y = std::min(min.y, vertices[i + 1]);
			min.z = std::min(min.z, vertices[i + 2]);

			max.x = std::max(max.x, vertices[i]);
			max.y = std::max(max.y, vertices[i + 1]);
			max.z = std::max(max.z, vertices[i + 2]);
		}
	}
	bounding_aabb::bounding_aabb(const std::vector<bounding_aabb>& aabbs)
	{
		// Combine the aabbs into one large aabb
		min.x = min.y = min.z = std::numeric_limits<float>::max();
		max.x = max.y = max.z = std::numeric_limits<float>::lowest();

		for (const bounding_aabb& aabb : aabbs)
		{
			min.x = std::min(min.x, aabb.min.x);
			min.y = std::min(min.y, aabb.min.y);
			min.z = std::min(min.z, aabb.min.z);

			max.x = std::max(max.x, aabb.max.x);
			max.y = std::max(max.y, aabb.max.y);
			max.z = std::max(max.z, aabb.max.z);
		}
	}
	bounding_aabb& bounding_aabb::transform(const math::mat4& transform)
	{
		const math::vec3 transformedCorners[8]
		{
			transform * math::vec4(min.x, min.y, min.z, 1.0f),
			transform * math::vec4(min.x, min.y, max.z, 1.0f),
			transform * math::vec4(min.x, max.y, min.z, 1.0f),
			transform * math::vec4(min.x, max.y, max.z, 1.0f),
			transform * math::vec4(max.x, min.y, min.z, 1.0f),
			transform * math::vec4(max.x, min.y, max.z, 1.0f),
			transform * math::vec4(max.x, max.y, min.z, 1.0f),
			transform * math::vec4(max.x, max.y, max.z, 1.0f),
		};
		math::vec3	newMin = transformedCorners[0],
			newMax = transformedCorners[0];

		for (uint8_t i = 1; i < 8; i++)
		{
			newMin = math::min(newMin, transformedCorners[i]);
			newMax = math::max(newMax, transformedCorners[i]);
		}

		min = newMin;
		max = newMax;

		return *this;
	}
	bounding_aabb& bounding_aabb::merge(const bounding_aabb& other)
	{
		min.x = std::min(min.x, other.min.x);
		min.y = std::min(min.y, other.min.y);
		min.z = std::min(min.z, other.min.z);

		max.x = std::max(max.x, other.max.x);
		max.y = std::max(max.y, other.max.y);
		max.z = std::max(max.z, other.max.z);

		return *this;
	}
	frustum::frustum(const math::mat4& viewProjection)
	{
		const math::vec4 row[4] =
		{
			math::row(viewProjection, 0),
			math::row(viewProjection, 1),
			math::row(viewProjection, 2),
			math::row(viewProjection, 3)
		};

		const math::vec4 coefficient[6] =
		{
			row[3] - row[1],
			row[3] + row[1],
			row[3] - row[0],
			row[3] + row[0],
			row[3] - row[2],
			row[3] + row[2]
		};

		for (uint8_t i = 0; i < 6; i++)
		{
			const math::vec4 norm = math::normalize(coefficient[i]);
			planes[i].normal = math::vec3(norm);
			planes[i].distance = norm.w;
		}
	}
	bool frustum::intersects(const bounding_aabb& volume) const
	{
		const math::vec3& min = volume.min, max = volume.max;

		for (uint8_t i = 0; i < 6; i++)
		{
			const math::vec3& normal = planes[i].normal;
			const float distance = planes[i].distance;

			math::vec3 positiveVertex = min;
			if (normal.x >= 0.0f) positiveVertex.x = max.x;
			if (normal.y >= 0.0f) positiveVertex.y = max.y;
			if (normal.z >= 0.0f) positiveVertex.z = max.z;

			if (math::dot(normal, positiveVertex) + distance < 0.0f) return false;
		}
		return true;
	}
	bool frustum::contains(const bounding_aabb& volume) const {
		const std::array<math::vec3, 8> corners
		{ {
			{volume.min.x, volume.min.y, volume.min.z},
			{volume.min.x, volume.min.y, volume.max.z},
			{volume.min.x, volume.max.y, volume.min.z},
			{volume.min.x, volume.max.y, volume.max.z},
			{volume.max.x, volume.min.y, volume.min.z},
			{volume.max.x, volume.min.y, volume.max.z},
			{volume.max.x, volume.max.y, volume.min.z},
			{volume.max.x, volume.max.y, volume.max.z}
		} };

		for (const auto& p : planes)
		{
			bool allCornersInside = true;
			for (const auto& corner : corners)
			{
				float distance = math::dot(p.normal, corner) + p.distance;
				if (distance < 0) { allCornersInside = false; break; }
			}
			if (!allCornersInside) return false;
		}
		return true;
	}
	void generate_tangents(mesh& inputMesh)
	{
		if (!inputMesh.has_attribute(Attribute::TANGENT)) inputMesh.add_attribute(Attribute::TANGENT, {});

		inputMesh.get_attribute(Attribute::TANGENT).data.resize(inputMesh.get_attribute(Attribute::POSITION).data.size() / 3 * 4);

		SMikkTSpaceInterface interface;
		interface.m_getNumFaces = mikktspace::get_face_count;
		interface.m_getNumVerticesOfFace = mikktspace::get_vertices_per_face;
		interface.m_getPosition = mikktspace::get_position;
		interface.m_getNormal = mikktspace::get_normal;
		interface.m_getTexCoord = mikktspace::get_tex_coord;
		interface.m_setTSpaceBasic = mikktspace::set_tangent_space_basic;
		interface.m_setTSpace = nullptr;

		SMikkTSpaceContext context;
		context.m_pInterface = &interface;
		context.m_pUserData = &inputMesh;

		const bool result = genTangSpaceDefault(&context);
		// TODO we should do something if we error
	}
}