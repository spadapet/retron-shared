#pragma once

#include "Core/AppService.h"
#include "Core/Audio.h"
#include "Core/Options.h"
#include "Core/GameSpec.h"
#include "Dict/ValueTable.h"
#include "Globals/AppGlobalsHelper.h"
#include "Graph/RenderTarget/Viewport.h"
#include "Input/InputMapping.h"
#include "Resource/Resources.h"
#include "Resource/ResourceValue.h"
#include "State/StateWrapper.h"
#include "UI/XamlGlobalHelper.h"

namespace ff
{
	class IPalette;
	class IPaletteData;
}

namespace ReTron
{
	class DebugState;

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
		virtual void AdvanceInput(ff::AppGlobals* globals) override;
		virtual void OnFrameRendered(ff::AppGlobals* globals, ff::AdvanceType type, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;
		virtual void SaveState(ff::AppGlobals* globals) override;
		virtual size_t GetChildStateCount() override;
		virtual ff::State* GetChildState(size_t index) override;

		// IAppService
		virtual ff::ProcessGlobals& GetProcessGlobals() override;
		virtual ff::AppGlobals& GetAppGlobals() override;
		virtual ff::XamlGlobalState& GetXamlGlobals() override;
		virtual ff::IResourceAccess* GetResources() override;
		virtual Audio& GetAudio() override;
		virtual const SystemOptions& GetSystemOptions() const override;
		virtual const GameOptions& GetDefaultGameOptions() const override;
		virtual const GameSpec& GetGameSpec() const override;
		virtual void SetSystemOptions(const SystemOptions& options) override;
		virtual void SetDefaultGameOptions(const GameOptions& options) override;
		virtual ff::IPalette* GetPalette() override;
		virtual ff::IPalette* GetPlayerPalette(size_t player) override;
		virtual ff::IRenderer* GetRenderer() const override;
		virtual entt::sink<void()> GetReloadResourcesSink() override;
		virtual bool ShouldRenderDebug() const override;

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
		void InitDebugState();
		void InitGameState();
		void ApplySystemOptions();

		void OnCustomDebug();
		void OnResourceRebuilt(ff::Module*);
		void OnRestartLevel();
		void OnRestartGame();
		void OnRebuildResources();

		// Globals
		ff::AppGlobals* _globals;
		std::shared_ptr<ff::StateWrapper> _gameState;
		std::shared_ptr<ff::XamlGlobalState> _xamlGlobals;
		SystemOptions _systemOptions;
		GameOptions _gameOptions;
		GameSpec _gameSpec;

		// Rendering
		std::unique_ptr<ff::IRenderer> _render;
		std::array<ff::ComPtr<ff::IPalette>, Constants::MAX_PLAYERS> _playerPalettes;
		ff::TypedResource<ff::IPaletteData> _paletteData;
		ff::Viewport _viewport;

		// Audio
		std::unique_ptr<Audio> _audio;

		// Debugging
		std::shared_ptr<ReTron::DebugState> _debugState;
		ff::TypedResource<ff::IInputMapping> _debugInput;
		ff::InputDevices _debugInputDevices;
		std::forward_list<entt::scoped_connection> _connections;
		entt::sigh<void()> _reloadResourcesEvent;
		double _debugTimeScale;
		bool _debugSteppingFrames;
		bool _debugStepOneFrame;
		bool _rebuildingResources;
		bool _renderDebug;
	};
}
