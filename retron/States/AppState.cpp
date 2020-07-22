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
#include "States/TitleState.h"
#include "Value/Values.h"
#include "UI/Components.h"
#include "UI/XamlGlobalState.h"
#include "UI/XamlView.h"

ReTron::AppState::AppState()
	: _processGlobals(nullptr)
	, _globals(nullptr)
	, _viewport(ff::PointFloat(Constants::RENDER_WIDTH, Constants::RENDER_HEIGHT))
	, _debugSteppingFrames(false)
	, _debugStepOneFrame(false)
	, _debugTimeScale(1.0)
	, _customDebugCookie(nullptr)
{
}

std::shared_ptr<ff::State> ReTron::AppState::Advance(ff::AppGlobals* globals)
{
	GetPalette()->Advance();

	return nullptr;
}

void ReTron::AppState::AdvanceDebugInput(ff::AppGlobals* globals)
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
}

void ReTron::AppState::SaveState(ff::AppGlobals* globals)
{
	_systemOptions._fullScreen = globals->GetTarget()->IsFullScreen();

	ff::Dict dict = globals->GetState(Strings::ID_APP_STATE);
	dict.Set<ff::DataValue>(Strings::ID_SYSTEM_OPTIONS, &_systemOptions, sizeof(SystemOptions));
	dict.Set<ff::DataValue>(Strings::ID_GAME_OPTIONS, &_gameOptions, sizeof(GameOptions));

	globals->SetState(Strings::ID_APP_STATE, dict);
}

ff::ProcessGlobals& ReTron::AppState::GetProcessGlobals()
{
	return *_processGlobals;
}

ff::AppGlobals& ReTron::AppState::GetAppGlobals()
{
	return *_globals;
}

ff::XamlGlobalState& ReTron::AppState::GetXamlGlobals()
{
	return *_xamlGlobals.get();
}

const ReTron::SystemOptions& ReTron::AppState::GetSystemOptions() const
{
	return _systemOptions;
}

const ReTron::GameOptions& ReTron::AppState::GetDefaultGameOptions() const
{
	return _gameOptions;
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

void ReTron::AppState::ClearLowTargets()
{
	_xamlTarget->Clear(&ff::GetColorNone());
	_lowTarget->Clear(&ff::GetColorNone());
	_highTarget->Clear(&ff::GetColorBlack());
}

void ReTron::AppState::RenderLowTargets(ff::IRenderTarget* target)
{
	ff::RendererActive render = ff::PixelRendererActive::BeginRender(_render.get(), _highTarget, nullptr, Constants::RENDER_RECT_HIGH, Constants::RENDER_RECT);
	render->PushPalette(GetPalette());
	render->DrawSprite(_lowTexture->AsSprite(), ff::Transform::Identity());
	render->DrawSprite(_xamlTexture->AsSprite(), ff::Transform::Identity());

	ff::RectFixedInt targetRect = _viewport.GetView(target).ToType<ff::FixedInt>();
	render = ff::PixelRendererActive::BeginRender(_render.get(), target, nullptr, targetRect, Constants::RENDER_RECT_HIGH);
	render->AsRendererActive11()->PushTextureSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR);
	render->DrawSprite(_highTexture->AsSprite(), ff::Transform::Identity());
}

ff::IPalette* ReTron::AppState::GetPalette()
{
	if (!_palette)
	{
		_palette = _paletteData->CreatePalette(0.25f);
	}

	return _palette;
}

ff::IRenderer* ReTron::AppState::GetRenderer() const
{
	return _render.get();
}

ff::ITexture* ReTron::AppState::GetXamlTexture() const
{
	return _xamlTexture;
}

ff::IRenderTarget* ReTron::AppState::GetXamlTarget() const
{
	return _xamlTarget;
}

ff::IRenderDepth* ReTron::AppState::GetXamlDepth() const
{
	return _xamlDepth;
}

ff::ITexture* ReTron::AppState::GetLowTexture() const
{
	return _lowTexture;
}

ff::IRenderTarget* ReTron::AppState::GetLowTarget() const
{
	return _lowTarget;
}

ff::IRenderDepth* ReTron::AppState::GetLowDepth() const
{
	return _lowDepth;
}

ff::IResourceAccess* ReTron::AppState::GetXamlResources()
{
	return ff::GetThisModule().GetResources();
}

ff::String ReTron::AppState::GetNoesisLicenseName()
{
	return ff::GetThisModule().GetString(ff::String::from_static(L"noesisLicenseName"));
}

ff::String ReTron::AppState::GetNoesisLicenseKey()
{
	return ff::GetThisModule().GetString(ff::String::from_static(L"noesisLicenseKey"));
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
	_processGlobals = ff::ProcessGlobals::Get();
	_globals = globals;

	InitOptions();
	InitResources();
	InitGraphics();

	return true;
}

void ReTron::AppState::OnGameThreadInitialized(ff::AppGlobals* globals)
{
	std::weak_ptr<ff::State> weakThis = weak_from_this();
	_customDebugCookie = globals->GetDebugPageState()->CustomDebugEvent().Add([weakThis]()
		{
			// Can show debug UI
		});

	_xamlGlobals = std::make_shared<ff::XamlGlobalState>(_globals);
	_xamlGlobals->Startup(this);

	ApplySystemOptions();
}

void ReTron::AppState::OnGameThreadShutdown(ff::AppGlobals* globals)
{
	if (_xamlGlobals)
	{
		_xamlGlobals->Shutdown();
		_xamlGlobals = nullptr;
	}

	globals->GetDebugPageState()->CustomDebugEvent().Remove(_customDebugCookie);
}

std::shared_ptr<ff::State> ReTron::AppState::CreateInitialState(ff::AppGlobals* globals)
{
	std::shared_ptr<ff::States> states = std::make_shared<ff::States>();
	states->AddTop(shared_from_this());
	states->AddTop(std::make_shared<TitleState>(this));
	states->AddTop(_xamlGlobals);

	return states;
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

void ReTron::AppState::InitResources()
{
	_paletteData.Init(L"palette");

	_debugInput.Init(L"gameDebugControls");
	_debugInputDevices._keys.Push(_globals->GetKeysDebug());
}

void ReTron::AppState::InitGraphics()
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
}

void ReTron::AppState::ApplySystemOptions()
{
	_globals->SetFullScreen(_systemOptions._fullScreen);
}
