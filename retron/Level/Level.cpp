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
	_advanceCallback.connect<&Level::AdvanceEntity>(this);
	_renderCallback.connect<&Level::RenderEntity>(this);

	for (size_t i = 0; i < _levelService->GetPlayerCount(); i++)
	{
		_entityFactory.CreatePlayer(i);
	}

	const LevelSpec& spec = _levelService->GetLevelSpec();

	std::vector<ff::RectFixedInt> avoidRects;
	avoidRects.reserve(spec._rects.size());

	for (const LevelRect& levelRect : spec._rects)
	{
		switch (levelRect._type)
		{
		case LevelRect::Type::Bounds:
			_entityFactory.CreateBounds(levelRect._rect.Deflate(Constants::LEVEL_BORDER_THICKNESS, Constants::LEVEL_BORDER_THICKNESS));
			break;

		case LevelRect::Type::Box:
			_entityFactory.CreateBox(levelRect._rect);
			avoidRects.push_back(levelRect._rect);
			break;

		case LevelRect::Type::Safe:
			avoidRects.push_back(levelRect._rect);
			break;
		}
	}

	for (const LevelObjectsSpec& objSpec : spec._objects)
	{
		_entityFactory.CreateObjects(objSpec._bonusWoman, EntityType::BonusWoman, objSpec._rect, avoidRects);
		_entityFactory.CreateObjects(objSpec._bonusMan, EntityType::BonusMan, objSpec._rect, avoidRects);
		_entityFactory.CreateObjects(objSpec._bonusChild, EntityType::BonusChild, objSpec._rect, avoidRects);
		_entityFactory.CreateObjects(objSpec._electrode, EntityType::Electrode, objSpec._rect, avoidRects);
		_entityFactory.CreateObjects(objSpec._hulk, EntityType::Hulk, objSpec._rect, avoidRects);
		_entityFactory.CreateObjects(objSpec._grunt, EntityType::Grunt, objSpec._rect, avoidRects);
	}
}

ReTron::Level::~Level()
{
}

void ReTron::Level::Advance(ff::RectFixedInt cameraRect)
{
	EnumEntities(_advanceCallback);
	AdvanceGrunts();
	AdvanceCollisions();

	_entitySystem.FlushDelete();
}

void ReTron::Level::Render(ff::IRenderTarget* target, ff::IRenderDepth* depth, ff::RectFixedInt targetRect, ff::RectFixedInt cameraRect)
{
	ff::RendererActive renderActive = ff::PixelRendererActive::BeginRender(_levelService->GetGameService()->GetAppService()->GetRenderer(), target, depth, targetRect, cameraRect);
	EnumEntities<ff::PixelRendererActive&>(_renderCallback, ff::PixelRendererActive(renderActive));

	if (_levelService->GetGameService()->GetAppService()->ShouldRenderDebug())
	{
		_collisionSystem.RenderDebug(ff::PixelRendererActive(renderActive));
		_positionSystem.RenderDebug(ff::PixelRendererActive(renderActive));
	}
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
	const ff::FixedInt dirScale = 1.5_f;

	ff::RectFixedInt dirPress(
		dirScale * inputEvents->GetDigitalValue(inputDevices, InputEvents::ID_LEFT),
		dirScale * inputEvents->GetDigitalValue(inputDevices, InputEvents::ID_UP),
		dirScale * inputEvents->GetDigitalValue(inputDevices, InputEvents::ID_RIGHT),
		dirScale * inputEvents->GetDigitalValue(inputDevices, InputEvents::ID_DOWN));

	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	pos = pos.Offset(dirPress.right - dirPress.left, dirPress.bottom - dirPress.top);
	_positionSystem.SetPosition(entity, pos);
}

void ReTron::Level::AdvanceGrunts()
{
}

void ReTron::Level::AdvanceCollisions()
{
	for (size_t i = _collisionSystem.DetectCollisions(); i != 0; i--)
	{
		auto [entity1, entity2] = _collisionSystem.GetCollision(i - 1);
	}
}

void ReTron::Level::RenderEntity(entt::entity entity, EntityType type, ff::PixelRendererActive& render)
{
	switch (type)
	{
	case EntityType::Player:
		RenderPlayer(entity, render);
		break;

	case EntityType::BonusWoman:
	case EntityType::BonusMan:
	case EntityType::BonusChild:
		RenderBonus(entity, type, render);
		break;

	case EntityType::Electrode:
		RenderElectrode(entity, render);
		break;

	case EntityType::Hulk:
		RenderHulk(entity, render);
		break;

	case EntityType::Grunt:
		RenderGrunt(entity, render);
		break;

	case EntityType::LevelBounds:
	case EntityType::LevelBox:
		render.DrawPaletteOutlineRectangle(_collisionSystem.GetHitBox(entity), Colors::LEVEL_BORDER,
			(type == EntityType::LevelBounds) ? -Constants::LEVEL_BORDER_THICKNESS : Constants::LEVEL_BOX_THICKNESS);
		break;
	}

}

void ReTron::Level::RenderPlayer(entt::entity entity, ff::PixelRendererActive& render)
{
	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	render.DrawPaletteFilledCircle(pos.Offset(0, -4), 4, 236);
}

void ReTron::Level::RenderBonus(entt::entity entity, EntityType type, ff::PixelRendererActive& render)
{
	int color = 251;
	switch (type)
	{
	case EntityType::BonusWoman: color = 238; break;
	case EntityType::BonusMan: color = 246; break;
	}

	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	render.DrawPaletteFilledRectangle(_collisionSystem.GetHitBox(entity), color);
}

void ReTron::Level::RenderElectrode(entt::entity entity, ff::PixelRendererActive& render)
{
	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	render.DrawPaletteFilledRectangle(_collisionSystem.GetHitBox(entity), 45);
}

void ReTron::Level::RenderHulk(entt::entity entity, ff::PixelRendererActive& render)
{
	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	render.DrawPaletteFilledRectangle(_collisionSystem.GetHitBox(entity), 235);
}

void ReTron::Level::RenderGrunt(entt::entity entity, ff::PixelRendererActive& render)
{
	ff::PointFixedInt pos = _positionSystem.GetPosition(entity);
	render.DrawPaletteFilledRectangle(_collisionSystem.GetHitBox(entity), 248);
}
