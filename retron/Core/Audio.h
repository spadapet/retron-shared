#pragma once

namespace ff
{
	class AppGlobals;
}

namespace ReTron
{
	class Audio
	{
	public:
		Audio(ff::AppGlobals* globals);

	private:
		ff::AppGlobals* _globals;
	};
}
