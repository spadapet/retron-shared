#pragma once

namespace ReTron
{
	struct Values
	{
		void Load();

		static const int CurrentVersion = 1;

		int _version;
		bool _allowDebug;

	private:
		void Init();
	};
}
