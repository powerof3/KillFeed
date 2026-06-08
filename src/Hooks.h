#pragma once

#include "CauseOfDeath.h"

namespace Hooks
{
	namespace ActiveEffect
	{
		template <class T>
		struct Finish
		{
			static void thunk(T* a_this)
			{
				if (auto commandedActor = a_this->commandedActor.get()) {
					CauseOfDeathManager::GetSingleton()->InsertIntoCommandedActorMap(commandedActor->GetFormID(), a_this->caster);
				}

				func(a_this);
			}
			static inline REL::Relocation<decltype(thunk)> func;
			static inline constexpr std::size_t            idx{ 0x15 };

			static void Install()
			{
				stl::write_vfunc<T, Finish>();
			}
		};
	}

	void Install();
}
