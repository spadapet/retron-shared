#pragma once

#include "Resource/ResourceValue.h"
#include "State/State.h"

namespace ff
{
	class XamlView;
	class XamlViewState;
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
		virtual void Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;
		virtual size_t GetChildStateCount() override;
		virtual State* GetChildState(size_t index) override;

	private:
		void OnClickPlayGame(Noesis::BaseComponent* sender, const Noesis::RoutedEventArgs& args);

		IAppService* _appService;
		std::shared_ptr<ff::XamlViewState> _viewState;
		std::shared_ptr<ff::State> _pendingState;
		std::unique_ptr<ff::IRenderer> _render;
		ff::TypedResource<ff::ISpriteFont> _font;
	};
}
