#include "pch.h"
#include "Globals/AppGlobals.h"
#include "Services/AppService.h"
#include "State/DebugPageState.h"
#include "States/DebugState.h"
#include "UI/DebugPage.xaml.h"
#include "UI/XamlGlobalState.h"
#include "UI/XamlView.h"
#include "UI/XamlViewState.h"

ReTron::DebugState::DebugState(IAppService* appService)
	: _appService(appService)
	, _visible(false)
{
	_customDebugCookie = _appService->GetAppGlobals().GetDebugPageState()->CustomDebugEvent().Add(
		[this]()
		{
			SetVisible(!GetVisible());
		});

	_debugPage = *new ReTron::DebugPage(_appService);

	std::shared_ptr<ff::XamlView> view = _appService->GetXamlGlobals().CreateView(_debugPage);
	_viewState = std::make_shared<ff::XamlViewState>(view, _appService->GetTempTarget(TempTargets::RgbPma2), _appService->GetTempDepth(TempTargets::RgbPma2));
}

ReTron::DebugState::~DebugState()
{
	_appService->GetAppGlobals().GetDebugPageState()->CustomDebugEvent().Remove(_customDebugCookie);
}

bool ReTron::DebugState::GetVisible() const
{
	return _visible;
}

void ReTron::DebugState::SetVisible(bool visible)
{
	if (_viewState)
	{
		_visible = visible;
	}
}

void ReTron::DebugState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	if (_visible)
	{
		_appService->ClearTempTargets(TempTargets::RgbPma2);

		ff::State::Render(globals, target, depth);

		_appService->RenderTempTargets(TempTargets::RgbPma2, target);
	}
}

size_t ReTron::DebugState::GetChildStateCount()
{
	return _visible ? 1 : 0;
}

ff::State* ReTron::DebugState::GetChildState(size_t index)
{
	return _viewState.get();
}
