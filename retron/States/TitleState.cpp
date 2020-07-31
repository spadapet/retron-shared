#include "pch.h"
#include "Services/AppService.h"
#include "States/TitleState.h"
#include "States/TransitionState.h"
#include "UI/TitlePage.xaml.h"
#include "UI/XamlGlobalState.h"
#include "UI/XamlView.h"
#include "UI/XamlViewState.h"

ReTron::TitleState::TitleState(IAppService* appService)
	: _appService(appService)
{
	_titlePage = *new ReTron::TitlePage(_appService);

	std::shared_ptr<ff::XamlView> view = _appService->GetXamlGlobals().CreateView(_titlePage);
	_viewState = std::make_shared<ff::XamlViewState>(view, _appService->GetTempTarget(TempTargets::RgbPma2), _appService->GetTempDepth(TempTargets::RgbPma2));
}

std::shared_ptr<ff::State> ReTron::TitleState::Advance(ff::AppGlobals* globals)
{
	ff::State::Advance(globals);

	std::shared_ptr<ff::State> newState = _titlePage->GetViewModel()->GetPendingState();
	if (newState)
	{
		newState = std::make_shared<TransitionState>(_appService, shared_from_this(), newState, ff::String::from_static(L"transition-bg-2.png"));
	}

	return newState;
}

void ReTron::TitleState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	_appService->ClearTempTargets(TempTargets::RgbPma2);

	ff::State::Render(globals, target, depth);

	_appService->RenderTempTargets(TempTargets::RgbPma2, target);
}

size_t ReTron::TitleState::GetChildStateCount()
{
	return 1;
}

ff::State* ReTron::TitleState::GetChildState(size_t index)
{
	return _viewState.get();
}
