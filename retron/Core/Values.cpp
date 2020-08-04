#include "pch.h"
#include "Core/Values.h"
#include "Module/Module.h"
#include "Resource/Resources.h"

static ff::StaticString PROP_ALLOW_DEBUG(L"allowDebug");
static ff::StaticString PROP_VALUES(L"values");

void ReTron::Values::Load()
{
	Init();

	ff::IResourceAccess* res = ff::GetThisModule().GetResources();
	const ff::Dict& dict = res->GetValue(::PROP_VALUES)->GetValue<ff::DictValue>();

	_allowDebug = dict.Get<ff::BoolValue>(::PROP_ALLOW_DEBUG);
}

void ReTron::Values::Init()
{
	std::memset(this, 0, sizeof(ReTron::Values));
	_version = ReTron::Values::CurrentVersion;
}
