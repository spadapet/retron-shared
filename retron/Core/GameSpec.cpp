#include "pch.h"
#include "Core/GameSpec.h"
#include "Dict/Dict.h"
#include "Module/Module.h"
#include "Resource/Resources.h"

static ff::StaticString PROP_ALLOW_DEBUG(L"allowDebug");
static ff::StaticString PROP_APP(L"app");
static ff::StaticString PROP_DIFFICULTIES(L"difficulties");
static ff::StaticString PROP_GRUNTS(L"grunts");
static ff::StaticString PROP_LEVEL_SET(L"levelSet");
static ff::StaticString PROP_LEVEL_SETS(L"levelSets");
static ff::StaticString PROP_LEVELS(L"levels");
static ff::StaticString PROP_LIVES(L"lives");
static ff::StaticString PROP_NAME(L"name");
static ff::StaticString PROP_PLAYER_START(L"playerStart");

ReTron::GameSpec ReTron::GameSpec::Load(ff::IResourceAccess* resources)
{
	ff::IResourceAccess* res = ff::GetThisModule().GetResources();
	const ff::Dict& appDict = res->GetValue(::PROP_APP)->GetValue<ff::DictValue>();
	const ff::Dict& diffsDict = res->GetValue(::PROP_DIFFICULTIES)->GetValue<ff::DictValue>();
	const ff::Dict& levelSetsDict = res->GetValue(::PROP_LEVEL_SETS)->GetValue<ff::DictValue>();
	const ff::Dict& levelsDict = res->GetValue(::PROP_LEVELS)->GetValue<ff::DictValue>();

	GameSpec spec;
	spec._allowDebug = appDict.Get<ff::BoolValue>(::PROP_ALLOW_DEBUG);

	for (ff::String name : diffsDict.GetAllNames())
	{
		const ff::Dict& dict = diffsDict.Get<ff::DictValue>(name);

		DifficultySpec diffSpec;
		diffSpec._name = dict.Get<ff::StringValue>(::PROP_NAME);
		diffSpec._lives = dict.Get<ff::SizeValue>(::PROP_LIVES);
		diffSpec._levelSet = dict.Get<ff::StringValue>(::PROP_LEVEL_SET);

		spec._difficulties.SetKey(std::move(name), std::move(diffSpec));
	}

	for (ff::String name : levelSetsDict.GetAllNames())
	{
		LevelSetSpec levelSetSpec;
		levelSetSpec._levels = levelSetsDict.Get<ff::StringStdVectorValue>(name);

		spec._levelSets.SetKey(std::move(name), std::move(levelSetSpec));
	}

	for (ff::String name : levelsDict.GetAllNames())
	{
		const ff::Dict& dict = levelsDict.Get<ff::DictValue>(name);

		LevelSpec levelSpec;
		levelSpec._playerStart = dict.Get<ff::PointFixedIntValue>(::PROP_PLAYER_START, Constants::RENDER_LEVEL_SIZE / 2_f);
		levelSpec._grunts = dict.Get<ff::SizeValue>(::PROP_GRUNTS);

		spec._levels.SetKey(std::move(name), std::move(levelSpec));
	}

	return spec;
}
