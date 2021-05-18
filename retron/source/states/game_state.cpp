#include "pch.h"
#include "source/core/app_service.h"
#include "source/level/level.h"
#include "source/states/game_over_state.h"
#include "source/states/game_state.h"
#include "source/states/high_score_state.h"
#include "source/states/ready_state.h"
#include "source/states/score_state.h"
#include "source/states/transition_state.h"

retron::game_state::game_state()
    : game_options_(retron::app_service::get().default_game_options())
    , difficulty_spec_(retron::app_service::get().game_spec().difficulties.at(std::string(this->game_options_.difficulty_id())))
    , level_set_spec(retron::app_service::get().game_spec().level_sets.at(this->difficulty_spec_.level_set))
    , playing_index(0)
{
    this->init_input();
    this->init_players();
    this->init_playing_states();
}

std::shared_ptr<ff::state> retron::game_state::advance_time()
{
    this->game_input_events->advance();

    for (auto& i : this->player_input_events)
    {
        i->advance();
    }

    ff::state::advance_time();

    std::shared_ptr<ff::state> new_state;

#if 0
    switch (this->playing().level().phase())
    {
        case retron::level_phase::ready:
            new_state = this->handle_level_ready();
            break;

        case retron::level_phase::won:
            new_state = this->handle_level_won();
            break;

        case retron::level_phase::dead:
            new_state = this->handle_level_dead();
            break;
    }
#endif

    return new_state;
}

size_t retron::game_state::child_state_count()
{
    return 1;
}

ff::state* retron::game_state::child_state(size_t index)
{
    return this->playing().state.get();
}

const retron::game_options& retron::game_state::game_options() const
{
    return this->game_options_;
}

const retron::difficulty_spec& retron::game_state::difficulty_spec() const
{
    return this->difficulty_spec_;
}

const ff::input_event_provider& retron::game_state::input_events(const retron::player& player) const
{
    return *this->player_input_events[player.index];
}

void retron::game_state::player_add_points(const retron::player& level_player, size_t points)
{
    retron::player& player = this->players[level_player.index].self_or_coop();
    player.points += points;

    if (player.next_life_points && player.points >= player.next_life_points)
    {
        const size_t next = this->difficulty_spec().next_free_life;
        size_t lives = 1;

        if (next)
        {
            lives += (player.points - player.next_life_points) / next;
            player.next_life_points += lives * next;
        }
        else
        {
            player.next_life_points = 0;
        }

        player.lives += lives;
    }
}

bool retron::game_state::player_add_life(const retron::player& level_player)
{
    retron::player& player = this->players[level_player.index].self_or_coop();
    if (player.lives > 0)
    {
        player.lives--;
        return true;
    }

    return false;
}

void retron::game_state::debug_restart_level()
{
    this->init_playing_states();
}

void retron::game_state::init_input()
{
    ff::auto_resource<ff::input_mapping> game_input_mapping = "game_controls";
    std::array<ff::auto_resource<ff::input_mapping>, constants::MAX_PLAYERS> player_input_mappings{ "player_controls", "player_controls" };
    std::vector<const ff::input_vk*> game_input_devices{ &ff::input::keyboard(), &ff::input::pointer() };
    std::array<std::vector<const ff::input_vk*>, constants::MAX_PLAYERS> player_input_devices;

    // Player specific devices
    for (size_t i = 0; i < player_input_devices.size(); i++)
    {
        // In coop, the first player must be controlled by a joystick
        if (!this->game_options_.coop() || i == 1)
        {
            player_input_devices[i].push_back(&ff::input::keyboard());
        }
    }

    // Add joysticks game-wide and player-specific
    for (size_t i = 0; i < ff::input::gamepad_count(); i++)
    {
        game_input_devices.push_back(&ff::input::gamepad(i));

        if (this->game_options_.coop())
        {
            if (i < player_input_devices.size())
            {
                player_input_devices[i].push_back(&ff::input::gamepad(i));
            }
        }
        else for (auto& input_devices : player_input_devices)
        {
            input_devices.push_back(&ff::input::gamepad(i));
        }
    }

    this->game_input_events = std::make_unique<ff::input_event_provider>(*game_input_mapping.object(), std::move(game_input_devices));

    for (size_t i = 0; i < this->player_input_events.size(); i++)
    {
        this->player_input_events[i] = std::make_unique<ff::input_event_provider>(*player_input_mappings[i].object(), std::move(player_input_devices[i]));
    }
}

static void init_player(retron::player& player, const retron::difficulty_spec& difficulty)
{
    player.lives = difficulty.lives;
    player.next_life_points = difficulty.first_free_life;
}

void retron::game_state::init_players()
{
    std::memset(this->players.data(), 0, this->players.size() * sizeof(retron::player));

    retron::player& coop_player = this->players.back();
    ::init_player(coop_player, this->difficulty_spec_);

    for (size_t i = 0; i < this->game_options_.player_count(); i++)
    {
        retron::player& player = this->players[i];
        player.index = i;

        if (this->game_options_.coop())
        {
            player.coop = &coop_player;
        }
        else
        {
            ::init_player(player, this->difficulty_spec_);
        }
    }
}

void retron::game_state::init_playing_states()
{
    this->playing_states.clear();
    this->playing_index = 0;

    if (this->game_options_.coop())
    {
        std::vector<retron::player*> players;
        for (size_t i = 0; i < this->game_options_.player_count(); i++)
        {
            players.push_back(&this->players[i]);
        }

        this->playing_states.push_back(this->create_playing(players));
    }
    else for (size_t i = 0; i < this->game_options_.player_count(); i++)
    {
        retron::player& player = this->players[i];
        this->playing_states.push_back(this->create_playing(std::vector<retron::player*>{ &player }));
    }
}

std::shared_ptr<ff::state> retron::game_state::handle_level_ready()
{
    this->playing().level->start();
    return nullptr;
}

std::shared_ptr<ff::state> retron::game_state::handle_level_won()
{
#if 0
    retron::player& player = this->playing().player_or_coop();
    retron::level_spec level_spec = this->level_spec(++player.level);

    auto old_level_state = this->playing().level_state;
    this->playing().level_state = std::make_shared<retron::level_state>(*this, std::move(level_spec), std::vector<retron::player*>(old_level_state->players()));
    *this->playing().play_state = std::make_shared<retron::transition_state>(old_level_state, this->playing().level_state, "transition_bg_2.png");
#endif
    return nullptr;
}

std::shared_ptr<ff::state> retron::game_state::handle_level_dead()
{
#if 0
    if (this->playing().level_state->game_over())
    {
        // Switch to next player that has lives, or show final game over screen
    }
    else if (this->player_add_life(0))
    {
        this->playing().level().start();
    }
    else
    {
        auto new_state = std::make_shared<retron::high_score_state>(this->playing().level_state);
        *this->playing().play_state = new_state;
    }
#endif

    return nullptr;
}

const retron::level_spec& retron::game_state::level_spec(size_t level_index)
{
    const size_t size = this->level_set_spec.levels.size();
    const size_t loop_start = this->level_set_spec.loop_start;
    const size_t level = (level_index >= size) ? (level_index - size) % (size - loop_start) + loop_start : level_index;
    const std::string& level_id = this->level_set_spec.levels[level];

    return retron::app_service::get().game_spec().levels.at(level_id);
}

retron::game_state::playing_t retron::game_state::create_playing(const std::vector<retron::player*>& players)
{
    std::vector<const retron::player*> const_all_players;
    for (size_t i = 0; i < (!this->game_options_.coop() ? this->game_options_.player_count() : 1); i++)
    {
        const_all_players.push_back(&this->players[i].self_or_coop());
    }

    std::vector<const retron::player*> const_players;
    for (const retron::player* player : players)
    {
        const_players.push_back(player);
    }

    retron::level_spec level_spec = this->level_spec(players.front()->self_or_coop().level);
    auto level = std::make_shared<retron::level>(*this, level_spec, const_players);
    auto states = std::make_shared<ff::state_list>();
    states->push(level);
    states->push(std::make_shared<retron::score_state>(const_all_players, players.front()->index));

    auto ready_state = std::make_shared<retron::ready_state>(*this, level, states);

    return playing_t{ players, level, ready_state->wrap() };
}

retron::game_state::playing_t& retron::game_state::playing()
{
    return this->playing_states[this->playing_index];
}

const retron::game_state::playing_t& retron::game_state::playing() const
{
    return this->playing_states[this->playing_index];
}
