#include "pch.h"
#include "Graph/Render/PixelRenderer.h"
#include "Graph/Render/Renderer.h"
#include "Graph/Render/RendererActive.h"
#include "Level/Level.h"
#include "Services/AppService.h"
#include "Services/GameService.h"
#include "Services/LevelService.h"

ReTron::Level::Level(ILevelService* levelService)
	: _levelService(levelService)
	, _entitySystem(_registry)
	, _positionSystem(_registry)
	, _collisionSystem(_registry, _positionSystem, _entitySystem)
	, _entityFactory(levelService, _registry, _entitySystem, _positionSystem, _collisionSystem)
{
	InitLevel();
}

ReTron::Level::~Level()
{
}

void ReTron::Level::Advance(ff::RectFixedInt cameraRect)
{
	ff::RectFixedInt advanceRect = cameraRect.Deflate(Constants::RENDER_WIDTH / -2_f, Constants::RENDER_HEIGHT / -2_f);
	_advanceEntities.clear();
	_collisionSystem.HitTest(advanceRect, _advanceEntities);

	for (entt::entity entity : _advanceEntities)
	{
	}

	_collisions.clear();
	_collisionSystem.DetectCollisions(_collisions);
	_entitySystem.FlushDelete();
}

void ReTron::Level::Render(ff::IRenderTarget* target, ff::IRenderDepth* depth, ff::RectFixedInt targetRect, ff::RectFixedInt cameraRect)
{
	ff::IRenderer* render = _levelService->GetGameService()->GetAppService()->GetRenderer();
	ff::RendererActive renderActive = ff::PixelRendererActive::BeginRender(render, target, depth, targetRect, cameraRect);
	ff::PixelRendererActive renderPixel(renderActive);
	const LevelSpec& spec = _levelService->GetLevelSpec();

	for (const ff::RectFixedInt& rect : spec._bounds)
	{
		renderPixel.DrawPaletteOutlineRectangle(rect, 77, -3_f);
	}

	for (const ff::RectFixedInt& rect : spec._boxes)
	{
		renderPixel.DrawPaletteOutlineRectangle(rect, 77, 3_f);
	}

	_collisionSystem.RenderDebug(renderPixel, _collisions);
	_positionSystem.RenderDebug(renderPixel);
}

void ReTron::Level::InitLevel()
{
	const LevelSpec& spec = _levelService->GetLevelSpec();

	for (const ff::RectFixedInt& rect : spec._bounds)
	{
		_entityFactory.CreateBounds(rect);
	}

	for (const ff::RectFixedInt& rect : spec._boxes)
	{
		_entityFactory.CreateBox(rect);
	}

	for (size_t i = 0; i < _levelService->GetPlayerCount(); i++)
	{
		_entityFactory.CreatePlayer(i);
	}
}
