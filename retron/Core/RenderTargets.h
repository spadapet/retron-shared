#pragma once

#include "Graph/RenderTarget/Viewport.h"
#include "Graph/RenderTarget/RenderDepth.h"
#include "Graph/RenderTarget/RenderTarget.h"
#include "Graph/Texture/Texture.h"

namespace ff
{
	class IRenderTarget;
}

namespace ReTron
{
	class IAppService;

	enum class RenderTargetTypes
	{
		None = 0x00,
		Palette1 = 0x01,
		RgbPma2 = 0x02,
	};

	class RenderTargets
	{
	public:
		RenderTargets(IAppService* appService, RenderTargetTypes targets);

		void Clear();
		void Render(ff::IRenderTarget* target);
		ff::ITexture* GetTexture(RenderTargetTypes target) const;
		ff::IRenderTarget* GetTarget(RenderTargetTypes target) const;
		ff::IRenderDepth* GetDepth(RenderTargetTypes target) const;

	private:
		IAppService* _appService;
		RenderTargetTypes _targets;
		ff::Viewport _viewport;
		ff::ComPtr<ff::IRenderDepth> _depth;
		ff::ComPtr<ff::ITexture> _textureRgbPma1;
		ff::ComPtr<ff::IRenderTarget> _targetRgbPma1;
		ff::ComPtr<ff::ITexture> _texturePalette1;
		ff::ComPtr<ff::IRenderTarget> _targetPalette1;
		ff::ComPtr<ff::ITexture> _texture1080;
		ff::ComPtr<ff::IRenderTarget> _target1080;
	};
}
