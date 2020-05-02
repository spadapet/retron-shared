#pragma once

#include "State/State.h"

namespace ff
{
	class XamlView;
}

namespace Game
{
	class IAppService;
	class IGameService;

	class TitleState : public ff::State
	{
	public:
		TitleState(IAppService* appService);

		virtual std::shared_ptr<ff::State> Advance(ff::AppGlobals* globals) override;
		virtual void OnFrameRendering(ff::AppGlobals* globals, ff::AdvanceType type) override;
		virtual void Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;

	private:
		void OnClickPlayGame(Noesis::BaseComponent* sender, const Noesis::RoutedEventArgs& args);

		IAppService* _appService;
		std::shared_ptr<ff::XamlView> _view;
		std::shared_ptr<ff::State> _pendingState;
	};
}
