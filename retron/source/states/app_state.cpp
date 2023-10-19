#include "pch.h"
#include "source/core/audio.h"
#include "source/game/game_state.h"
#include "source/states/app_state.h"
#include "source/states/particle_lab_state.h"
#include "source/states/title_state.h"
#include "source/states/transition_state.h"
#include "source/states/ui_view_state.h"
#include "source/ui/debug_page.xaml.h"
#include "source/ui/particle_lab_page.xaml.h"

static const float PALETTE_CYCLES_PER_SECOND = 0.25f;
static retron::app_service* app_service;

retron::app_service& retron::app_service::get()
{
    assert(::app_service);
    return *::app_service;
}

ff::dxgi::draw_ptr retron::app_service::begin_palette_draw(ff::dxgi::command_context_base& context, ff::render_targets& targets)
{
    ff::dxgi::target_base& target = targets.target(context, ff::render_target_type::palette);
    ff::dxgi::depth_base& depth = targets.depth(context);

    return ff::dxgi_client().global_draw_device().begin_draw(context, target, &depth, retron::constants::RENDER_RECT, retron::constants::RENDER_RECT);
}

retron::app_state::app_state()
    : ff::game::app_state_base(
        ff::render_target(retron::constants::RENDER_SIZE.cast<size_t>(), &ff::dxgi::color_black(), 224),
        {
            { "palette_main", "player_0", ::PALETTE_CYCLES_PER_SECOND },
            { "palette_main", "player_1", ::PALETTE_CYCLES_PER_SECOND },
        })
    , render_debug_(retron::render_debug_t::none)
    , debug_cheats_(retron::debug_cheats_t::none)
{
    assert(!::app_service);
    ::app_service = this;
}

retron::app_state::~app_state()
{
    assert(::app_service == this);
    ::app_service = nullptr;
}

bool retron::app_state::allow_debug_commands()
{
    return ff::game::app_state_base::allow_debug_commands() || this->game_spec_.allow_debug();
}

bool retron::app_state::debug_command_override(size_t command_id)
{
    if (command_id == commands::ID_DEBUG_PARTICLE_LAB)
    {
        auto view = std::make_shared<ff::ui_view>(Noesis::MakePtr<retron::particle_lab_page>());
        auto view_state = std::make_shared<retron::ui_view_state>(view);
        auto particle_lab = std::make_shared<retron::particle_lab_state>(view);
        auto top_states = std::make_shared<ff::state_list>(std::vector<std::shared_ptr<ff::state>>{ particle_lab, view_state });
        this->show_debug_state(top_states);
        view->focused(true);
    }
    else if (command_id == commands::ID_DEBUG_RESTART_LEVEL)
    {
        std::shared_ptr<retron::game_state> game_state = std::dynamic_pointer_cast<retron::game_state>(this->game_state()->unwrap());
        if (game_state)
        {
            game_state->debug_restart_level();
        }
        else
        {
            this->debug_command(ff::game::app_state_base::ID_DEBUG_RESTART_GAME);
        }
    }
    else if (command_id == input_events::ID_DEBUG_RENDER_TOGGLE)
    {
        switch (this->render_debug_)
        {
            case retron::render_debug_t::set_0:
                this->render_debug_ = retron::render_debug_t::set_1;
                break;

            case retron::render_debug_t::set_1:
                this->render_debug_ = retron::render_debug_t::set_2;
                break;

            default:
            case retron::render_debug_t::set_2:
                this->render_debug_ = retron::render_debug_t::set_0;
                break;
        }
    }
    else if (command_id == input_events::ID_DEBUG_INVINCIBLE_TOGGLE)
    {
        this->debug_cheats_ = ff::flags::toggle(this->debug_cheats_, retron::debug_cheats_t::invincible);
    }
    else if (command_id == input_events::ID_DEBUG_COMPLETE_LEVEL)
    {
        this->debug_cheats_ = ff::flags::set(this->debug_cheats_, retron::debug_cheats_t::complete_level);
    }

    return false;
}

retron::audio& retron::app_state::audio()
{
    return *this->audio_;
}

const retron::particle_effect_base* retron::app_state::level_particle_effect(std::string_view name)
{
    if (this->level_particle_effects.empty() && this->level_particle_value.valid())
    {
        ff::dict level_particles_dict = this->level_particle_value.value()->get<ff::dict>();
        for (std::string_view name : level_particles_dict.child_names())
        {
            this->level_particle_effects.try_emplace(name, retron::particles::effect_t(level_particles_dict.get(name)));
        }

        this->level_particle_value = ff::auto_resource_value();
    }

    auto i = this->level_particle_effects.find(name);
    return (i != this->level_particle_effects.cend()) ? &i->second : nullptr;
}

const retron::game_options& retron::app_state::default_game_options() const
{
    return this->game_options_;
}

const retron::game_spec& retron::app_state::game_spec() const
{
    return this->game_spec_;
}

const ff::game::system_options& retron::app_state::system_options() const
{
    return ff::game::app_state_base::system_options();
}

void retron::app_state::system_options(const ff::game::system_options& options)
{
    ff::game::app_state_base::system_options(options);
}

void retron::app_state::default_game_options(const retron::game_options& options)
{
    this->game_options_ = options;
}

ff::dxgi::palette_base* retron::app_state::palette()
{
    return this->ff::game::app_state_base::palette(0);
}

ff::dxgi::palette_base& retron::app_state::player_palette(size_t player)
{
    return *this->ff::game::app_state_base::palette(player);
}

ff::signal_sink<>& retron::app_state::reload_resources_sink()
{
    return ff::game::app_state_base::reload_resources_sink();
}

retron::render_debug_t retron::app_state::render_debug() const
{
    return this->render_debug_;
}

void retron::app_state::render_debug(retron::render_debug_t flags)
{
    this->render_debug_ = flags;
}

retron::debug_cheats_t retron::app_state::debug_cheats() const
{
    return this->debug_cheats_;
}

void retron::app_state::debug_cheats(retron::debug_cheats_t flags)
{
    this->debug_cheats_ = flags;
}

void retron::app_state::debug_command(size_t command_id)
{
    return this->ff::game::app_state_base::debug_command(command_id);
}

std::shared_ptr<ff::state> retron::app_state::create_initial_game_state()
{
    auto title_state = std::make_shared<retron::title_state>();
    return std::make_shared<retron::transition_state>(nullptr, title_state, "transition_bg_1.png", 4, 24);
}

std::shared_ptr<ff::state> retron::app_state::create_debug_overlay_state()
{
    auto view = std::make_shared<ff::ui_view>(Noesis::MakePtr<retron::debug_page>());
    auto view_state = std::make_shared<retron::ui_view_state>(view);
    view->focused(true);
    return view_state;
}

void retron::app_state::save_settings(ff::dict& dict)
{
    dict.set_struct(strings::ID_GAME_OPTIONS, this->game_options_);
}

void retron::app_state::load_settings(const ff::dict& dict)
{
    if (!dict.get_struct(strings::ID_GAME_OPTIONS, this->game_options_) || this->game_options_.version != retron::game_options::CURRENT_VERSION)
    {
        this->game_options_ = retron::game_options();
    }
}

// Must be able to be called multiple times (whenever resources are hot reloaded)
void retron::app_state::load_resources()
{
    this->audio_ = std::make_unique<retron::audio>();
    this->game_spec_ = retron::game_spec::load();
    this->level_particle_value = ff::auto_resource_value("level_particles");
    this->level_particle_effects.clear();
}
