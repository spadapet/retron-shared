#pragma once

namespace ReTron
{
	class ILevelService;

	class EntityFactory
	{
	public:
		EntityFactory(ILevelService* levelService);

		void InitLevel();

	private:
		ILevelService* _levelService;
	};
}
