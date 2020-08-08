#pragma once

namespace ReTron
{
	class ILevelService;

	class LevelAdvance
	{
	public:
		LevelAdvance(ILevelService* levelService);

		void Advance();

	private:
		ILevelService* _levelService;
	};
}
