#include "pch.h"
#include "App.xaml.h"
#include "Data/Data.h"
#include "Data/SavedData.h"
#include "Dict/Dict.h"
#include "Globals/AppGlobals.h"
#include "Globals/ProcessGlobals.h"
#include "Graph/GraphDevice.h"
#include "Graph/Render/Renderer.h"
#include "Graph/Render/RendererActive.h"
#include "Graph/RenderTarget/RenderDepth.h"
#include "Graph/RenderTarget/RenderTargetWindow.h"
#include "Graph/Texture/Palette.h"
#include "Graph/Texture/PaletteData.h"
#include "Graph/Texture/Texture.h"
#include "Module/Module.h"
#include "Resource/Resources.h"
#include "State/DebugPageState.h"
#include "State/States.h"
#include "States/AppState.h"
#include "States/TitleState.h"
#include "Value/Values.h"
#include "UI/Components.h"
#include "UI/XamlGlobalState.h"
#include "UI/XamlView.h"

Game::AppState::AppState(ff::ProcessGlobals* processGlobals, ff::AppGlobals* globals)
	: _processGlobals(processGlobals)
	, _globals(globals)
	, _viewport(ff::PointFloat(Constants::RENDER_WIDTH, Constants::RENDER_HEIGHT))
	, _debugSteppingFrames(false)
	, _debugStepOneFrame(false)
	, _debugTimeScale(1.0)
	, _customDebugCookie(nullptr)
	, _paletteData(L"palette")
{
}

void Game::AppState::AdvanceDebugInput(ff::AppGlobals* globals)
{
	bool debugEnabled = DEBUG;
	if (!debugEnabled)
	{
		_debugStepOneFrame = false;
		_debugSteppingFrames = false;
		_debugTimeScale = 1.0;
		return;
	}

	noAssertRet(_debugInput.HasObject());

	if (_debugInput->Advance(_debugInputDevices, GetAppGlobals().GetGlobalTime()._secondsPerAdvance))
	{
		if (_debugInput->HasStartEvent(InputEvents::ID_DEBUG_CANCEL_STEP_ONE_FRAME))
		{
			_debugStepOneFrame = false;
			_debugSteppingFrames = false;
		}

		if (_debugInput->HasStartEvent(InputEvents::ID_DEBUG_STEP_ONE_FRAME))
		{
			_debugStepOneFrame = _debugSteppingFrames;
			_debugSteppingFrames = true;
		}
	}

	if (_debugInput->GetDigitalValue(_debugInputDevices, InputEvents::ID_DEBUG_SPEED_FAST))
	{
		_debugTimeScale = 4.0;
	}
	else if (_debugInput->GetDigitalValue(_debugInputDevices, InputEvents::ID_DEBUG_SPEED_SLOW))
	{
		_debugTimeScale = 0.25;
	}
	else
	{
		_debugTimeScale = 1.0;
	}
}

void Game::AppState::OnFrameRendering(ff::AppGlobals* globals, ff::AdvanceType type)
{
	_xamlTarget->Clear();
	_lowTarget->Clear();
}

void Game::AppState::OnFrameRendered(ff::AppGlobals* globals, ff::AdvanceType type, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	_debugStepOneFrame = false;

	ff::RectFloat lowRect(0, 0, Constants::RENDER_WIDTH, Constants::RENDER_HEIGHT);
	ff::RectFloat highRect(0, 0, Constants::RENDER_WIDTH * Constants::RENDER_SCALE, Constants::RENDER_HEIGHT * Constants::RENDER_SCALE);
	ff::RectFloat targetRect = _viewport.GetView(target);
	ff::PointFloat targetScale = targetRect.Size() / lowRect.Size();
	ff::PointFloat targetPos = targetRect.TopLeft();

	for (ff::XamlView* view : _xamlGlobals->GetRenderedViews())
	{
		view->SetViewToScreenTransform(targetPos, targetScale);
	}

	ff::RendererActive render = _render->BeginRender(_highTarget, nullptr, highRect, lowRect);
	render->PushPalette(GetPalette());
	render->DrawSprite(_lowTexture->AsSprite());
	render->DrawSprite(_xamlTexture->AsSprite());

	render = _render->BeginRender(target, nullptr, targetRect, highRect);
	render->AsRendererActive11()->PushTextureSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR);
	render->DrawSprite(_highTexture->AsSprite());
}

void Game::AppState::SaveState(ff::AppGlobals* globals)
{
	ff::Dict dict = globals->GetState(Strings::ID_APP_STATE);
	dict.Set<ff::DataValue>(Strings::ID_SYSTEM_OPTIONS, &_systemOptions, sizeof(SystemOptions));
	dict.Set<ff::DataValue>(Strings::ID_GAME_OPTIONS, &_gameOptions, sizeof(GameOptions));

	globals->SetState(Strings::ID_APP_STATE, dict);
}

ff::ProcessGlobals& Game::AppState::GetProcessGlobals()
{
	return *_processGlobals;
}

ff::AppGlobals& Game::AppState::GetAppGlobals()
{
	return *_globals;
}

ff::XamlGlobalState& Game::AppState::GetXamlGlobals()
{
	return *_xamlGlobals.get();
}

const Game::SystemOptions& Game::AppState::GetSystemOptions() const
{
	return _systemOptions;
}

const Game::GameOptions& Game::AppState::GetDefaultGameOptions() const
{
	return _gameOptions;
}

void Game::AppState::SetSystemOptions(const SystemOptions& options)
{
	_systemOptions = options;
}

void Game::AppState::SetDefaultGameOptions(const GameOptions& options)
{
	_gameOptions = options;
}

ff::IPalette* Game::AppState::GetPalette()
{
	if (!_palette)
	{
		_palette = _paletteData.Flush()->CreatePalette(_globals->GetGraph());
	}

	return _palette;
}

ff::IRenderer* Game::AppState::GetRenderer() const
{
	return _render.get();
}

ff::ITexture* Game::AppState::GetXamlTexture() const
{
	return _xamlTexture;
}

ff::IRenderTarget* Game::AppState::GetXamlTarget() const
{
	return _xamlTarget;
}

ff::IRenderDepth* Game::AppState::GetXamlDepth() const
{
	return _xamlDepth;
}

ff::ITexture* Game::AppState::GetLowTexture() const
{
	return _lowTexture;
}

ff::IRenderTarget* Game::AppState::GetLowTarget() const
{
	return _lowTarget;
}

ff::IRenderDepth* Game::AppState::GetLowDepth() const
{
	return _lowDepth;
}

bool Game::AppState::OnInitialized(ff::AppGlobals* globals)
{
	InitOptions();
	InitInput();
	InitGraphics();

	return true;
}

void Game::AppState::OnGameThreadInitialized(ff::AppGlobals* globals)
{
	std::weak_ptr<ff::State> weakThis = weak_from_this();
	_customDebugCookie = globals->GetDebugPageState()->CustomDebugEvent().Add([weakThis]()
		{
			// Can show debug UI
		});

	_xamlGlobals = std::make_shared<ff::XamlGlobalState>(_globals);
	_xamlGlobals->Startup(
		ff::String::from_static(L"9e6fb182-647d-454a-8f95-fcdf88e3c3c2"),
		ff::String::from_static(L"g8nV9oGB1fZ5EP22GHDZv3T6uCQdsGyA3YlNsw6AFmDSr4IV"),
		ff::GetThisModule().GetResources(),
		ff::GetThisModule().GetValueTable(),
		ff::String::from_static(L"ApplicationResources.xaml"));

	Game::RegisterNoesisComponents();

	_globals->SetFullScreen(_systemOptions._fullScreen);
}

void Game::AppState::OnGameThreadShutdown(ff::AppGlobals* globals)
{
	if (_xamlGlobals)
	{
		_xamlGlobals->Shutdown();
		_xamlGlobals = nullptr;
	}

	globals->GetDebugPageState()->CustomDebugEvent().Remove(_customDebugCookie);
}

std::shared_ptr<ff::State> Game::AppState::CreateInitialState(ff::AppGlobals* globals)
{
	std::shared_ptr<ff::States> states = std::make_shared<ff::States>();
	states->AddTop(shared_from_this());
	states->AddTop(std::make_shared<TitleState>(this));
	states->AddTop(_xamlGlobals);

	return states;
}

double Game::AppState::GetTimeScale(ff::AppGlobals* globals)
{
	return _debugTimeScale;
}

ff::AdvanceType Game::AppState::GetAdvanceType(ff::AppGlobals* globals)
{
	if (_debugStepOneFrame)
	{
		return ff::AdvanceType::SingleStep;
	}

	if (_debugSteppingFrames)
	{
		return ff::AdvanceType::Stopped;
	}

	return ff::AdvanceType::Running;
}

void Game::AppState::InitOptions()
{
	ff::Dict dict = GetAppGlobals().GetState(Strings::ID_APP_STATE);

	ff::ComPtr<ff::IData> systemData = dict.Get<ff::DataValue>(Strings::ID_SYSTEM_OPTIONS);
	if (systemData && systemData->GetSize() == sizeof(SystemOptions))
	{
		_systemOptions = *(SystemOptions*)systemData->GetMem();
	}

	ff::ComPtr<ff::IData> gameData = dict.Get<ff::DataValue>(Strings::ID_GAME_OPTIONS);
	if (gameData && gameData->GetSize() == sizeof(GameOptions))
	{
		_gameOptions = *(GameOptions*)gameData->GetMem();
	}
}

void Game::AppState::InitInput()
{
	_debugInput.Init(L"gameDebugControls");
	_debugInputDevices._keys.Push(_globals->GetKeysDebug());
}

void Game::AppState::InitGraphics()
{
	ff::IGraphDevice* graph = _globals->GetGraph();
	ff::PointInt lowSize(Constants::RENDER_WIDTH, Constants::RENDER_HEIGHT);
	ff::PointInt highSize = lowSize * Constants::RENDER_SCALE;

	_render = graph->CreateRenderer();

	_xamlTexture = graph->CreateTexture(lowSize, ff::TextureFormat::RGBA32);
	_xamlTarget = graph->CreateRenderTargetTexture(_xamlTexture);
	_xamlDepth = graph->CreateRenderDepth(lowSize);

	_lowTexture = graph->CreateTexture(lowSize, ff::TextureFormat::R8_UINT);
	_lowTarget = graph->CreateRenderTargetTexture(_lowTexture);
	_lowDepth = graph->CreateRenderDepth(lowSize);

	_highTexture = graph->CreateTexture(highSize, ff::TextureFormat::RGBA32);
	_highTarget = graph->CreateRenderTargetTexture(_highTexture);
	_highTarget->Clear();
}
