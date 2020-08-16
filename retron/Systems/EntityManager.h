#pragma once

namespace ReTron
{
	class EntityManager
	{
	public:
		EntityManager(entt::registry& registry);

		void FlushDelete();
		void DelayDelete(entt::entity entity);

		entt::entity CreatePlayer(size_t index);

	private:
		entt::registry& _registry;
	};
}
