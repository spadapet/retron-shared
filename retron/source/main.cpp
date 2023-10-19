#include "pch.h"
#include "source/core/options.h"
#include "source/states/app_state.h"
#include "source/ui/debug_page.xaml.h"
#include "source/ui/particle_lab_page.xaml.h"
#include "source/ui/title_page.xaml.h"

namespace res
{
    void register_bonus();
    void register_controls();
    void register_electrode();
    void register_game_spec();
    void register_graphics();
    void register_particles();
    void register_player();
    void register_sprites();
    void register_xaml();
}

static std::shared_ptr<ff::game::app_state_base> create_app_state()
{
    return std::make_shared<retron::app_state>();
}

static void register_global_resources()
{
    ::res::register_bonus();
    ::res::register_controls();
    ::res::register_electrode();
    ::res::register_game_spec();
    ::res::register_graphics();
    ::res::register_particles();
    ::res::register_player();
    ::res::register_sprites();
    ::res::register_xaml();

    Noesis::RegisterComponent<Noesis::EnumConverter<retron::game_flags>>();
    Noesis::RegisterComponent<Noesis::EnumConverter<retron::game_players>>();
    Noesis::RegisterComponent<Noesis::EnumConverter<retron::game_difficulty>>();

    Noesis::RegisterComponent(Noesis::TypeOf<retron::debug_page>(), nullptr);
    Noesis::RegisterComponent(Noesis::TypeOf<retron::debug_page_view_model>(), nullptr);
    Noesis::RegisterComponent(Noesis::TypeOf<retron::particle_lab_page>(), nullptr);
    Noesis::RegisterComponent(Noesis::TypeOf<retron::particle_lab_page_view_model>(), nullptr);
    Noesis::RegisterComponent(Noesis::TypeOf<retron::title_page>(), nullptr);
    Noesis::RegisterComponent(Noesis::TypeOf<retron::title_page_view_model>(), nullptr);
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    ff::game::init_params_t<retron::app_state> params{};
    params.create_initial_state = ::create_app_state;
    params.register_global_resources = ::register_global_resources;

    return ff::game::run(params);
}
