#include "pch.h"
#include "Core/AppService.h"
#include "Core/GameService.h"
#include "Core/LevelService.h"
#include "Core/Player.h"
#include "Graph/Render/PixelRenderer.h"
#include "Graph/Render/Renderer.h"
#include "Graph/Render/RendererActive.h"
#include "Input/InputMapping.h"
#include "Level/Level.h"

ReTron::Level::Level(ILevelService* levelService)
	: _levelService(levelService)
	, _entitySystem(_registry)
	, _positionSystem(_registry)
	, _collisionSystem(_registry, _positionSystem, _entitySystem)
	, _entityFactory(levelService, _registry, _entitySystem, _positionSystem, _collisionSystem)
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

ReTron::Level::~Level()
{
}

void ReTron::Level::Advance(ff::RectFixedInt cameraRect)
{
	EnumEntities(AdvanceEntityCallback());
	AdvanceGrunts();
	AdvanceCollisions();

	_entitySystem.FlushDelete();
}

void ReTron::Level::Render(ff::IRenderTarget* target, ff::IRenderDepth* depth, ff::RectFixedInt targetRect, ff::RectFixedInt cameraRect)
{
	ff::IRenderer* render = _levelService->GetGameService()->GetAppService()->GetRenderer();
	ff::RendererActive renderActive = ff::PixelRendererActive::BeginRender(render, target, depth, targetRect, cameraRect);
	ff::PixelRendererActive renderPixel(renderActive);

	EnumEntities<ff::PixelRendererActive&>(RenderEntityCallback(), renderPixel);

	if (_levelService->GetGameService()->GetAppService()->ShouldRenderDebug())
	{
		_collisionSystem.RenderDebug(renderPixel);
		_positionSystem.RenderDebug(renderPixel);
	}
}

entt::delegate<void(entt::entity, ReTron::EntityType)> ReTron::Level::AdvanceEntityCallback()
{
	entt::delegate<void(entt::entity, EntityType)> callback{};
	callback.connect<&Level::AdvanceEntity>(this);
	return callback;
}

void ReTron::Level::AdvanceEntity(entt::entity entity, EntityType type)
{
	switch (type)
	{
	case EntityType::Player:
		AdvancePlayer(entity);
		break;
	}
}

void ReTron::Level::AdvancePlayer(entt::entity entity)
{
	size_t index = _entitySystem.GetPlayer(entity);
	Player& player = _levelService->GetPlayer(index);

	const ff::InputDevices& inputDevices = _levelService->GetGameService()->GetInputDevices(player);
	const ff::IInputEvents* inputEvents = _levelService->GetGameService()->GetInputEvents(player);

	ff::RectFixedInt dirPress(
		inputEvents->GetAnalogValue(inputDevices, InputEvents::ID_LEFT),
		inputEvents->GetAnalogValue(inputDevices, InputEvents::ID_UP),
		inputEvents->GetAnalogValue(inputDevices, InputEvents::ID_RIGHT),
		inputEvents->GetAnalogValue(inputDevices, InputEvents::ID_DOWN));

	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	pos = pos.Offset(dirPress.right - dirPress.left, dirPress.bottom - dirPress.top);
	_positionSystem.SetPosition(entity, pos);
}

void ReTron::Level::AdvanceGrunts()
{
}

void ReTron::Level::AdvanceCollisions()
{
	size_t count = _collisionSystem.DetectCollisions();

	for (size_t i = 0; i < count; i++)
	{
		std::pair<entt::entity, entt::entity> pair = _collisionSystem.GetCollision(i);
	}
}

entt::delegate<void(entt::entity, ReTron::EntityType, ff::PixelRendererActive&)> ReTron::Level::RenderEntityCallback()
{
	entt::delegate<void(entt::entity, EntityType, ff::PixelRendererActive&)> callback{};
	callback.connect<&Level::RenderEntity>(this);
	return callback;
}

void ReTron::Level::RenderEntity(entt::entity entity, EntityType type, ff::PixelRendererActive& render)
{
	switch (type)
	{
	case EntityType::Player:
		RenderPlayer(entity, render);
		break;

	case EntityType::LevelBounds:
	case EntityType::LevelBox:
		render.DrawPaletteOutlineRectangle(_collisionSystem.GetHitBox(entity), Colors::LEVEL_BORDER,
			(type == EntityType::LevelBounds) ? Constants::LEVEL_BORDER_THICKNESS : Constants::LEVEL_BOX_THICKNESS);
		break;
	}

}

void ReTron::Level::RenderPlayer(entt::entity entity, ff::PixelRendererActive& render)
{
	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	render.DrawPaletteFilledCircle(pos.Offset(0, -4), 4, Colors::LEVEL_BORDER);
}
