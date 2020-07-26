#pragma once

#include "Resource/ResourceValue.h"
#include "State/State.h"

namespace ff
{
	class XamlViewState;
	class ITexture;
}

namespace ReTron
{
	class IAppService;
	class IGameService;
	class TitlePage;

	class TitleState : public ff::State
	{
	public:
		TitleState(IAppService* appService);

		virtual std::shared_ptr<ff::State> Advance(ff::AppGlobals* globals) override;
		virtual void Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;
		virtual size_t GetChildStateCount() override;
		virtual State* GetChildState(size_t index) override;

	private:
		IAppService* _appService;
		Noesis::Ptr<TitlePage> _titlePage;
		std::shared_ptr<ff::XamlViewState> _viewState;
	};
}
