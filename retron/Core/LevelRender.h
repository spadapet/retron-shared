#pragma once

namespace ff
{
	class IRenderDepth;
	class IRenderTarget;
}

namespace ReTron
{
	class ILevelService;

	class LevelRender
	{
	public:
		LevelRender(ILevelService* levelService);

		void Render(ff::IRenderTarget* target, ff::IRenderDepth* depth);

	private:
		ILevelService* _levelService;
	};
}
