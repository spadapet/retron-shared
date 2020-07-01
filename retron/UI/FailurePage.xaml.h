#pragma once

#include "UI/FailurePage.g.h"

namespace ff
{
	class AppGlobals;
}

namespace Game
{
	[Windows::UI::Xaml::Data::Bindable]
	public ref class FailurePage sealed
	{
	public:
		property Platform::String^ ErrorText { Platform::String^ get(); }

	internal:
		FailurePage(ff::AppGlobals* globals);

	private:
		Platform::String^ _error;
	};
}
