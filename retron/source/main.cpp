#include "pch.h"
#include "source/core/options.h"
#include "source/states/app_state.h"
#include "source/ui/debug_page.xaml.h"
#include "source/ui/particle_lab_page.xaml.h"
#include "source/ui/set_panel_child_focus_action.h"
#include "source/ui/title_page.xaml.h"

static std::shared_ptr<ff::game::app_state_base> create_app_state()
{
    return std::make_shared<retron::app_state>();
}

static void register_global_resources()
{
    verify(ff::global_resources::add_files(ff::filesystem::executable_path().parent_path()));

    Noesis::RegisterComponent<Noesis::EnumConverter<retron::game_flags>>();
    Noesis::RegisterComponent<Noesis::EnumConverter<retron::game_players>>();
    Noesis::RegisterComponent<Noesis::EnumConverter<retron::game_difficulty>>();

    Noesis::RegisterComponent(Noesis::TypeOf<retron::debug_page>(), nullptr);
    Noesis::RegisterComponent(Noesis::TypeOf<retron::debug_page_view_model>(), nullptr);
    Noesis::RegisterComponent(Noesis::TypeOf<retron::particle_lab_page>(), nullptr);
    Noesis::RegisterComponent(Noesis::TypeOf<retron::particle_lab_page_view_model>(), nullptr);
    Noesis::RegisterComponent(Noesis::TypeOf<retron::title_page>(), nullptr);
    Noesis::RegisterComponent(Noesis::TypeOf<retron::title_page_view_model>(), nullptr);

    Noesis::RegisterComponent<retron::set_panel_child_focus_action>();
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    ff::game::init_params_t<retron::app_state> params{};
    params.create_initial_state = ::create_app_state;
    params.register_global_resources = ::register_global_resources;

    return ff::game::run(params);
}
