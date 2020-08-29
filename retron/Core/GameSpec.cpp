#include "pch.h"
#include "Core/GameSpec.h"
#include "Dict/Dict.h"
#include "Module/Module.h"
#include "Resource/Resources.h"

static ff::StaticString PROP_ALLOW_DEBUG(L"allowDebug");
static ff::StaticString PROP_APP(L"app");
static ff::StaticString PROP_DIFFICULTIES(L"difficulties");
static ff::StaticString PROP_HEIGHT(L"height");
static ff::StaticString PROP_LAYERS(L"layers");
static ff::StaticString PROP_LEVEL_SET(L"levelSet");
static ff::StaticString PROP_LEVEL_SETS(L"levelSets");
static ff::StaticString PROP_LEVELS(L"levels");
static ff::StaticString PROP_LIVES(L"lives");
static ff::StaticString PROP_NAME(L"name");
static ff::StaticString PROP_OBJECTS(L"objects");
static ff::StaticString PROP_PLAYER_START(L"playerStart");
static ff::StaticString PROP_POINT(L"point");
static ff::StaticString PROP_PROPERTIES(L"properties");
static ff::StaticString PROP_TYPE(L"type");
static ff::StaticString PROP_VALUE(L"value");
static ff::StaticString PROP_WIDTH(L"width");
static ff::StaticString PROP_X(L"x");
static ff::StaticString PROP_Y(L"y");

static ReTron::DifficultySpec LoadDifficultySpec(const ff::Dict& dict)
{
	ReTron::DifficultySpec diffSpec{};
	diffSpec._name = dict.Get<ff::StringValue>(::PROP_NAME);
	diffSpec._lives = dict.Get<ff::SizeValue>(::PROP_LIVES);
	diffSpec._levelSet = dict.Get<ff::StringValue>(::PROP_LEVEL_SET);

	return diffSpec;
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

	GameSpec spec{};
	spec._allowDebug = appDict.Get<ff::BoolValue>(::PROP_ALLOW_DEBUG);

	for (ff::String name : diffsDict.GetAllNames())
	{
		DifficultySpec diffSpec = ::LoadDifficultySpec(diffsDict.Get<ff::DictValue>(name));
		spec._difficulties.SetKey(std::move(name), std::move(diffSpec));
	}

	for (ff::String name : levelSetsDict.GetAllNames())
	{
		LevelSetSpec levelSetSpec{};
		levelSetSpec._levels = levelSetsDict.Get<ff::StringStdVectorValue>(name);
		spec._levelSets.SetKey(std::move(name), std::move(levelSetSpec));
	}

	for (ff::String name : levelsDict.GetAllNames())
	{
		LevelSpec levelSpec = ::LoadLevelSpec(levelsDict.Get<ff::DictValue>(name));
		spec._levels.SetKey(std::move(name), std::move(levelSpec));
	}

	return spec;
}
