#pragma once

#include "Graph/Anim/Transform.h"
#include "Windows/Handles.h"

namespace ff
{
	class Dict;
	class IAnimation;
	class PixelRendererActive;
	class Value;
}

namespace ReTron
{
	class Particles
	{
	public:
		Particles();

		ff::AtScope AdvanceAsync();
		void Render(ff::PixelRendererActive& render, uint8_t type = 0);

		bool IsEffectActive(int effectId) const;
		void SetEffectPosition(int effectId, ff::PointFixedInt pos);

		struct EffectOptions
		{
			std::pair<ff::FixedInt, ff::FixedInt> _angle = std::make_pair(0_f, 360_f);
			ff::PointFixedInt _scale = ff::PointFixedInt::Ones();
			ff::FixedInt _rotate = 0_f;
			uint8_t _type = 0;
			bool _reverse = false;
		};

	private:
		class Spec
		{
		public:
			Spec() = default;
			Spec(Spec&&) = default;
			Spec(const Spec&) = default;
			Spec(const ff::Dict& dict);

			Spec& operator=(Spec&&) = default;
			Spec& operator=(const Spec&) = default;

			void Add(Particles& particles, ff::PointFixedInt pos, int effectId, const EffectOptions& options) const;

		private:
			std::pair<int, int> _count;
			std::pair<int, int> _delay;
			std::pair<int, int> _life;

			std::pair<ff::FixedInt, ff::FixedInt> _dist;
			std::pair<ff::FixedInt, ff::FixedInt> _distVel;
			std::pair<ff::FixedInt, ff::FixedInt> _size;
			std::pair<ff::FixedInt, ff::FixedInt> _angle;
			std::pair<ff::FixedInt, ff::FixedInt> _angleVel;
			std::pair<ff::FixedInt, ff::FixedInt> _spin;
			std::pair<ff::FixedInt, ff::FixedInt> _spinVel;

			ff::FixedInt _rotate;
			ff::PointFixedInt _scale;

			std::vector<int> _colors;
			std::vector<ff::ComPtr<ff::IAnimation>> _animations;

			bool _reverse;
			bool _hasAngle;
		};

		struct ParticleGroup
		{
			DirectX::XMFLOAT4X4 _matrix;
			ff::PixelTransform _transform;
			int _refs;
			int _effectId;
		};

		struct Particle
		{
			bool IsColor() const
			{
				return (_internalType & 1) != 0;
			}

			void SetColor(int color)
			{
				_internalType |= 1;
				_color = color;
			}

			void SetAnimation(ff::IAnimation* anim)
			{
				_anim = anim;
			}

			// Position of particle within its group
			float _angle;
			float _angleVel;
			float _dist;
			float _distVel;

			// Rendering of particle
			float _size;
			float _spin;
			float _spinVel;
			float _timer;

			unsigned short _group;
			unsigned short _delay;
			unsigned short _life;
			unsigned char _internalType;
			unsigned char _type;

			union
			{
				int _color;
				ff::IAnimation* _anim;
			};
		};

		void AdvanceBlock();
		void AdvanceNow();

		unsigned short AddGroup(const ff::PixelTransform& transform, int effectId, int count);
		void ReleaseGroup(unsigned short groupId);
		const DirectX::XMFLOAT4X4& GetMatrix(unsigned short groupId) const;

		std::vector<Particle> _particlesNew;
		std::vector<Particle> _particlesAsync;
		std::vector<ParticleGroup> _groups;
		ff::WinHandle _event;

	public:
		class Effect
		{
		public:
			Effect() = default;
			Effect(Effect&&) = default;
			Effect(const Effect&) = default;
			Effect(const ff::Value* value);

			Effect& operator=(Effect&&) = default;
			Effect& operator=(const Effect&) = default;

			int Add(Particles& particles, ff::PointFixedInt pos, const EffectOptions* options = nullptr) const;
			int Add(Particles& particles, const ff::PointFixedInt* pos, size_t posCount, const EffectOptions* options = nullptr) const;

		private:
			std::vector<Spec> _specs;
		};
	};
}
