#pragma once

#include "Types/Event.h"
#include "UI/Utility/NotifyPropertyChangedBase.h"

namespace ff
{
	class IRenderTargetWindow;
	class State;
}

namespace ReTron
{
	class IAppService;

	class DebugPageViewModel : public ff::NotifyPropertyChangedBase
	{
	public:
		DebugPageViewModel(IAppService* appService);

		ff::Event<void> RestartLevelEvent;
		ff::Event<void> RestartGameEvent;
		ff::Event<void> RestartAppEvent;
		ff::Event<void> RebuildResourcesEvent;

	private:
		void RestartLevel(Noesis::BaseComponent* param);
		void RestartGame(Noesis::BaseComponent* param);
		void RestartApp(Noesis::BaseComponent* param);
		void RebuildResources(Noesis::BaseComponent* param);

		IAppService* _appService;
		Noesis::Ptr<Noesis::ICommand> _restartLevelCommand;
		Noesis::Ptr<Noesis::ICommand> _restartGameCommand;
		Noesis::Ptr<Noesis::ICommand> _restartAppCommand;
		Noesis::Ptr<Noesis::ICommand> _rebuildResourcesCommand;

		NS_DECLARE_REFLECTION(DebugPageViewModel, ff::NotifyPropertyChangedBase);
	};

	class DebugPage : public Noesis::UserControl
	{
	public:
		DebugPage(IAppService* appService);

		DebugPageViewModel* GetViewModel() const;

	private:
		IAppService* _appService;
		Noesis::Ptr<DebugPageViewModel> _viewModel;

		NS_DECLARE_REFLECTION(DebugPage, Noesis::UserControl);
	};
}
