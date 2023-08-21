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
    : render_debug_(retron::render_debug_t::none)
    , debug_cheats_(retron::debug_cheats_t::none)
    , render_target(retron::constants::RENDER_SIZE.cast<size_t>(), &ff::dxgi::color_black(), 224)
{
    assert(!::app_service);
    ::app_service = this;
}

retron::app_state::~app_state()
{
    assert(::app_service == this);
    ::app_service = nullptr;
}

ff::dxgi::palette_base* retron::app_state::palette()
{
    return &this->player_palette(0);
}

bool retron::app_state::clear_color(DirectX::XMFLOAT4& color)
{
    color = ff::dxgi::color_black();
    return true;
}

bool retron::app_state::allow_debug_commands()
{
    return ff::game::app_state_base::allow_debug_commands() || this->game_spec_.allow_debug();
}

void retron::app_state::debug_command(size_t command_id)
{
    if (this->allow_debug_commands())
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
        else
        {
            ff::game::app_state_base::debug_command(command_id);
        }
    }
}

std::shared_ptr<ff::state> retron::app_state::advance_time()
{
    for (auto& i : this->player_palettes)
    {
        if (i)
        {
            i->advance();
        }
    }

    return ff::game::app_state_base::advance_time();
}

void retron::app_state::advance_input()
{
    if (this->allow_debug_commands())
    {
        if (!this->debug_input_events)
        {
            this->debug_input_events = std::make_unique<ff::input_event_provider>(*this->debug_input_mapping.object(),
                std::vector<const ff::input_vk*>{ &ff::input::keyboard(), & ff::input::pointer() });
        }

        if (this->debug_input_events->advance())
        {
            if (this->debug_input_events->event_hit(input_events::ID_DEBUG_RENDER_TOGGLE))
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

            if (this->debug_input_events->event_hit(input_events::ID_DEBUG_INVINCIBLE_TOGGLE))
            {
                this->debug_cheats_ = ff::flags::toggle(this->debug_cheats_, retron::debug_cheats_t::invincible);
            }

            if (this->debug_input_events->event_hit(input_events::ID_DEBUG_COMPLETE_LEVEL))
            {
                this->debug_cheats_ = ff::flags::set(this->debug_cheats_, retron::debug_cheats_t::complete_level);
            }

            if (this->debug_input_events->event_hit(input_events::ID_SHOW_CUSTOM_DEBUG))
            {
                this->debug_command(ff::game::app_state_base::ID_DEBUG_SHOW_UI);
            }
        }
    }

    ff::game::app_state_base::advance_input();
}

void retron::app_state::render(ff::dxgi::command_context_base& context, ff::render_targets& targets)
{
    size_t old_ui_view_count = ff::ui::rendered_views().size();

    targets.push(this->render_target);
    ff::game::app_state_base::render(context, targets);
    ff::rect_float target_rect = targets.pop(context, nullptr, this->palette());

    for (auto i = ff::ui::rendered_views().cbegin() + old_ui_view_count; i != ff::ui::rendered_views().end(); i++)
    {
        (*i)->viewport(target_rect);
    }
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

ff::dxgi::palette_base& retron::app_state::player_palette(size_t player)
{
    return *this->player_palettes[player];
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
    this->debug_input_events.reset();
    this->debug_input_mapping = "game_debug_controls";
    this->palette_data = "palette_main";
    this->level_particle_value = ff::auto_resource_value("level_particles");
    this->level_particle_effects.clear();

    for (size_t i = 0; i < this->player_palettes.size(); i++)
    {
        std::ostringstream str;
        str << "player_" << i;
        this->player_palettes[i] = std::make_shared<ff::palette_cycle>(this->palette_data.object(), str.str().c_str(), ::PALETTE_CYCLES_PER_SECOND);
    }
}
