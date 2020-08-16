#pragma once

namespace Strings
{
	extern const ff::StaticString ID_APP_STATE;
	extern const ff::StaticString ID_GAME_OPTIONS;
	extern const ff::StaticString ID_SYSTEM_OPTIONS;
}

namespace InputEvents
{
	extern const ff::hash_t ID_UP;
	extern const ff::hash_t ID_DOWN;
	extern const ff::hash_t ID_LEFT;
	extern const ff::hash_t ID_RIGHT;
	extern const ff::hash_t ID_SHOOT_UP;
	extern const ff::hash_t ID_SHOOT_DOWN;
	extern const ff::hash_t ID_SHOOT_LEFT;
	extern const ff::hash_t ID_SHOOT_RIGHT;
	extern const ff::hash_t ID_ACTION;
	extern const ff::hash_t ID_CANCEL;
	extern const ff::hash_t ID_PAUSE;
	extern const ff::hash_t ID_START;

	extern const ff::hash_t ID_DEBUG_STEP_ONE_FRAME;
	extern const ff::hash_t ID_DEBUG_CANCEL_STEP_ONE_FRAME;
	extern const ff::hash_t ID_DEBUG_SPEED_SLOW;
	extern const ff::hash_t ID_DEBUG_SPEED_FAST;
	extern const ff::hash_t ID_DEBUG_SHOW_UI;
}

namespace Constants
{
	const size_t MAX_PLAYERS = 2;

	const ff::FixedInt RENDER_WIDTH = 480_f; // 1920 / 4
	const ff::FixedInt RENDER_HEIGHT = 270_f; // 1080 / 4
	const ff::PointFixedInt RENDER_SIZE(RENDER_WIDTH, RENDER_HEIGHT);
	const ff::RectFixedInt RENDER_RECT(0_f, 0_f, RENDER_WIDTH, RENDER_HEIGHT);

	const ff::FixedInt RENDER_LEVEL_WIDTH = 464_f;
	const ff::FixedInt RENDER_LEVEL_HEIGHT = 254_f;
	const ff::PointFixedInt RENDER_LEVEL_SIZE(RENDER_LEVEL_WIDTH, RENDER_LEVEL_HEIGHT);
	const ff::RectFixedInt RENDER_LEVEL_RECT(8_f, 8_f, 472_f, 262_f);

	const ff::FixedInt RENDER_SCALE = 4_f; // scale to a 1080p buffer, which then gets scaled to the screen

	const ff::FixedInt RENDER_WIDTH_HIGH = 1920_f;
	const ff::FixedInt RENDER_HEIGHT_HIGH = 1080_f;
	const ff::PointFixedInt RENDER_SIZE_HIGH(RENDER_WIDTH_HIGH, RENDER_HEIGHT_HIGH);
	const ff::RectFixedInt RENDER_RECT_HIGH(0_f, 0_f, RENDER_WIDTH_HIGH, RENDER_HEIGHT_HIGH);
}
