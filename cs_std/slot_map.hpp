#pragma once

#include "slot_map/slotmap.hpp"

namespace cs_std
{
	// Glue code for slot_map
	template<typename T>
	using slotmap = dod::slot_map<T, dod::slot_map_key32<T>>;
	template<typename T>
	using slotmap_key = dod::slot_map_key32<T>;
}