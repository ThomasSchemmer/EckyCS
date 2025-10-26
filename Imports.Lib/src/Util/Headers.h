#pragma once
#include <functional>

namespace GameImports
{
	template <class T>
    using Action = std::function<void(T)>;
	
	inline bool Approximately(float A, float B)
	{
		return std::abs(A - B) < std::numeric_limits<float>::epsilon();
	}

}
