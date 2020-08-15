#pragma once

#include "Core/RenderTargets.h"
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
	class DebugPageViewModel;

	class DebugState : public ff::State
	{
	public:
		DebugState(IAppService* appService);
		virtual ~DebugState() override;

		bool GetVisible() const;
		void SetVisible(const std::shared_ptr<ff::State>& underState);
		void Hide();

		// State
		virtual void Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;
		virtual size_t GetChildStateCount() override;
		virtual State* GetChildState(size_t index) override;

		// UI events
		entt::sink<void()> RestartLevelEvent();
		entt::sink<void()> RestartGameEvent();
		entt::sink<void()> RebuildResourcesEvent();

	private:
		friend class DebugPageViewModel;

		IAppService* _appService;
		RenderTargets _targets;
		Noesis::Ptr<DebugPage> _debugPage;
		std::shared_ptr<ff::XamlViewState> _viewState;
		std::shared_ptr<ff::State> _underState;
		entt::sigh<void()> _restartLevelEvent;
		entt::sigh<void()> _restartGameEvent;
		entt::sigh<void()> _rebuildResourcesEvent;
	};
}
