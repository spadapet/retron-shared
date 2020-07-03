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
	std::shared_ptr<ff::XamlView> view = _appService->GetXamlGlobals().CreateView(ff::String::from_static(L"TitlePage.xaml"));
	_viewState = std::make_shared<ff::XamlViewState>(view, appService->GetXamlTarget(), appService->GetXamlDepth());

	Noesis::FrameworkElement* root = view->GetContent();
	Noesis::Button* playGameButton = root->FindName<Noesis::Button>("playGameButton");

	playGameButton->Click() += Noesis::MakeDelegate(this, &TitleState::OnClickPlayGame);
}

std::shared_ptr<ff::State> Game::TitleState::Advance(ff::AppGlobals* globals)
{
	ff::State::Advance(globals);
	return _pendingState;
}

void Game::TitleState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	ff::State::Render(globals, target, depth);

	ff::RendererActive render = ff::PixelRendererActive::BeginRender(_render.get(), _appService->GetLowTarget(), _appService->GetLowDepth(), Constants::RENDER_RECT, Constants::RENDER_RECT);

	if (_font.HasObject())
	{
		ff::String text = ff::GetThisModule().GetString(ff::String::from_static(L"titleIntro"));
		ff::Transform transform = ff::Transform::Identity();
		transform._position.SetPoint(8, 8);
		ff::PaletteIndexToColor(252, transform._color);
		_font->DrawText(render, text, transform, ff::GetColorNone(), ((globals->GetGlobalTime()._advanceCount % 60) < 30) ? ff::SpriteFontOptions::NoOutline : ff::SpriteFontOptions::None);
	}
}

size_t Game::TitleState::GetChildStateCount()
{
	return 1;
}

ff::State* Game::TitleState::GetChildState(size_t index)
{
	return _viewState.get();
}

void Game::TitleState::OnClickPlayGame(Noesis::BaseComponent* sender, const Noesis::RoutedEventArgs& args)
{
	// _pendingState = std::make_shared<PlayGameState>(_appService, _appService->GetGameOptions());
}
