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
#include "UI/XamlGlobalState.h"
#include "UI/XamlView.h"
#include "UI/XamlViewState.h"

Game::TitleState::TitleState(IAppService* appService)
	: _appService(appService)
	, _render(appService->GetAppGlobals().GetGraph()->CreateRenderer())
	, _font(L"GameFont")
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

	ff::RendererActive render = ff::PixelRendererActive::BeginRender(_render.get(), _appService->GetLowTarget(), _appService->GetLowDepth(), Constants::RENDER_RECT, Constants::RENDER_RECT);

	if (_font.HasObject())
	{
		ff::String text = ff::GetThisModule().GetString(ff::String::from_static(L"titleIntro"));
		ff::Transform transform = ff::Transform::Identity();
		transform._position.SetPoint(8, 8);
		ff::PaletteIndexToColor(252, transform._color);
		_font->DrawText(render, text, transform, ((globals->GetGlobalTime()._advanceCount % 60) < 30) ? ff::GetColorWhite() : ff::GetColorNone());
	}
}

void Game::TitleState::OnClickPlayGame(Noesis::BaseComponent* sender, const Noesis::RoutedEventArgs& args)
{
	// _pendingState = std::make_shared<PlayGameState>(_appService, _appService->GetGameOptions());
}
