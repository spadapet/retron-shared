#pragma once

#include "UI/Utility/NotifyPropertyChangedBase.h"

namespace ReTron
{
	class DebugState;
	class IAppService;

	class DebugPageViewModel : public ff::NotifyPropertyChangedBase
	{
	public:
		DebugPageViewModel(IAppService* appService, DebugState* debugState);

	private:
		void RestartLevel(Noesis::BaseComponent* param);
		void RestartGame(Noesis::BaseComponent* param);
		void RebuildResources(Noesis::BaseComponent* param);
		void CloseDebug(Noesis::BaseComponent* param);

		IAppService* _appService;
		DebugState* _debugState;
		Noesis::Ptr<Noesis::ICommand> _restartLevelCommand;
		Noesis::Ptr<Noesis::ICommand> _restartGameCommand;
		Noesis::Ptr<Noesis::ICommand> _rebuildResourcesCommand;
		Noesis::Ptr<Noesis::ICommand> _closeDebugCommand;

		NS_DECLARE_REFLECTION(DebugPageViewModel, ff::NotifyPropertyChangedBase);
	};

	class DebugPage : public Noesis::UserControl
	{
	public:
		DebugPage(IAppService* appService, DebugState* debugState);

		DebugPageViewModel* GetViewModel() const;

	private:
		IAppService* _appService;
		Noesis::Ptr<DebugPageViewModel> _viewModel;

		NS_DECLARE_REFLECTION(DebugPage, Noesis::UserControl);
	};
}
