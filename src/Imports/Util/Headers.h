#pragma once
#include <functional>

namespace GameImports
{
	template <class T>
    using Action = std::function<void(T)>;
}