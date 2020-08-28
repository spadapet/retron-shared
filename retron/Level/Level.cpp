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

	for (const LevelObjectsSpec& objSpec : spec._objects)
	{
		for (size_t i = 0; i < objSpec._bonusWoman; i++)
		{
			CreateObject(EntityType::BonusWoman, objSpec._rect, spec._boxes);
		}

		for (size_t i = 0; i < objSpec._bonusMan; i++)
		{
			CreateObject(EntityType::BonusMan, objSpec._rect, spec._boxes);
		}

		for (size_t i = 0; i < objSpec._bonusChild; i++)
		{
			CreateObject(EntityType::BonusChild, objSpec._rect, spec._boxes);
		}

		for (size_t i = 0; i < objSpec._electrode; i++)
		{
			CreateObject(EntityType::Electrode, objSpec._rect, spec._boxes);
		}

		for (size_t i = 0; i < objSpec._hulk; i++)
		{
			CreateObject(EntityType::Hulk, objSpec._rect, spec._boxes);
		}

		for (size_t i = 0; i < objSpec._grunt; i++)
		{
			CreateObject(EntityType::Grunt, objSpec._rect, spec._boxes);
		}
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

entt::entity ReTron::Level::CreateObject(EntityType type, const ff::RectFixedInt& rect, const std::vector<ff::RectFixedInt>& boxes)
{
	const ff::RectFixedInt& hitSpec = ReTron::GetHitBoxSpec(type);
	entt::entity entity = entt::null;

	for (size_t i = 0; i < 2048; i++)
	{
		ff::PointFixedInt pos(
			(std::rand() << 2) % (int)(rect.Width() - hitSpec.Width()) - (int)hitSpec.left + (int)rect.left,
			(std::rand() << 2) % (int)(rect.Height() - hitSpec.Height()) - (int)hitSpec.top + (int)rect.top);

		ff::RectFixedInt hitBox = hitSpec + pos;
		bool goodPos = true;

		for (const ff::RectFixedInt& box : boxes)
		{
			if (hitBox.DoesIntersect(box))
			{
				goodPos = false;
				break;
			}
		}

		if (goodPos)
		{
			entity = _entityFactory.CreateEntity(type, pos);
			break;
		}
	}

	assert(entity != entt::null); // probably too many boxes in the level
	return entity;
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
			(type == EntityType::LevelBounds) ? Constants::LEVEL_BORDER_THICKNESS : Constants::LEVEL_BOX_THICKNESS);
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
