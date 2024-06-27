#pragma once

#include "model.hpp"

#include <vector>

namespace cs_std::graphics
{

	// Generate the tangents for a specific mesh
	void generate_tangents(mesh<uint32_t>& inputMesh);

	struct bounding_aabb
	{
		math::vec3 min, max;

		bounding_aabb() : min(0.0f), max(0.0f) {}
		bounding_aabb(const math::vec3& min, const math::vec3& max) : min(min), max(max) {}
		bounding_aabb(const std::vector<float>& vertices);
		bounding_aabb(const std::vector<bounding_aabb>& aabbs);

		bounding_aabb& transform(const math::mat4& transform);
		bounding_aabb& merge(const bounding_aabb& other);
	};

	struct frustum
	{
		struct plane { math::vec3 normal; float distance; };
		// top, bottom, right, left, far, near
		plane planes[6];

		frustum() = default;
		// Generate a view frustum from a given viewProjection, works with perspective and orthographic
		frustum(const math::mat4& viewProjection);
		// True if fully inside or partially intersects
		bool intersects(const bounding_aabb& volume) const;
		// True only if fully inside
		bool contains(const bounding_aabb& volume) const;
	};
}