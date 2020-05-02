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

namespace Game
{
	class AppState
		: public ff::State
		, public ff::IAppGlobalsHelper
		, public ff::IResourceAccess
		, public ff::IValueAccess
		, public Game::IAppService
	{
	public:
		AppState(ff::ProcessGlobals* processGlobals, ff::AppGlobals* globals);

		// State
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
		virtual ff::IRenderer* GetRenderer() const override;
		virtual ff::ITexture* GetXamlTexture() const override;
		virtual ff::IRenderTarget* GetXamlTarget() const override;
		virtual ff::IRenderDepth* GetXamlDepth() const override;
		virtual ff::ITexture* GetLowTexture() const override;
		virtual ff::IRenderTarget* GetLowTarget() const override;
		virtual ff::IRenderDepth* GetLowDepth() const override;

		// IAppGlobalsHelper
		virtual bool OnInitialized(ff::AppGlobals* globals) override;
		virtual void OnGameThreadInitialized(ff::AppGlobals* globals) override;
		virtual void OnGameThreadShutdown(ff::AppGlobals* globals)  override;
		virtual std::shared_ptr<State> CreateInitialState(ff::AppGlobals* globals) override;
		virtual double GetTimeScale(ff::AppGlobals* globals) override;
		virtual ff::AdvanceType GetAdvanceType(ff::AppGlobals* globals) override;

		// IResourceAccess
		virtual ff::AutoResourceValue GetResource(ff::StringRef name) override;
		virtual ff::Vector<ff::String> GetResourceNames() const override;

		// IValueAccess
		virtual ff::ValuePtr GetValue(ff::StringRef name) const override;
		virtual ff::String GetString(ff::StringRef name) const override;

	private:
		void InitOptions();
		void InitInputDevices();
		void InitGraphics();

		ff::ProcessGlobals* _processGlobals;
		ff::AppGlobals* _globals;
		std::shared_ptr<ff::XamlGlobalState> _xamlGlobals;
		SystemOptions _systemOptions;
		GameOptions _gameOptions;
		ff::ComPtr<ff::IResources> _levelResources;
		ff::Event<ff::hash_t> _appEvents;

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
		ff::ComPtr<ff::IRenderDepth> _highDepth;
		ff::Viewport _viewport;

		// Debugging
		ff::TypedResource<ff::IInputMapping> _debugInputResource;
		ff::InputDevices _debugInputDevices;
		ff::EventCookie _customDebugCookie;
		double _debugTimeScale;
		bool _debugSteppingFrames;
		bool _debugStepOneFrame;
	};
}
