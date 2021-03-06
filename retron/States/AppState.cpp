#include "pch.h"
#include "App.xaml.h"
#include "Data/Data.h"
#include "Data/SavedData.h"
#include "Dict/Dict.h"
#include "Globals/AppGlobals.h"
#include "Globals/ProcessGlobals.h"
#include "Graph/Anim/Transform.h"
#include "Graph/GraphDevice.h"
#include "Graph/Render/PixelRenderer.h"
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
#include "States/DebugState.h"
#include "States/GameState.h"
#include "States/TitleState.h"
#include "States/TransitionState.h"
#include "UI/Components.h"
#include "UI/XamlGlobalState.h"
#include "UI/XamlView.h"

static const float PALETTE_CYCLES_PER_SECOND = 0.25f;

ReTron::AppState::AppState()
	: _globals(nullptr)
	, _viewport(ff::PointFloat(Constants::RENDER_WIDTH, Constants::RENDER_HEIGHT))
	, _debugSteppingFrames(false)
	, _debugStepOneFrame(false)
	, _debugTimeScale(1.0)
	, _rebuildingResources(false)
	, _renderDebug(false)
{
}

std::shared_ptr<ff::State> ReTron::AppState::Advance(ff::AppGlobals* globals)
{
	GetPalette()->Advance();

	return ff::State::Advance(globals);
}

void ReTron::AppState::AdvanceInput(ff::AppGlobals* globals)
{
	if (_gameSpec._allowDebug || DEBUG)
	{
		if (_debugInput->Advance(_debugInputDevices, ff::SECONDS_PER_ADVANCE_D))
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

			if (_debugInput->HasStartEvent(InputEvents::ID_DEBUG_RENDER_TOGGLE))
			{
				_renderDebug = !_renderDebug;
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
	else
	{
		_debugStepOneFrame = false;
		_debugSteppingFrames = false;
		_debugTimeScale = 1.0;
	}

	ff::State::AdvanceInput(globals);
}

void ReTron::AppState::OnFrameRendered(ff::AppGlobals* globals, ff::AdvanceType type, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	_debugStepOneFrame = false;

	ff::RectFixedInt targetRect = _viewport.GetView(target).ToType<ff::FixedInt>();
	ff::PointFixedInt targetScale = targetRect.Size() / Constants::RENDER_SIZE;
	ff::PointFixedInt targetPos = targetRect.TopLeft();

	for (ff::XamlView* view : _xamlGlobals->GetRenderedViews())
	{
		view->SetViewToScreenTransform(targetPos.ToType<float>(), targetScale.ToType<float>());
	}

	ff::State::OnFrameRendered(globals, type, target, depth);
}

void ReTron::AppState::SaveState(ff::AppGlobals* globals)
{
	_systemOptions._fullScreen = globals->GetTarget()->IsFullScreen();

	ff::Dict dict = globals->GetState(Strings::ID_APP_STATE);
	dict.Set<ff::DataValue>(Strings::ID_SYSTEM_OPTIONS, &_systemOptions, sizeof(SystemOptions));
	dict.Set<ff::DataValue>(Strings::ID_GAME_OPTIONS, &_gameOptions, sizeof(GameOptions));

	globals->SetState(Strings::ID_APP_STATE, dict);

	ff::State::SaveState(globals);
}

size_t ReTron::AppState::GetChildStateCount()
{
	return 2;
}

ff::State* ReTron::AppState::GetChildState(size_t index)
{
	if (index == 0)
	{
		return _debugState && _debugState->GetVisible()
			? static_cast<ff::State*>(_debugState.get())
			: static_cast<ff::State*>(_gameState.get());
	}

	return _xamlGlobals.get();

}

ff::ProcessGlobals& ReTron::AppState::GetProcessGlobals()
{
	return *ff::ProcessGlobals::Get();
}

ff::AppGlobals& ReTron::AppState::GetAppGlobals()
{
	return *_globals;
}

ff::XamlGlobalState& ReTron::AppState::GetXamlGlobals()
{
	return *_xamlGlobals.get();
}

ff::IResourceAccess* ReTron::AppState::GetResources()
{
	return ff::GetThisModule().GetResources();
}

ReTron::Audio& ReTron::AppState::GetAudio()
{
	return *_audio;
}

const ReTron::SystemOptions& ReTron::AppState::GetSystemOptions() const
{
	return _systemOptions;
}

const ReTron::GameOptions& ReTron::AppState::GetDefaultGameOptions() const
{
	return _gameOptions;
}

const ReTron::GameSpec& ReTron::AppState::GetGameSpec() const
{
	return _gameSpec;
}

void ReTron::AppState::SetSystemOptions(const SystemOptions& options)
{
	_systemOptions = options;
	ApplySystemOptions();
}

void ReTron::AppState::SetDefaultGameOptions(const GameOptions& options)
{
	_gameOptions = options;
}

ff::IPalette* ReTron::AppState::GetPalette()
{
	return GetPlayerPalette(0);
}

ff::IPalette* ReTron::AppState::GetPlayerPalette(size_t player)
{
	assertRetVal(player < _playerPalettes.size(), nullptr);

	ff::ComPtr<ff::IPalette>& playerPalette = _playerPalettes[player];
	if (!playerPalette)
	{
		playerPalette = _paletteData->CreatePalette(ff::String::format_new(L"player-%lu", player), ::PALETTE_CYCLES_PER_SECOND);
	}

	return playerPalette;
}

ff::IRenderer* ReTron::AppState::GetRenderer() const
{
	return _render.get();
}

entt::sink<void()> ReTron::AppState::GetReloadResourcesSink()
{
	return entt::sink{ _reloadResourcesEvent };
}

bool ReTron::AppState::ShouldRenderDebug() const
{
	return _renderDebug;
}

ff::IResourceAccess* ReTron::AppState::GetXamlResources()
{
	return GetResources();
}

ff::String ReTron::AppState::GetNoesisLicenseName()
{
	return GetResources()->GetString(ff::String::from_static(L"noesisLicenseName"));
}

ff::String ReTron::AppState::GetNoesisLicenseKey()
{
	return GetResources()->GetString(ff::String::from_static(L"noesisLicenseKey"));
}

ff::String ReTron::AppState::GetApplicationResourcesName()
{
	return ff::String::from_static(L"ApplicationResources.xaml");
}

ff::String ReTron::AppState::GetDefaultFont()
{
	return ff::String::from_static(L"Fonts/#Robotron 2084");
}

float ReTron::AppState::GetDefaultFontSize()
{
	return 8.0f;
}

bool ReTron::AppState::IsSRGB()
{
	return false;
}

void ReTron::AppState::RegisterNoesisComponents()
{
	ReTron::RegisterNoesisComponents();
}

void ReTron::AppState::OnApplicationResourcesLoaded(Noesis::ResourceDictionary* resources)
{
}

bool ReTron::AppState::OnInitialized(ff::AppGlobals* globals)
{
	_globals = globals;
	_render = globals->GetGraph()->CreateRenderer();
	_audio = std::make_unique<Audio>(globals);
	_debugInputDevices._keys.Push(_globals->GetKeys());

	InitOptions();
	InitResources();

	return true;
}

void ReTron::AppState::OnGameThreadInitialized(ff::AppGlobals* globals)
{
	_xamlGlobals = std::make_shared<ff::XamlGlobalState>(_globals, this);

	InitDebugState();
	InitGameState();
	ApplySystemOptions();
}

void ReTron::AppState::OnGameThreadShutdown(ff::AppGlobals* globals)
{
	_connections.clear();
	_gameState = nullptr;
	_debugState = nullptr;
	_xamlGlobals = nullptr;
}

std::shared_ptr<ff::State> ReTron::AppState::CreateInitialState(ff::AppGlobals* globals)
{
	return shared_from_this();
}

double ReTron::AppState::GetTimeScale(ff::AppGlobals* globals)
{
	return _debugTimeScale;
}

ff::AdvanceType ReTron::AppState::GetAdvanceType(ff::AppGlobals* globals)
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

void ReTron::AppState::InitOptions()
{
	ff::Dict dict = GetAppGlobals().GetState(Strings::ID_APP_STATE);

	ff::ComPtr<ff::IData> systemData = dict.Get<ff::DataValue>(Strings::ID_SYSTEM_OPTIONS);
	if (systemData && systemData->GetSize() == sizeof(SystemOptions))
	{
		_systemOptions = *(SystemOptions*)systemData->GetMem();

		if (_systemOptions._version != SystemOptions::CurrentVersion)
		{
			_systemOptions = SystemOptions();
		}
	}

	ff::ComPtr<ff::IData> gameData = dict.Get<ff::DataValue>(Strings::ID_GAME_OPTIONS);
	if (gameData && gameData->GetSize() == sizeof(GameOptions))
	{
		_gameOptions = *(GameOptions*)gameData->GetMem();

		if (_gameOptions._version != GameOptions::CurrentVersion)
		{
			_gameOptions = GameOptions();
		}
	}
}

// Must be able to be called multiple times (whenever resources are hot reloaded)
void ReTron::AppState::InitResources()
{
	_gameSpec = GameSpec::Load(GetResources());

	for (size_t i = 0; i < _playerPalettes.size(); i++)
	{
		_playerPalettes[i] = nullptr;
	}

	_paletteData.Init(L"palette-main");
	_debugInput.Init(L"gameDebugControls");
}

void ReTron::AppState::InitDebugState()
{
	_debugState = std::make_shared<ReTron::DebugState>(this);

	_connections.emplace_front(_globals->GetDebugPageState()->CustomDebugSink().connect<&ReTron::AppState::OnCustomDebug>(this));
	_connections.emplace_front(ff::GetThisModule().GetResourceRebuiltSink().connect<&ReTron::AppState::OnResourceRebuilt>(this));
	_connections.emplace_front(_debugState->RestartLevelEvent().connect<&ReTron::AppState::OnRestartLevel>(this));
	_connections.emplace_front(_debugState->RestartGameEvent().connect<&ReTron::AppState::OnRestartGame>(this));
	_connections.emplace_front(_debugState->RebuildResourcesEvent().connect<&ReTron::AppState::OnRebuildResources>(this));
}

void ReTron::AppState::OnCustomDebug()
{
	if (_debugState->GetVisible())
	{
		_debugState->Hide();
	}
	else if (_gameSpec._allowDebug || DEBUG)
	{
		_debugState->SetVisible(_gameState);
	}
}

void ReTron::AppState::OnResourceRebuilt(ff::Module*)
{
	_rebuildingResources = false;
	InitResources();
	_reloadResourcesEvent.publish();
}

void ReTron::AppState::OnRestartLevel()
{
	_debugState->Hide();

	std::shared_ptr<GameState> gameState = std::dynamic_pointer_cast<GameState>(_gameState->GetWrappedState());
	if (gameState)
	{
		gameState->RestartLevel();
	}
	else
	{
		InitGameState();
	}
}

void ReTron::AppState::OnRestartGame()
{
	_debugState->Hide();
	InitGameState();
}

void ReTron::AppState::OnRebuildResources()
{
	_debugState->Hide();

	if (!_rebuildingResources)
	{
		_rebuildingResources = true;
		ff::GetThisModule().RebuildResourcesFromSourceAsync();
	}
}

void ReTron::AppState::InitGameState()
{
	std::shared_ptr<ff::State> titleState = std::make_shared<TitleState>(this);
	std::shared_ptr<TransitionState> transitionState = std::make_shared<TransitionState>(
		this, nullptr, titleState, ff::String::from_static(L"transition-bg-1.png"), 4, 24);

	_gameState = std::make_shared<ff::StateWrapper>(transitionState);
}

void ReTron::AppState::ApplySystemOptions()
{
	_globals->SetFullScreen(_systemOptions._fullScreen);
}
