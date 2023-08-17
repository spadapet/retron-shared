#include "pch.h"
#include "source/core/app_service.h"
#include "source/core/options.h"
#include "source/states/app_state.h"
#include "source/ui/debug_page.xaml.h"
#include "source/ui/particle_lab_page.xaml.h"
#include "source/ui/title_page.xaml.h"

static const std::string_view NOESIS_NAME = "d704047b-5bd2-4757-9858-6a7d86cdd006";
static const std::string_view NOESIS_KEY = "Dkpz5K4Gc+RCr+UyeqnA2MuDFIA7HzAWsAv2vEG1dl4vQ2Ob";

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

static ff::game::init_params get_game_init_params()
{
    ff::game::init_params params{};
    params.create_initial_state = ::create_app_state;
    params.noesis_application_resources_name = "application_resources.xaml";
    params.noesis_license_key = ::NOESIS_KEY;
    params.noesis_license_name = ::NOESIS_NAME;
    params.register_global_resources = ::register_global_resources;

    return params;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    ff::game::init_params init_params = ::get_game_init_params();
    return ff::game::run(init_params);
}
