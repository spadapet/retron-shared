#include "pch.h"
#include "Core/AppService.h"
#include "Core/RenderTargets.h"
#include "Globals/AppGlobals.h"
#include "Graph/Anim/Transform.h"
#include "Graph/GraphDevice.h"
#include "Graph/Render/PixelRenderer.h"
#include "Graph/Render/Renderer.h"
#include "Graph/Render/RendererActive.h"

ReTron::RenderTargets::RenderTargets(IAppService* appService, RenderTargetTypes targets)
	: _appService(appService)
	, _targets(targets)
	, _viewport(ff::PointFloat(Constants::RENDER_WIDTH, Constants::RENDER_HEIGHT))
{
	ff::IGraphDevice* graph = _appService->GetAppGlobals().GetGraph();
	ff::PointInt lowSize(Constants::RENDER_WIDTH, Constants::RENDER_HEIGHT);
	ff::PointInt highSize = lowSize * Constants::RENDER_SCALE;

	_depth = graph->CreateRenderDepth(lowSize);

	_textureRgbPma1 = graph->CreateTexture(lowSize, ff::TextureFormat::RGBA32);
	_targetRgbPma1 = graph->CreateRenderTargetTexture(_textureRgbPma1);

	_texturePalette1 = graph->CreateTexture(lowSize, ff::TextureFormat::R8_UINT);
	_targetPalette1 = graph->CreateRenderTargetTexture(_texturePalette1);

	_texture1080 = graph->CreateTexture(highSize, ff::TextureFormat::RGBA32);
	_target1080 = graph->CreateRenderTargetTexture(_texture1080);
}

void ReTron::RenderTargets::Clear()
{
	if (ff::HasAllFlags(_targets, RenderTargetTypes::Palette1))
	{
		GetTarget(RenderTargetTypes::Palette1)->Clear(&ff::GetColorNone());
	}

	if (ff::HasAllFlags(_targets, RenderTargetTypes::RgbPma2))
	{
		GetTarget(RenderTargetTypes::RgbPma2)->Clear(&ff::GetColorNone());
	}
}

void ReTron::RenderTargets::Render(ff::IRenderTarget* target)
{
	_target1080->Clear(&ff::GetColorNone());

	ff::RendererActive render = ff::PixelRendererActive::BeginRender(_appService->GetRenderer(), _target1080, nullptr, Constants::RENDER_RECT_HIGH, Constants::RENDER_RECT);
	if (ff::HasAllFlags(_targets, RenderTargetTypes::Palette1))
	{
		render->PushPalette(_appService->GetPalette());
		render->DrawSprite(GetTexture(RenderTargetTypes::Palette1)->AsSprite(), ff::Transform::Identity());
	}

	if (ff::HasAllFlags(_targets, RenderTargetTypes::RgbPma2))
	{
		render->PushPreMultipliedAlpha();
		render->DrawSprite(GetTexture(RenderTargetTypes::RgbPma2)->AsSprite(), ff::Transform::Identity());
	}

	ff::RectFixedInt targetRect = _viewport.GetView(target).ToType<ff::FixedInt>();
	render = ff::PixelRendererActive::BeginRender(_appService->GetRenderer(), target, nullptr, targetRect, Constants::RENDER_RECT_HIGH);
	render->AsRendererActive11()->PushTextureSampler(D3D11_FILTER_MIN_MAG_MIP_LINEAR);
	render->DrawSprite(_texture1080->AsSprite(), ff::Transform::Identity());
}

ff::ITexture* ReTron::RenderTargets::GetTexture(RenderTargetTypes target) const
{
	switch (target)
	{
	case RenderTargetTypes::Palette1:
		return _texturePalette1;

	case RenderTargetTypes::RgbPma2:
		return _textureRgbPma1;

	default:
		return nullptr;
	}
}

ff::IRenderTarget* ReTron::RenderTargets::GetTarget(RenderTargetTypes target) const
{
	switch (target)
	{
	case RenderTargetTypes::Palette1:
		return _targetPalette1;

	case RenderTargetTypes::RgbPma2:
		return _targetRgbPma1;

	default:
		return nullptr;
	}
}

ff::IRenderDepth* ReTron::RenderTargets::GetDepth(RenderTargetTypes target) const
{
	switch (target)
	{
	case RenderTargetTypes::Palette1:
	case RenderTargetTypes::RgbPma2:
		return _depth;

	default:
		return nullptr;
	}
}
