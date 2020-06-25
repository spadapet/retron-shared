#pragma once

#include "Resource/ResourceValue.h"
#include "State/State.h"

namespace ff
{
	class XamlView;
	class IRenderer;
	class ISpriteFont;
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
		std::unique_ptr<ff::IRenderer> _render;
		ff::TypedResource<ff::ISpriteFont> _font;
	};
}
