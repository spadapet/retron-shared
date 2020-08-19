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
	, _positionComponents(_registry)
	, _collisionSystem(_registry, _positionComponents)
	, _entityManager(_registry)
{
	InitLevel();
}

ReTron::Level::~Level()
{
}

void ReTron::Level::Advance(ff::RectFixedInt cameraRect)
{
	_entityManager.FlushDelete();
}

void ReTron::Level::Render(ff::IRenderTarget* target, ff::IRenderDepth* depth, ff::RectFixedInt targetRect, ff::RectFixedInt cameraRect)
{
	ff::IRenderer* render = _levelService->GetGameService()->GetAppService()->GetRenderer();
	ff::RendererActive renderActive = ff::PixelRendererActive::BeginRender(render, target, depth, targetRect, cameraRect);
	ff::PixelRendererActive renderPixel(renderActive);
	renderPixel.DrawPaletteOutlineRectangle(cameraRect, 77, 3_f);
}

ff::PointFixedInt ReTron::Level::GetPlayerPosition(size_t index)
{
	return ff::PointFixedInt();
}

void ReTron::Level::InitLevel()
{
}
