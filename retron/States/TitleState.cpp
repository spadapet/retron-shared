#include "pch.h"
#include "Globals/AppGlobals.h"
#include "Graph/Anim/Transform.h"
#include "Graph/Font/SpriteFont.h"
#include "Graph/GraphDevice.h"
#include "Graph/Render/PixelRenderer.h"
#include "Graph/Render/Renderer.h"
#include "Graph/Render/RendererActive.h"
#include "Graph/RenderTarget/RenderTarget.h"
#include "Graph/Texture/Texture.h"
#include "Services/AppService.h"
#include "States/TitleState.h"
#include "UI/TitlePage.xaml.h"
#include "UI/XamlGlobalState.h"
#include "UI/XamlView.h"
#include "UI/XamlViewState.h"

ReTron::TitleState::TitleState(IAppService* appService)
	: _appService(appService)
{
	_titlePage = *new ReTron::TitlePage(_appService);

	std::shared_ptr<ff::XamlView> view = _appService->GetXamlGlobals().CreateView(_titlePage);
	_viewState = std::make_shared<ff::XamlViewState>(view, _appService->GetXamlTarget(), _appService->GetXamlDepth());
}

std::shared_ptr<ff::State> ReTron::TitleState::Advance(ff::AppGlobals* globals)
{
	ff::State::Advance(globals);

	return _titlePage->GetViewModel()->GetPendingState();
}

void ReTron::TitleState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	_appService->ClearLowTargets();

	ff::State::Render(globals, target, depth);

	_appService->RenderLowTargets(target);
}

size_t ReTron::TitleState::GetChildStateCount()
{
	return 1;
}

ff::State* ReTron::TitleState::GetChildState(size_t index)
{
	return _viewState.get();
}
