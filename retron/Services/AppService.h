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

	enum class TempTargets
	{
		None = 0x00,
		Palette1 = 0x01,
		RgbPma2 = 0x02,
	};

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
		virtual ff::IPalette* GetPalette() = 0;
		virtual ff::IRenderer* GetRenderer() const = 0;
		virtual void ClearTempTargets(TempTargets tempTargets) = 0;
		virtual void RenderTempTargets(TempTargets tempTargets, ff::IRenderTarget* target) = 0;
		virtual ff::ITexture* GetTempTexture(TempTargets tempTarget) const = 0;
		virtual ff::IRenderTarget* GetTempTarget(TempTargets tempTarget) const = 0;
		virtual ff::IRenderDepth* GetTempDepth(TempTargets tempTarget) const = 0;
	};
}
