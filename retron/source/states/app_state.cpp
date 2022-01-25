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

ff::dxgi::draw_ptr retron::app_service::begin_palette_draw()
{
    retron::render_targets& targets = *retron::app_service::get().render_targets();
    ff::dxgi::target_base& target = *targets.target(retron::render_target_types::palette_1);
    ff::dxgi::depth_base& depth = *targets.depth(retron::render_target_types::palette_1);

    return ff::dxgi_client().global_draw_device().begin_draw(target, &depth, retron::constants::RENDER_RECT, retron::constants::RENDER_RECT);
}

retron::app_state::app_state()
    : viewport(ff::point_size(constants::RENDER_WIDTH, constants::RENDER_HEIGHT))
    , render_debug_(retron::render_debug_t::none)
    , debug_cheats_(retron::debug_cheats_t::none)
    , texture_1080(std::make_shared<ff::texture>(ff::dxgi_client().create_render_texture(retron::constants::RENDER_SIZE_HIGH.cast<size_t>(), DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, &ff::dxgi::color_black())))
    , target_1080(ff::dxgi_client().create_target_for_texture(this->texture_1080->dxgi_texture()))
{
    assert(!::app_service);
    ::app_service = this;

    this->render_targets_stack.push_back(nullptr);
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

bool retron::app_state::allow_debug()
{
    return this->game_spec_.allow_debug();
}

void retron::app_state::debug_command(size_t command_id)
{
    if (this->allow_debug())
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
    if (this->allow_debug())
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

void retron::app_state::render(ff::dxgi::target_base& target, ff::dxgi::depth_base& depth)
{
    this->target_1080->clear(ff::dxgi_client().frame_context(), ff::dxgi::color_black());

    this->push_render_targets(this->render_targets_);
    ff::state::render();
    this->pop_render_targets(*this->target_1080);

    // Draw to final target, with black bars
    {
        ff::point_size target_size = target.size().logical_pixel_size;
        ff::rect_float target_rect = this->viewport.view(target_size).cast<float>();
        ff::point_float target_scale = target_rect.size() / retron::constants::RENDER_SIZE_HIGH.cast<float>();
        ff::rect_float full_target({}, target_size.cast<float>());

        ff::dxgi::draw_ptr draw = ff::dxgi_client().global_draw_device().begin_draw(target, nullptr, full_target, full_target);
        if (draw)
        {
            draw->push_sampler_linear_filter(true);

            if (target_rect.left > 0)
            {
                draw->draw_filled_rectangle(ff::rect_float(full_target.top_left(), target_rect.bottom_left()), ff::dxgi::color_black());
                draw->draw_filled_rectangle(ff::rect_float(target_rect.top_right(), full_target.bottom_right()), ff::dxgi::color_black());
            }
            else
            {
                draw->draw_filled_rectangle(ff::rect_float(full_target.top_left(), target_rect.top_right()), ff::dxgi::color_black());
                draw->draw_filled_rectangle(ff::rect_float(target_rect.bottom_left(), full_target.bottom_right()), ff::dxgi::color_black());
            }

            draw->draw_sprite(this->texture_1080->sprite_data(), ff::dxgi::transform(target_rect.top_left(), target_scale));
        }
    }
}

void retron::app_state::frame_rendered(ff::state::advance_t type, ff::dxgi::target_base& target, ff::dxgi::depth_base& depth)
{
    ff::rect_float viewport = this->viewport.last_view().cast<float>();

    for (ff::ui_view* view : ff::ui::rendered_views())
    {
        view->viewport(viewport);
    }

    ff::state::frame_rendered(type, target, depth);
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

retron::render_targets* retron::app_state::render_targets() const
{
    assert(this->render_targets_stack.size() > 1);
    return this->render_targets_stack.back();
}

void retron::app_state::push_render_targets(retron::render_targets& targets)
{
    targets.clear();
    return this->render_targets_stack.push_back(&targets);
}

void retron::app_state::pop_render_targets(ff::dxgi::target_base& final_target)
{
    assert(this->render_targets_stack.size() > 1);
    this->render_targets_stack.back()->render(final_target);
    this->render_targets_stack.pop_back();
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
