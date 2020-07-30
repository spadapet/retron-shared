#pragma once

namespace ff
{
	class AppGlobals;
	class IPalette;
	class IRenderDepth;
	class IRenderer;
	class IRenderTarget;
	class IResourceAccess;
	class ITexture;
	class ProcessGlobals;
	class XamlGlobalState;
}

namespace ReTron
{
	struct GameOptions;
	struct SystemOptions;

	class IAppService
	{
	public:
		// Globals
		virtual ff::ProcessGlobals& GetProcessGlobals() = 0;
		virtual ff::AppGlobals& GetAppGlobals() = 0;
		virtual ff::XamlGlobalState& GetXamlGlobals() = 0;
		virtual ff::IResourceAccess* GetResources() = 0;

		// Options
		virtual const SystemOptions& GetSystemOptions() const = 0;
		virtual const GameOptions& GetDefaultGameOptions() const = 0;
		virtual void SetSystemOptions(const SystemOptions& options) = 0;
		virtual void SetDefaultGameOptions(const GameOptions& options) = 0;

		// Rendering
		virtual void ClearLowTargets() = 0;
		virtual void RenderLowTargets(ff::IRenderTarget* target) = 0;
		virtual ff::IPalette* GetPalette() = 0;
		virtual ff::IRenderer* GetRenderer() const = 0;
		virtual ff::ITexture* GetXamlTexture() const = 0;
		virtual ff::IRenderTarget* GetXamlTarget() const = 0;
		virtual ff::IRenderDepth* GetXamlDepth() const = 0;
		virtual ff::ITexture* GetLowTexture() const = 0;
		virtual ff::IRenderTarget* GetLowTarget() const = 0;
		virtual ff::IRenderDepth* GetLowDepth() const = 0;
	};
}
