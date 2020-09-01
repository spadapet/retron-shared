#include "pch.h"
#include "Core/Globals.h"

const ff::StaticString Strings::ID_APP_STATE(L"AppState");
const ff::StaticString Strings::ID_GAME_OPTIONS(L"GameOptions");
const ff::StaticString Strings::ID_SYSTEM_OPTIONS(L"SystemOptions");

const ff::hash_t InputEvents::ID_UP = ff::HashStaticString(L"up");
const ff::hash_t InputEvents::ID_DOWN = ff::HashStaticString(L"down");
const ff::hash_t InputEvents::ID_LEFT = ff::HashStaticString(L"left");
const ff::hash_t InputEvents::ID_RIGHT = ff::HashStaticString(L"right");
const ff::hash_t InputEvents::ID_SHOOT_UP = ff::HashStaticString(L"shoot-up");
const ff::hash_t InputEvents::ID_SHOOT_DOWN = ff::HashStaticString(L"shoot-down");
const ff::hash_t InputEvents::ID_SHOOT_LEFT = ff::HashStaticString(L"shoot-left");
const ff::hash_t InputEvents::ID_SHOOT_RIGHT = ff::HashStaticString(L"shoot-right");
const ff::hash_t InputEvents::ID_ACTION = ff::HashStaticString(L"action");
const ff::hash_t InputEvents::ID_CANCEL = ff::HashStaticString(L"cancel");
const ff::hash_t InputEvents::ID_PAUSE = ff::HashStaticString(L"pause");
const ff::hash_t InputEvents::ID_START = ff::HashStaticString(L"start");

const ff::hash_t InputEvents::ID_DEBUG_STEP_ONE_FRAME = ff::HashStaticString(L"stepOneFrame");
const ff::hash_t InputEvents::ID_DEBUG_CANCEL_STEP_ONE_FRAME = ff::HashStaticString(L"cancelStepOneFrame");
const ff::hash_t InputEvents::ID_DEBUG_SPEED_SLOW = ff::HashStaticString(L"speedSlow");
const ff::hash_t InputEvents::ID_DEBUG_SPEED_FAST = ff::HashStaticString(L"speedFast");
const ff::hash_t InputEvents::ID_DEBUG_RENDER_TOGGLE = ff::HashStaticString(L"debugRenderToggle");

static std::default_random_engine randomEngine(std::random_device{}());
static std::uniform_int_distribution<int> randomInt;

int Random::Positive()
{
	return ::randomInt(::randomEngine);
}

int Random::Sign()
{
	return -std::uniform_int_distribution<int>{0, 1}(::randomEngine);
}

int Random::Range(int start, int end)
{
	return std::uniform_int_distribution<int>{start, end}(::randomEngine);
}
