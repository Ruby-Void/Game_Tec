#pragma once
#include "../../Common/stb/stb_image.h"

namespace NCL {
	namespace CSC8503 {
		class State {
		public:
			State() {}
			virtual ~State() {}
			virtual void Update() = 0; //Pure virtual base class
		};

		typedef void(*StateFunc)(void*, void*);

		class GenericState : public State {
		public:
			GenericState(StateFunc someFunc, void* someDataA, void* someDataB) {
				func		= someFunc;
				funcDataA	= someDataA;
				funcDataB	= someDataB;
			}
			virtual void Update() {
				if (funcDataA != nullptr && funcDataB != nullptr) { func(funcDataA, funcDataB); }
			}
		protected:
			StateFunc func;
			void* funcDataA;
			void* funcDataB;
		};
	}
}