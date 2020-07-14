#pragma once

#include "Core/Options.h"
#include "Dict/ValueTable.h"
#include "Globals/AppGlobalsHelper.h"
#include "Graph/RenderTarget/Viewport.h"
#include "Input/InputMapping.h"
#include "Resource/Resources.h"
#include "Resource/ResourceValue.h"
#include "Services/AppService.h"
#include "State/State.h"
#include "UI/XamlGlobalHelper.h"

namespace ff
{
	class IPalette;
	class IPaletteData;
}

namespace ReTron
{
	class AppState
		: public ff::State
		, public ff::IAppGlobalsHelper
		, public ff::IXamlGlobalHelper
		, public ReTron::IAppService
	{
	public:
		AppState();

		// State
		virtual std::shared_ptr<State> Advance(ff::AppGlobals* globals) override;
		virtual void AdvanceDebugInput(ff::AppGlobals* globals) override;
		virtual void OnFrameRendering(ff::AppGlobals* globals, ff::AdvanceType type) override;
		virtual void OnFrameRendered(ff::AppGlobals* globals, ff::AdvanceType type, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;
		virtual void SaveState(ff::AppGlobals* globals) override;

		// IAppService
		virtual ff::ProcessGlobals& GetProcessGlobals() override;
		virtual ff::AppGlobals& GetAppGlobals() override;
		virtual ff::XamlGlobalState& GetXamlGlobals() override;
		virtual const SystemOptions& GetSystemOptions() const override;
		virtual const GameOptions& GetDefaultGameOptions() const override;
		virtual void SetSystemOptions(const SystemOptions& options) override;
		virtual void SetDefaultGameOptions(const GameOptions& options) override;
		virtual ff::IPalette* GetPalette() override;
		virtual ff::IRenderer* GetRenderer() const override;
		virtual ff::ITexture* GetXamlTexture() const override;
		virtual ff::IRenderTarget* GetXamlTarget() const override;
		virtual ff::IRenderDepth* GetXamlDepth() const override;
		virtual ff::ITexture* GetLowTexture() const override;
		virtual ff::IRenderTarget* GetLowTarget() const override;
		virtual ff::IRenderDepth* GetLowDepth() const override;

		// IXamlGlobalHelper
		virtual ff::IResourceAccess* GetXamlResources() override;
		virtual ff::String GetNoesisLicenseName() override;
		virtual ff::String GetNoesisLicenseKey() override;
		virtual ff::String GetApplicationResourcesName() override;
		virtual ff::String GetDefaultFont() override;
		virtual float GetDefaultFontSize() override;
		virtual bool IsSRGB() override;
		virtual void RegisterNoesisComponents() override;
		virtual void OnApplicationResourcesLoaded(Noesis::ResourceDictionary* resources) override;

		// IAppGlobalsHelper
		virtual bool OnInitialized(ff::AppGlobals* globals) override;
		virtual void OnGameThreadInitialized(ff::AppGlobals* globals) override;
		virtual void OnGameThreadShutdown(ff::AppGlobals* globals)  override;
		virtual std::shared_ptr<State> CreateInitialState(ff::AppGlobals* globals) override;
		virtual double GetTimeScale(ff::AppGlobals* globals) override;
		virtual ff::AdvanceType GetAdvanceType(ff::AppGlobals* globals) override;

	private:
		void InitOptions();
		void InitResources();
		void InitGraphics();
		void ApplySystemOptions();

		// Globals
		ff::ProcessGlobals* _processGlobals;
		ff::AppGlobals* _globals;
		std::shared_ptr<ff::XamlGlobalState> _xamlGlobals;
		SystemOptions _systemOptions;
		GameOptions _gameOptions;

		// Rendering
		std::unique_ptr<ff::IRenderer> _render;
		ff::ComPtr<ff::ITexture> _xamlTexture;
		ff::ComPtr<ff::IRenderTarget> _xamlTarget;
		ff::ComPtr<ff::IRenderDepth> _xamlDepth;
		ff::ComPtr<ff::ITexture> _lowTexture;
		ff::ComPtr<ff::IRenderTarget> _lowTarget;
		ff::ComPtr<ff::IRenderDepth> _lowDepth;
		ff::ComPtr<ff::ITexture> _highTexture;
		ff::ComPtr<ff::IRenderTarget> _highTarget;
		ff::ComPtr<ff::IPalette> _palette;
		ff::TypedResource<ff::IPaletteData> _paletteData;
		ff::Viewport _viewport;

		// Debugging
		ff::TypedResource<ff::IInputMapping> _debugInput;
		ff::InputDevices _debugInputDevices;
		ff::EventCookie _customDebugCookie;
		double _debugTimeScale;
		bool _debugSteppingFrames;
		bool _debugStepOneFrame;
	};
}
