#include "pch.h"
#include "Globals/AppGlobals.h"
#include "Graph/Anim/Transform.h"
#include "Graph/GraphDevice.h"
#include "Graph/Render/Renderer.h"
#include "Graph/Render/RendererActive.h"
#include "Graph/Render/PixelRenderer.h"
#include "Graph/RenderTarget/RenderDepth.h"
#include "Graph/RenderTarget/RenderTarget.h"
#include "Graph/Texture/Texture.h"
#include "Services/AppService.h"
#include "States/TransitionState.h"

ReTron::TransitionState::TransitionState(IAppService* appService, std::shared_ptr<ff::State> oldState, std::shared_ptr<ff::State> newState, ff::StringRef imageResource, size_t speed, size_t verticalPixelStop)
	: _appService(appService)
	, _oldState(oldState)
	, _newState(newState)
	, _image(appService->GetResources(), imageResource)
	, _counter(0)
	, _speed(speed)
	, _offsetStop(verticalPixelStop)
{
	assert(_newState);

	ff::IGraphDevice* graph = appService->GetAppGlobals().GetGraph();
	ff::PointInt size = Constants::RENDER_SIZE.ToType<int>();

	_texture = graph->CreateTexture(size, ff::TextureFormat::RGBA32);
	_texture2 = graph->CreateTexture(size, ff::TextureFormat::RGBA32);
	_target = graph->CreateRenderTargetTexture(_texture);
	_target2 = graph->CreateRenderTargetTexture(_texture2);
	_depth = graph->CreateRenderDepth(size);
}

std::shared_ptr<ff::State> ReTron::TransitionState::Advance(ff::AppGlobals* globals)
{
	if (_oldState)
	{
		_counter += 40;

		if (_counter > Constants::RENDER_WIDTH)
		{
			_oldState = nullptr;
			_counter = 0;
		}
	}
	else
	{
		_counter += _speed;

		if (_counter >= Constants::RENDER_HEIGHT - _offsetStop)
		{
			_newState->Advance(globals);
			return _newState;
		}
	}

	return nullptr;
}

void ReTron::TransitionState::Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth)
{
	_target->Clear(&ff::GetColorBlack());
	_target2->Clear(&ff::GetColorBlack());

	ff::FixedInt halfHeight = Constants::RENDER_HEIGHT / 2;

	if (_oldState)
	{
		_oldState->Render(globals, _target, _depth);

		ff::RectFixedInt rect = Constants::RENDER_RECT.RightEdge();
		rect.left -= std::min(_counter, Constants::RENDER_WIDTH);

		ff::RendererActive render = ff::PixelRendererActive::BeginRender(_appService->GetRenderer(), _target, nullptr, rect, rect);
		if (render)
		{
			render->DrawFilledRectangle(rect.ToType<float>(), ff::GetColorBlack());
		}
	}
	else if (_counter <= halfHeight)
	{
		ff::FixedInt offset = halfHeight - std::min(halfHeight, _counter);
		ff::RectFixedInt rect(offset, offset, Constants::RENDER_WIDTH - offset, Constants::RENDER_HEIGHT - offset);
		ff::RendererActive render = ff::PixelRendererActive::BeginRender(_appService->GetRenderer(), _target, nullptr, rect, rect);
		if (render)
		{
			render->PushPalette(_appService->GetPalette());
			render->DrawSprite(_image->AsSprite(), ff::Transform::Identity());
		}
	}
	else
	{
		// Draw gradient image
		{
			ff::RendererActive render = _appService->GetRenderer()->BeginRender(_target, nullptr, Constants::RENDER_RECT.ToType<float>(), Constants::RENDER_RECT.ToType<float>());
			render->PushPalette(_appService->GetPalette());
			render->DrawSprite(_image->AsSprite(), ff::Transform::Identity());
		}

		_newState->Render(globals, _target2, _depth);

		// Draw new state
		{
			ff::FixedInt offset = halfHeight - std::min(halfHeight, _counter - halfHeight);
			ff::RectFixedInt rect(offset, offset, Constants::RENDER_WIDTH - offset, Constants::RENDER_HEIGHT - offset);
			ff::RendererActive render = ff::PixelRendererActive::BeginRender(_appService->GetRenderer(), _target, nullptr, rect, rect);
			if (render)
			{
				render->DrawSprite(_texture2->AsSprite(), ff::Transform::Identity());
			}
		}
	}

	_appService->ClearTempTargets(TempTargets::RgbPma2);
	{
		ff::RendererActive render = _appService->GetRenderer()->BeginRender(
			_appService->GetTempTarget(TempTargets::RgbPma2),
			_appService->GetTempDepth(TempTargets::RgbPma2),
			Constants::RENDER_RECT.ToType<float>(),
			Constants::RENDER_RECT.ToType<float>());
		render->DrawSprite(_texture->AsSprite(), ff::Transform::Identity());
	}
	_appService->RenderTempTargets(TempTargets::RgbPma2, target);
}
