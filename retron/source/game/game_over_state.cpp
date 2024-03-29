#include "pch.h"
#include "source/core/app_service.h"
#include "source/game/game_over_state.h"

retron::game_over_state::game_over_state(std::shared_ptr<ff::state> under_state)
    : under_state(under_state)
    , game_font("game_font")
{}

std::shared_ptr<ff::state> retron::game_over_state::advance_time()
{
    return nullptr;
}

void retron::game_over_state::render(ff::dxgi::command_context_base& context, ff::render_targets& targets)
{
    this->under_state->render(context, targets);

    ff::dxgi::draw_ptr draw = retron::app_service::begin_palette_draw(context, targets);
    if (draw)
    {
        std::string_view text = "GAME OVER";

        const ff::point_float scale(2, 2);
        ff::point_float size = this->game_font->measure_text(text, scale);

        this->game_font->draw_text(draw, text,
            ff::dxgi::transform(retron::constants::RENDER_RECT.center().cast<float>() - size / 3.0f, scale, 0, ff::dxgi::palette_index_to_color(retron::colors::ACTIVE_STATUS)),
            ff::dxgi::palette_index_to_color(224));
    }
}
