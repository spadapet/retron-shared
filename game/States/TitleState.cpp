#include "pch.h"
#include "Globals/AppGlobals.h"
#include "Graph/Render/Renderer.h"
#include "Graph/Render/RendererActive.h"
#include "Graph/RenderTarget/RenderTarget.h"
#include "Graph/Texture/Texture.h"
#include "Services/AppService.h"
#include "States/TitleState.h"
#include "UI/XamlGlobalState.h"
#include "UI/XamlView.h"
#include "UI/XamlViewState.h"

Game::TitleState::TitleState(IAppService* appService)
	: _appService(appService)
{
	_view = _appService->GetXamlGlobals().CreateView(ff::String::from_static(L"TitlePage.xaml"), appService->GetXamlTarget());

	Noesis::FrameworkElement* root = _view->GetContent();
	Noesis::Button* playGameButton = root->FindName<Noesis::Button>("playGameButton");

	playGameButton->Click() += Noesis::MakeDelegate(this, &TitleState::OnClickPlayGame);
}

std::shared_ptr<ff::State> Game::TitleState::Advance(ff::AppGlobals* globals)
{
	_view->Advance();
	return _pendingState;
}

void Game::TitleState::OnFrameRendering(ff::AppGlobals* globals, ff::AdvanceType type)
{
	_view->PreRender();
}

void Game::TitleState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	_view->Render(_appService->GetXamlTarget(), _appService->GetXamlDepth());
}

void Game::TitleState::OnClickPlayGame(Noesis::BaseComponent* sender, const Noesis::RoutedEventArgs& args)
{
	// _pendingState = std::make_shared<PlayGameState>(_appService, _appService->GetGameOptions());
}
