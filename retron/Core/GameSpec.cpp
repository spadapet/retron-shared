#include "pch.h"
#include "Core/GameSpec.h"
#include "Dict/Dict.h"
#include "Module/Module.h"
#include "Resource/Resources.h"

static ff::StaticString PROP_ALLOW_DEBUG(L"allowDebug");
static ff::StaticString PROP_APP(L"app");
static ff::StaticString PROP_DEFAULT(L"default");
static ff::StaticString PROP_DIFFICULTIES(L"difficulties");
static ff::StaticString PROP_GRUNT_MAX_TICKS(L"gruntMaxTicks");
static ff::StaticString PROP_GRUNT_MAX_TICKS_RATE(L"gruntMaxTicksRate");
static ff::StaticString PROP_GRUNT_MIN_TICKS(L"gruntMinTicks");
static ff::StaticString PROP_GRUNT_MOVE(L"gruntMove");
static ff::StaticString PROP_GRUNT_TICK_FRAMES(L"gruntTickFrames");
static ff::StaticString PROP_HEIGHT(L"height");
static ff::StaticString PROP_LAYERS(L"layers");
static ff::StaticString PROP_JOYSTICK_MIN(L"joystickMin");
static ff::StaticString PROP_JOYSTICK_MAX(L"joystickMax");
static ff::StaticString PROP_LEVEL_SET(L"levelSet");
static ff::StaticString PROP_LEVEL_SETS(L"levelSets");
static ff::StaticString PROP_LEVELS(L"levels");
static ff::StaticString PROP_LIVES(L"lives");
static ff::StaticString PROP_LOOP_START(L"loopStart");
static ff::StaticString PROP_NAME(L"name");
static ff::StaticString PROP_OBJECTS(L"objects");
static ff::StaticString PLAYER_SHOT_COUNTER(L"playerShotCounter");
static ff::StaticString PLAYER_MOVE(L"playerMove");
static ff::StaticString PLAYER_SHOT_MOVE(L"playerShotMove");
static ff::StaticString PROP_PLAYER_START(L"playerStart");
static ff::StaticString PROP_POINT(L"point");
static ff::StaticString PROP_PROPERTIES(L"properties");
static ff::StaticString PROP_TYPE(L"type");
static ff::StaticString PROP_VALUE(L"value");
static ff::StaticString PROP_WIDTH(L"width");
static ff::StaticString PROP_X(L"x");
static ff::StaticString PROP_Y(L"y");

template<typename ValueType>
static auto GetValue(const ff::Dict& dict, const ff::Dict& defaultDict, ff::StringRef name)
{
	return dict.Get<ValueType>(name, defaultDict.Get<ValueType>(name));
}

static ReTron::DifficultySpec LoadDifficultySpec(const ff::Dict& dict, const ff::Dict& defaultDict)
{
	ReTron::DifficultySpec diffSpec{};
	diffSpec._name = ::GetValue<ff::StringValue>(dict, defaultDict, ::PROP_NAME);
	diffSpec._levelSet = ::GetValue<ff::StringValue>(dict, defaultDict, ::PROP_LEVEL_SET);
	diffSpec._lives = ::GetValue<ff::SizeValue>(dict, defaultDict, ::PROP_LIVES);
	diffSpec._gruntMaxTicks = ::GetValue<ff::SizeValue>(dict, defaultDict, ::PROP_GRUNT_MAX_TICKS);
	diffSpec._gruntMinTicks = ::GetValue<ff::SizeValue>(dict, defaultDict, ::PROP_GRUNT_MIN_TICKS);
	diffSpec._gruntMaxTicksRate = ::GetValue<ff::SizeValue>(dict, defaultDict, ::PROP_GRUNT_MAX_TICKS_RATE);
	diffSpec._gruntTickFrames = ::GetValue<ff::SizeValue>(dict, defaultDict, ::PROP_GRUNT_TICK_FRAMES);
	diffSpec._gruntMove = ::GetValue<ff::FixedIntValue>(dict, defaultDict, ::PROP_GRUNT_MOVE);
	diffSpec._playerMove = ::GetValue<ff::FixedIntValue>(dict, defaultDict, ::PLAYER_MOVE);
	diffSpec._playerShotMove = ::GetValue<ff::FixedIntValue>(dict, defaultDict, ::PLAYER_SHOT_MOVE);
	diffSpec._playerShotCounter = ::GetValue<ff::SizeValue>(dict, defaultDict, ::PLAYER_SHOT_COUNTER);

	return diffSpec;
}

static ReTron::LevelSetSpec LoadLevelSetSpec(const ff::Dict& dict)
{
	ReTron::LevelSetSpec levelSetSpec{};
	levelSetSpec._levels = dict.Get<ff::StringStdVectorValue>(::PROP_LEVELS);
	levelSetSpec._loopStart = dict.Get<ff::SizeValue>(::PROP_LOOP_START);

	return levelSetSpec;
}

static ReTron::LevelObjectsSpec LoadLevelObjectsSpec(ff::RectFixedInt rect, const ff::Vector<ff::ValuePtr>& properties)
{
	ReTron::LevelObjectsSpec objectsSpec{};
	objectsSpec._type = ReTron::LevelRect::Type::Objects;
	objectsSpec._rect = rect;

	for (ff::ValuePtr propertyValue : properties)
	{
		const ff::Dict& propertyDict = propertyValue->GetValue<ff::DictValue>();
		ff::StringRef propertyName = propertyDict.Get<ff::StringValue>(::PROP_NAME);
		size_t propertyValue = propertyDict.Get<ff::SizeValue>(::PROP_VALUE);

		if (propertyName == L"Electrode")
		{
			objectsSpec._electrode = propertyValue;
		}
		else if (propertyName == L"Grunt")
		{
			objectsSpec._grunt = propertyValue;
		}
		else if (propertyName == L"Hulk")
		{
			objectsSpec._hulk = propertyValue;
		}
		else if (propertyName == L"Bonus.Woman")
		{
			objectsSpec._bonusWoman = propertyValue;
		}
		else if (propertyName == L"Bonus.Man")
		{
			objectsSpec._bonusMan = propertyValue;
		}
		else if (propertyName == L"Bonus.Child")
		{
			objectsSpec._bonusChild = propertyValue;
		}
		else
		{
			assertSz(false, ff::String::format_new(L"Unknown entity type: %s", propertyName.c_str()).c_str());
		}
	}

	return objectsSpec;
}

static void LoadLevelSpecObjectLayer(ReTron::LevelSpec& levelSpec, const ff::Dict& layerDict)
{
	for (ff::ValuePtr objectValue : layerDict.Get<ff::ValueVectorValue>(::PROP_OBJECTS))
	{
		const ff::Dict& objectDict = objectValue->GetValue<ff::DictValue>();

		ff::String objectType = objectDict.Get<ff::StringValue>(::PROP_TYPE);
		if (objectType == L"Player")
		{
			if (objectDict.Get<ff::BoolValue>(::PROP_POINT))
			{
				levelSpec._playerStart.SetPoint(objectDict.Get<ff::FixedIntValue>(::PROP_X), objectDict.Get<ff::FixedIntValue>(::PROP_Y));
			}
		}
		else
		{
			ff::PointFixedInt pos(objectDict.Get<ff::FixedIntValue>(::PROP_X), objectDict.Get<ff::FixedIntValue>(::PROP_Y));
			ff::PointFixedInt size(objectDict.Get<ff::FixedIntValue>(::PROP_WIDTH), objectDict.Get<ff::FixedIntValue>(::PROP_HEIGHT));
			ReTron::LevelRect levelRect{};
			levelRect._rect = ff::RectFixedInt(pos, pos + size);

			if (objectType == L"Bounds")
			{
				levelRect._type = ReTron::LevelRect::Type::Bounds;
				levelSpec._rects.push_back(levelRect);
			}
			else if (objectType == L"Box")
			{
				levelRect._type = ReTron::LevelRect::Type::Box;
				levelSpec._rects.push_back(levelRect);
			}
			else if (objectType == L"Safe")
			{
				levelRect._type = ReTron::LevelRect::Type::Safe;
				levelSpec._rects.push_back(levelRect);
			}
			else if (objectType == L"Objects")
			{
				levelRect._type = ReTron::LevelRect::Type::Objects;
				ReTron::LevelObjectsSpec objectsSpec = ::LoadLevelObjectsSpec(levelRect._rect, objectDict.Get<ff::ValueVectorValue>(::PROP_PROPERTIES));
				levelSpec._objects.push_back(std::move(objectsSpec));
			}
			else
			{
				assertSz(false, ff::String::format_new(L"Unknown level object type: %s", objectType.c_str()).c_str());
			}

			assertSz(levelRect._rect.Area(), ff::String::format_new(L"Invalid size for object: %s", objectType.c_str()).c_str());
		}
	}
}

static void LoadLevelSpecLayer(ReTron::LevelSpec& levelSpec, const ff::Dict& layerDict)
{
	if (layerDict.Get<ff::StringValue>(::PROP_TYPE) == L"objectgroup")
	{
		::LoadLevelSpecObjectLayer(levelSpec, layerDict);
	}
}

static ReTron::LevelSpec LoadLevelSpec(const ff::Dict& dict)
{
	ReTron::LevelSpec levelSpec{};
	levelSpec._playerStart = ff::PointFixedInt::Zeros();

	for (ff::ValuePtr layerValue : dict.Get<ff::ValueVectorValue>(::PROP_LAYERS))
	{
		::LoadLevelSpecLayer(levelSpec, layerValue->GetValue<ff::DictValue>());
	}

	return levelSpec;
}

ReTron::GameSpec ReTron::GameSpec::Load(ff::IResourceAccess* resources)
{
	const ff::Dict& appDict = resources->GetValue(::PROP_APP)->GetValue<ff::DictValue>();
	const ff::Dict& diffsDict = resources->GetValue(::PROP_DIFFICULTIES)->GetValue<ff::DictValue>();
	const ff::Dict& levelSetsDict = resources->GetValue(::PROP_LEVEL_SETS)->GetValue<ff::DictValue>();
	const ff::Dict& levelsDict = resources->GetValue(::PROP_LEVELS)->GetValue<ff::DictValue>();
	const ff::Dict& defaultDiffDict = diffsDict.Get<ff::DictValue>(::PROP_DEFAULT);

	GameSpec spec{};
	spec._allowDebug = appDict.Get<ff::BoolValue>(::PROP_ALLOW_DEBUG);
	spec._joystickMin = appDict.Get<ff::FixedIntValue>(::PROP_JOYSTICK_MIN);
	spec._joystickMax = appDict.Get<ff::FixedIntValue>(::PROP_JOYSTICK_MAX);

	for (ff::String name : diffsDict.GetAllNames())
	{
		if (name != ::PROP_DEFAULT.GetString())
		{
			DifficultySpec diffSpec = ::LoadDifficultySpec(diffsDict.Get<ff::DictValue>(name), defaultDiffDict);
			spec._difficulties.SetKey(std::move(name), std::move(diffSpec));
		}
	}

	for (ff::String name : levelSetsDict.GetAllNames())
	{
		LevelSetSpec levelSetSpec = ::LoadLevelSetSpec(levelSetsDict.Get<ff::DictValue>(name));
		spec._levelSets.SetKey(std::move(name), std::move(levelSetSpec));
	}

	for (ff::String name : levelsDict.GetAllNames())
	{
		LevelSpec levelSpec = ::LoadLevelSpec(levelsDict.Get<ff::DictValue>(name));
		spec._levels.SetKey(std::move(name), std::move(levelSpec));
	}

	return spec;
}

