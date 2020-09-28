#include "pch.h"
#include "Graph/Anim/Animation.h"
#include "Graph/Render/MatrixStack.h"
#include "Graph/Render/PixelRenderer.h"
#include "Graph/Render/RendererActive.h"
#include "Level/Particles.h"
#include "Thread/ThreadPool.h"
#include "Thread/ThreadUtil.h"

static ff::StaticString PROP_ANGLE(L"angle");
static ff::StaticString PROP_ANGLE_VELOCITY(L"angleVelocity");
static ff::StaticString PROP_ANIMATIONS(L"animations");
static ff::StaticString PROP_COLORS(L"colors");
static ff::StaticString PROP_COUNT(L"count");
static ff::StaticString PROP_DELAY(L"delay");
static ff::StaticString PROP_DISTANCE(L"distance");
static ff::StaticString PROP_LIFE(L"life");
static ff::StaticString PROP_REVERSE(L"reverse");
static ff::StaticString PROP_ROTATE(L"rotate");
static ff::StaticString PROP_SCALE(L"scale");
static ff::StaticString PROP_SIZE(L"size");
static ff::StaticString PROP_SPIN(L"spin");
static ff::StaticString PROP_SPIN_VELOCITY(L"spinVelocity");
static ff::StaticString PROP_VELOCITY(L"velocity");

ReTron::Particles::Particles()
	: _event(ff::CreateEvent())
{
	_particlesNew.reserve(256);
	_particlesAsync.reserve(512);
	_groups.reserve(64);
}

ff::AtScope ReTron::Particles::AdvanceAsync()
{
	ff::GetThreadPool()->AddTask(std::bind(&Particles::AdvanceNow, this));
	return std::bind(&Particles::AdvanceBlock, this);
}

void ReTron::Particles::AdvanceBlock()
{
	ff::WaitForEventAndReset(_event);

	if (_particlesNew.size())
	{
		_particlesAsync.insert(_particlesAsync.end(), _particlesNew.begin(), _particlesNew.end());
		_particlesNew.clear();
	}

	for (auto& i : _groups)
	{
		if (!i._refs)
		{
			// Allow reuse of this group
			i._refs = -1;
			i._effectId = 0;
			i._animations.clear();
		}
	}
}

void ReTron::Particles::AdvanceNow()
{
	for (size_t i = 0; i < _particlesAsync.size(); )
	{
		Particle& p = _particlesAsync[i];
		if (p._delay > 0)
		{
			p._delay--;
			i++;
		}
		else if (p._life > 1)
		{
			p._life--;
			p._angle += p._angleVel;
			p._dist += p._distVel;
			p._spin += p._spinVel;
			p._timer += ff::SECONDS_PER_ADVANCE_F;
			i++;
		}
		else
		{
			ReleaseGroup(p._group);
			p = std::move(_particlesAsync.back());
			_particlesAsync.pop_back();
		}
	}

	::SetEvent(_event);
}

unsigned short ReTron::Particles::AddGroup(const ff::PixelTransform& transform, int effectId, int count, const std::vector<ff::ComPtr<ff::IAnimation>>& animations)
{
	ParticleGroup group;
	DirectX::XMStoreFloat4x4(&group._matrix, transform.GetMatrix());
	group._transform = transform;
	group._refs = count;
	group._effectId = effectId;
	group._animations = animations;

	for (size_t i = 0; i < _groups.size(); i++)
	{
		if (_groups[i]._refs == -1)
		{
			_groups[i] = std::move(group);
			return static_cast<unsigned short>(i);
		}
	}

	_groups.push_back(std::move(group));
	return static_cast<unsigned short>(_groups.size() - 1);
}

void ReTron::Particles::ReleaseGroup(unsigned short groupId)
{
	verify(--_groups[groupId]._refs >= 0);
}

const DirectX::XMFLOAT4X4& ReTron::Particles::GetMatrix(unsigned short groupId) const
{
	assert(_groups[groupId]._refs > 0);
	return _groups[groupId]._matrix;
}

void ReTron::Particles::Render(ff::PixelRendererActive& render, uint8_t type)
{
	ff::Transform transform = ff::Transform::Identity();

	for (const Particle& p : _particlesAsync)
	{
		if (!p._delay && type == p._type)
		{
			DirectX::XMFLOAT2 angleCosSin;
			DirectX::XMScalarSinCosEst(&angleCosSin.y, &angleCosSin.x, p._angle);
			DirectX::XMFLOAT2 pos(angleCosSin.x * p._dist, angleCosSin.y * p._dist);
			DirectX::XMStoreFloat2(&pos,
				DirectX::XMVector2Transform(
					DirectX::XMLoadFloat2(&pos),
					DirectX::XMLoadFloat4x4(&GetMatrix(p._group))));

			if (p.IsColor())
			{
				render.GetRenderer()->DrawPaletteFilledRectangle(ff::RectFloat(pos.x - p._size, pos.y - p._size, pos.x + p._size, pos.y + p._size), p._color);
			}
			else
			{
				transform._position.SetPoint(pos.x, pos.y);
				transform._scale.SetPoint(p._size, p._size);
				transform._rotation = p._spin;
				p._anim->RenderFrame(render.GetRenderer(), transform, p._timer * p._anim->GetFramesPerSecond());
			}
		}
	}
}

bool ReTron::Particles::IsEffectActive(int effectId) const
{
	for (auto& group : _groups)
	{
		if (group._effectId == effectId && group._refs != -1)
		{
			return true;
		}
	}

	return false;
}

void ReTron::Particles::SetEffectPosition(int effectId, ff::PointFixedInt pos)
{
	for (auto& group : _groups)
	{
		if (group._effectId == effectId && group._refs != -1)
		{
			group._transform._position = pos;
			DirectX::XMStoreFloat4x4(&group._matrix, group._transform.GetMatrix());
		}
	}
}

template<typename ValueT, typename T = decltype(((ValueT*)0)->GetValue())>
static std::pair<T, T> ReadPair(const ff::Dict& dict, ff::StringRef name, T default1, T default2, bool* hasValue)
{
	bool stackHasValue;
	hasValue = hasValue ? hasValue : &stackHasValue;
	*hasValue = false;

	ff::ValuePtr value = dict.GetValue(name);
	if (value)
	{
		ff::ValuePtrT<ValueT> valuet = value;
		if (valuet)
		{
			*hasValue = true;
			return std::make_pair(valuet.GetValue(), valuet.GetValue());
		}

		ff::ValuePtrT<ff::ValueVectorValue> vector = value;
		if (vector && vector.GetValue().Size() == 1)
		{
			valuet = vector.GetValue()[0];
			if (valuet)
			{
				*hasValue = true;
				return std::make_pair(valuet.GetValue(), valuet.GetValue());
			}
		}

		if (vector && vector.GetValue().Size() > 1)
		{
			ff::ValuePtrT<ValueT> v0 = vector.GetValue()[0];
			ff::ValuePtrT<ValueT> v1 = vector.GetValue()[1];
			if (v0 && v1)
			{
				*hasValue = true;
				return std::make_pair(v0.GetValue(), v1.GetValue());
			}
		}
	}

	return std::make_pair(default1, default2);
}

ReTron::Particles::Spec::Spec(const ff::Dict& dict)
{
	_count = ::ReadPair<ff::IntValue>(dict, ::PROP_COUNT, 32, 32, nullptr);
	_delay = ::ReadPair<ff::IntValue>(dict, ::PROP_DELAY, 0, 0, nullptr);
	_life = ::ReadPair<ff::IntValue>(dict, ::PROP_LIFE, 32, 32, nullptr);

	_dist = ::ReadPair<ff::FixedIntValue>(dict, ::PROP_DISTANCE, 0_f, 0_f, nullptr);
	_distVel = ::ReadPair<ff::FixedIntValue>(dict, ::PROP_VELOCITY, 1_f, 1_f, nullptr);
	_size = ::ReadPair<ff::FixedIntValue>(dict, ::PROP_SIZE, 1_f, 1_f, nullptr);
	_angle = ::ReadPair<ff::FixedIntValue>(dict, ::PROP_ANGLE, 0_f, 360_f, &_hasAngle);
	_angleVel = ::ReadPair<ff::FixedIntValue>(dict, ::PROP_ANGLE_VELOCITY, 0_f, 0_f, nullptr);
	_spin = ::ReadPair<ff::FixedIntValue>(dict, ::PROP_SPIN, 0_f, 0_f, nullptr);
	_spinVel = ::ReadPair<ff::FixedIntValue>(dict, ::PROP_SPIN_VELOCITY, 0_f, 0_f, nullptr);

	_rotate = dict.Get<ff::FixedIntValue>(::PROP_ROTATE);
	_scale = dict.Get<ff::PointFixedIntValue>(::PROP_SCALE, ff::PointFixedInt::Ones());
	_reverse = dict.Get<ff::BoolValue>(::PROP_REVERSE);
	_colors = dict.Get<ff::IntStdVectorValue>(::PROP_COLORS);

	for (ff::ValuePtr value : dict.Get<ff::ValueVectorValue>(::PROP_ANIMATIONS))
	{
		ff::ValuePtrT<ff::SharedResourceWrapperValue> valuet = value;
		if (valuet)
		{
			ff::AutoResourceValue autoValue(valuet.GetValue());
			ff::TypedResource<ff::IAnimation> typedValue(autoValue);
			ff::ComPtr<ff::IAnimation> anim = typedValue.Flush();
			assert(anim);

			if (anim)
			{
				_animations.push_back(anim);
			}
		}
	}
}

void ReTron::Particles::Spec::Add(Particles& particles, ff::PointFixedInt pos, int effectId, const EffectOptions& options) const
{
	const int count = Random::Range(_count);
	noAssertRet(count > 0);

	unsigned short groupId = particles.AddGroup(ff::PixelTransform::Create(pos, _scale * options._scale, _rotate + options._rotate), effectId, count, _animations);

	for (int i = 0; i < count; i++)
	{
		Particle p;

		p._angle = (float)Random::Range(_hasAngle ? _angle : options._angle) * ff::DEG_TO_RAD_F;
		p._angleVel = (float)Random::Range(_angleVel) * ff::DEG_TO_RAD_F;
		p._dist = Random::Range(_dist);
		p._distVel = Random::Range(_distVel);

		p._size = Random::Range(_size);
		p._spin = Random::Range(_spin);
		p._spinVel = Random::Range(_spinVel);
		p._timer = 0;

		p._delay = Random::Range(_delay);
		p._life = Random::Range(_life);
		p._type = options._type;
		p._internalType = 0;
		p._group = groupId;

		if (_animations.size())
		{
			ff::IAnimation* anim = _animations[Random::Range(0u, _animations.size() - 1)];
			p.SetAnimation(anim);
		}
		else
		{
			p._size /= 2.0f;

			int color = _colors.size() ? _colors[Random::Range(0u, _colors.size() - 1)] : 0;
			p.SetColor(color);
		}

		if ((_reverse ^ options._reverse) != 0)
		{
			p._delay = (_delay.second + _life.second) - (p._delay + p._life);
			p._dist += p._distVel * p._life;
			p._distVel = -p._distVel;
			p._angle += p._angleVel * p._life;
			p._angleVel = -p._angleVel;
		}

		particles._particlesNew.push_back(p);
	}
}

ReTron::Particles::Effect::Effect(const ff::Value* value)
{
	std::vector<ff::ValuePtrT<ff::DictValue>> dictValues;

	ff::ValuePtrT<ff::DictValue> dictValue = value;
	ff::ValuePtrT<ff::ValueVectorValue> vectorValue = value;

	if (dictValue)
	{
		dictValues.push_back(dictValue);
	}
	else if (vectorValue)
	{
		for (ff::ValuePtr value2 : vectorValue.GetValue())
		{
			dictValue = value2;
			if (dictValue)
			{
				dictValues.push_back(dictValue);
			}
		}
	}
	else
	{
		assert(false);
	}

	for (ff::ValuePtrT<ff::DictValue> value2 : dictValues)
	{
		_specs.emplace_back(value2.GetValue());
	}
}

int ReTron::Particles::Effect::Add(Particles& particles, ff::PointFixedInt pos, const EffectOptions* options) const
{
	return Add(particles, &pos, 1, options);
}

int ReTron::Particles::Effect::Add(Particles& particles, const ff::PointFixedInt* pos, size_t posCount, const EffectOptions* options) const
{
	static std::atomic_int s_effectId;
	int effectId = s_effectId.fetch_add(1) + 1;

	static EffectOptions defaultOptions;
	options = options ? options : &defaultOptions;

	for (const Spec& spec : _specs)
	{
		spec.Add(particles, *pos, effectId, *options);

		if (posCount > 1)
		{
			posCount--;
			pos++;
		}
	}

	return effectId;
}
