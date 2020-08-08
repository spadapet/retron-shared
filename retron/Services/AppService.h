#pragma once

namespace ff
{
	class AppGlobals;
	class IPalette;
	class IRenderDepth;
	class IRenderer;
	class IRenderTarget;
	class IResourceAccess;
	class IValueAccess;
	class ITexture;
	class ProcessGlobals;
	class XamlGlobalState;
}

namespace ReTron
{
	class Audio;
	struct GameOptions;
	struct SystemOptions;
	struct GameSpec;

	class IAppService
	{
	public:
		// Globals
		virtual ff::ProcessGlobals& GetProcessGlobals() = 0;
		virtual ff::AppGlobals& GetAppGlobals() = 0;
		virtual ff::XamlGlobalState& GetXamlGlobals() = 0;
		virtual ff::IResourceAccess* GetResources() = 0;
		virtual Audio& GetAudio() = 0;

		// Options
		virtual const SystemOptions& GetSystemOptions() const = 0;
		virtual const GameOptions& GetDefaultGameOptions() const = 0;
		virtual const GameSpec& GetGameSpec() const = 0;
		virtual void SetSystemOptions(const SystemOptions& options) = 0;
		virtual void SetDefaultGameOptions(const GameOptions& options) = 0;

		// Rendering
		virtual ff::IPalette* GetPalette() = 0;
		virtual ff::IPalette* GetPlayerPalette(size_t player) = 0;
		virtual ff::IRenderer* GetRenderer() const = 0;

		// Debug
		virtual ff::Event<void>& GetReloadResourcesEvent() = 0;
	};
}
