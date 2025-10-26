#pragma once

#include <functional>
#include <map>

#include "Headers.h"

namespace GameImports {
	class GameService;

	/**
	 * Maps X to Action<Y>, so to a function that takes Y as parameter
	 * Has to be a map instead of a list, because once we bind an obj we cannot
	 * really access it anymore -> store a lookup
	 */
	template <class X, class Y>
	class ActionMap {

	public:
		std::map<X, std::pair<Action<Y>, bool>> Actions;

		void Add(X Type, Action<Y> Action, bool bDeleteAfterUse = true) {
			Actions.insert(
				std::make_pair(
					Type,
					std::make_pair(Action, bDeleteAfterUse)
				)
			);
		}

		void Remove(X Type) {
			Actions.erase(Type);
		}
				
		void ForEach(Y t) {
			for (auto it = Actions.rbegin(); it != Actions.rend(); ++it)
			{
				auto& Tuple = it->second;
				auto& Action = std::get<0>(Tuple);
				if (!Action)
					continue;
				
				Action(t);
				auto& bDeleteAfterUse = std::get<1>(Tuple);
				if (!bDeleteAfterUse)
					continue;

				Actions.erase(it->first);
				// have to actually break, otherwise the "++it" in the
				// loop triggers assertion since we modified the map
				if (it == Actions.rend())
					break;
				
				std::advance(it, 1);
			}
		}

		bool Contains(X Type)
		{
			return Actions.contains(Type);
		}
		
	};

}
