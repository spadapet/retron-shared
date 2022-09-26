#pragma once

namespace retron
{
    enum class game_flags
    {
        normal = 0x00,
        infinite_lives = 0x01,
        no_bosses = 0x02,

        default_ = normal
    };

    enum class game_players
    {
        one = 0,
        two_take_turns = 1,
        two_coop = 2,

        default_ = one
    };

    enum class game_difficulty
    {
        baby = 0,
        easy = 1,
        normal = 2,
        hard = 3,

        default_ = normal
    };

    struct game_options
    {
        game_options();

        std::string_view difficulty_id() const;
        size_t player_count() const;
        bool coop() const;

        static const int CURRENT_VERSION = 1;

        int version;
        game_flags flags;
        game_players players;
        game_difficulty difficulty;
    };
}

NS_DECLARE_REFLECTION_ENUM(retron::game_flags)
NS_DECLARE_REFLECTION_ENUM(retron::game_players)
NS_DECLARE_REFLECTION_ENUM(retron::game_difficulty)
