#pragma once

#include <functional>
#include <vector>
#include "Headers.h"

namespace GameImports {
	template <class T>
	class ActionList {

	public:
		std::vector<Action<T>> Actions;

		void Add(Action<T> Action) {
			Actions.push_back(Action);
		}

		void Remove(const Action<T>& Action) {
			for (size_t i = 0; i < Actions.size(); i++) {
				if (!IsEqual(Action, Actions[i]))
					continue;

				Actions.erase(Actions.begin() + i);
			}
		}
				
		void ForEach(T t) {
			for (const auto& Action : Actions) {
				Action(t);
			}
		}

		static size_t GetAddress(Action<T> Action) {
			typedef void(fnType)(T*);
			fnType** fnPointer = Action.template target<fnType*>();
			return reinterpret_cast<size_t>(*fnPointer);
		}

		static bool IsEqual(Action<T> A, Action<T> B) {
			auto AddrA = GetAddress(A);
			auto AddrB = GetAddress(B);
			return AddrA == AddrB;
		}

	};

}
