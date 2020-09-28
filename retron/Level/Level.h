#pragma once

#include "Core/GameSpec.h"
#include "Level/Collision.h"
#include "Level/Entities.h"
#include "Level/Particles.h"
#include "Level/Position.h"

namespace ff
{
	class IAnimation;
	class IRenderDepth;
	class IRenderTarget;
}

namespace ReTron
{
	class ILevelService;

	class Level
	{
	public:
		Level(ILevelService* levelService);
		~Level();

		void Advance(ff::RectFixedInt cameraRect);
		void Render(ff::IRenderTarget* target, ff::IRenderDepth* depth, ff::RectFixedInt targetRect, ff::RectFixedInt cameraRect);

	private:
		void InitResources();

		entt::entity CreateEntity(EntityType type, const ff::PointFixedInt& pos);
		entt::entity CreatePlayer(size_t indexInLevel);
		entt::entity CreatePlayerBullet(entt::entity player, ff::PointFixedInt shotPos, ff::PointFixedInt shotDir);
		entt::entity CreateBounds(const ff::RectFixedInt& rect);
		entt::entity CreateBox(const ff::RectFixedInt& rect);
		void CreateObjects(size_t count, EntityType type, const ff::RectFixedInt& rect, const std::vector<ff::RectFixedInt>& avoidRects);

		void AdvanceEntity(entt::entity entity, EntityType type);
		void AdvancePlayer(entt::entity entity);
		void AdvancePlayerBullet(entt::entity entity);
		void AdvanceGrunt(entt::entity entity);
		void AdvanceParticleEffectPositions();

		void HandleCollisions();
		void HandleBoundsCollision(entt::entity entity1, entt::entity entity2);
		void HandlePlayerBulletBoundsCollision(entt::entity entity);
		void HandleEntityCollision(entt::entity entity1, entt::entity entity2);

		void RenderParticles(ff::PixelRendererActive& render);
		void RenderEntity(entt::entity entity, EntityType type, ff::PixelRendererActive& render);
		void RenderPlayer(entt::entity entity, ff::PixelRendererActive& render);
		void RenderPlayerBullet(entt::entity entity, ff::PixelRendererActive& render);
		void RenderBonus(entt::entity entity, EntityType type, ff::PixelRendererActive& render);
		void RenderElectrode(entt::entity entity, ff::PixelRendererActive& render);
		void RenderHulk(entt::entity entity, ff::PixelRendererActive& render);
		void RenderGrunt(entt::entity entity, ff::PixelRendererActive& render);
		void RenderAnimation(entt::entity entity, ff::PixelRendererActive& render, ff::IAnimation* anim, ff::FixedInt frame);
		void RenderDebug(ff::PixelRendererActive& render);

		size_t PickGruntMoveCounter();
		ff::PointFixedInt PickMoveDestination(entt::entity entity, entt::entity destEntity, CollisionBoxType collisionType);

		template<typename... Args>
		void EnumEntities(entt::delegate<void(entt::entity, EntityType, Args&&...)> func, Args&&... args);

		ILevelService* _levelService;
		IGameService* _gameService;
		IAppService* _appService;

		const GameSpec& _gameSpec;
		const LevelSpec& _levelSpec;
		const DifficultySpec& _difficultySpec;
		size_t _frames;

		ff::TypedResource<ff::IAnimation> _playerSprite;
		ff::TypedResource<ff::IAnimation> _playerBulletSprite;
		ff::TypedResource<ff::ISprite> _playerParts[7];

		entt::registry _registry;
		entt::delegate<void(entt::entity, EntityType)> _advanceCallback;
		entt::delegate<void(entt::entity, EntityType, ff::PixelRendererActive&)> _renderCallback;
		std::forward_list<entt::scoped_connection> _connections;
		std::vector<std::pair<entt::entity, entt::entity>> _collisions;
		std::vector<entt::entity> _hits;

		Entities _entities;
		Position _position;
		Collision _collision;

		Particles _particles;
		Particles::Effect _destroyGruntParticles;
		Particles::Effect _playerBulletHitBoundsParticles;
		Particles::Effect _playerEnterParticles;
	};
}
