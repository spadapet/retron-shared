#pragma once

#include "source/core/app_service.h"
#include "source/core/game_spec.h"
#include "source/core/options.h"
#include "source/core/particles.h"

namespace retron
{
    class app_state : public ff::game::app_state_base, public retron::app_service
    {
    public:
        app_state();
        virtual ~app_state() override;

        // ff::game::app_state_base
        virtual bool allow_debug_commands();

        // retron::app_service
        virtual retron::audio& audio() override;
        virtual const retron::particle_effect_base* level_particle_effect(std::string_view name) override;
        virtual const ff::game::system_options& system_options() const override;
        virtual const retron::game_options& default_game_options() const override;
        virtual const retron::game_spec& game_spec() const override;
        virtual void system_options(const ff::game::system_options& options) override;
        virtual void default_game_options(const retron::game_options& options) override;
        virtual ff::dxgi::palette_base* palette() override;
        virtual ff::dxgi::palette_base& player_palette(size_t player) override;
        virtual ff::signal_sink<>& reload_resources_sink() override;
        virtual retron::render_debug_t render_debug() const override;
        virtual void render_debug(retron::render_debug_t flags) override;
        virtual retron::debug_cheats_t debug_cheats() const override;
        virtual void debug_cheats(retron::debug_cheats_t flags) override;
        virtual void debug_command(size_t command_id) override;

    protected:
        // ff::game::app_state_base
        virtual std::shared_ptr<ff::state> create_initial_game_state() override;
        virtual std::shared_ptr<ff::state> create_debug_overlay_state() override;
        virtual void save_settings(ff::dict& dict) override;
        virtual void load_settings(const ff::dict& dict) override;
        virtual void load_resources() override;
        virtual bool debug_command_override(size_t command_id) override;

    private:
        // Globals
        retron::game_options game_options_;
        retron::game_spec game_spec_;

        // Resources
        std::unique_ptr<retron::audio> audio_;
        std::unordered_map<std::string_view, retron::particles::effect_t> level_particle_effects;
        ff::auto_resource_value level_particle_value;;

        // Debugging
        retron::render_debug_t render_debug_;
        retron::debug_cheats_t debug_cheats_;
    };
}
