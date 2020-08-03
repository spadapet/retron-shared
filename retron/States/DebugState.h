#pragma once

#include "State/State.h"

namespace ff
{
	class XamlViewState;
	class ITexture;
}

namespace ReTron
{
	class IAppService;
	class DebugPage;

	class DebugState : public ff::State
	{
	public:
		DebugState(IAppService* appService);
		virtual ~DebugState() override;

		bool GetVisible() const;
		void SetVisible(bool visible);

		virtual void Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;
		virtual size_t GetChildStateCount() override;
		virtual State* GetChildState(size_t index) override;

	private:
		IAppService* _appService;
		ff::EventCookie _customDebugCookie;
		Noesis::Ptr<DebugPage> _debugPage;
		std::shared_ptr<ff::XamlViewState> _viewState;
		bool _visible;
	};
}
