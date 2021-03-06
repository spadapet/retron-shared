﻿#pragma once
#if METRO_APP

#include "App.g.h"
#include "Globals/MetroGlobals.h"
#include "Globals/ProcessGlobals.h"

namespace ReTron
{
	class AppState;

	ref class App sealed
	{
	public:
		App();
		virtual ~App();

		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args) override;

	private:
		void InitializeGlobals();

		ff::ProcessGlobals _processGlobals;
		ff::MetroGlobals _globals;
		std::shared_ptr<AppState> _appState;
	};
}

#endif
