#pragma once

namespace retron
{
    enum class entity_category
    {
        none = 0,

        animation = 1 << 16,
        player = 1 << 17,
        bullet = 1 << 18,
        bonus = 1 << 19,
        enemy = 1 << 20,
        electrode = 1 << 21,
        level = 1 << 22,
    };

    enum class entity_type
    {
        none = 0,

        // Categories
        category_mask = 0x7FFF0000,

        category_animation = static_cast<int>(retron::entity_category::animation),
        category_player = static_cast<int>(retron::entity_category::player),
        category_bullet = static_cast<int>(retron::entity_category::bullet),
        category_bonus = static_cast<int>(retron::entity_category::bonus),
        category_enemy = static_cast<int>(retron::entity_category::enemy),
        category_electrode = static_cast<int>(retron::entity_category::electrode),
        category_level = static_cast<int>(retron::entity_category::level),

        // Collisions

        category_player_collision = category_bonus | category_enemy | category_electrode | category_level,
        category_bullet_collision = category_enemy | category_electrode | category_level,
        category_bonus_collision = category_enemy | category_electrode | category_level,
        category_enemy_collision = category_electrode | category_level,

        // Types

        animation_top = category_animation | (1 << 0),
        animation_bottom = category_animation | (1 << 1),
        bullet_player_0 = category_bullet | (1 << 0),
        bullet_player_1 = category_bullet | (1 << 1),
        player_0 = category_player | (1 << 0),
        player_1 = category_player | (1 << 1),
        bonus_woman = category_bonus | (1 << 0),
        bonus_man = category_bonus | (1 << 1),
        bonus_girl = category_bonus | (1 << 2),
        bonus_boy = category_bonus | (1 << 3),
        bonus_dog = category_bonus | (1 << 4),
        enemy_grunt = category_enemy | (1 << 0),
        enemy_hulk = category_enemy | (1 << 1),
        enemy_flipper = category_enemy | (1 << 2),
        electrode_0 = category_electrode | (1 << 0),
        electrode_1 = category_electrode | (1 << 1),
        electrode_2 = category_electrode | (1 << 2),
        electrode_3 = category_electrode | (1 << 3),
        level_bounds = category_level | (1 << 0),
        level_box = category_level | (1 << 1),
    };
}

namespace retron::entity_util
{
    constexpr retron::entity_category category(retron::entity_type type)
    {
        return static_cast<retron::entity_category>(ff::flags::get(type, retron::entity_type::category_mask));
    }
}
