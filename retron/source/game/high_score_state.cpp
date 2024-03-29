#include "pch.h"
#include "source/game/high_score_state.h"

retron::high_score_state::high_score_state(const retron::game_options& game_options, const retron::player& player, std::shared_ptr<ff::state> next_state)
    : game_options(game_options)
    , player(player.self_or_coop())
    , next_state(next_state)
{}

std::shared_ptr<ff::state> retron::high_score_state::advance_time()
{
    return this->next_state;
}

void retron::high_score_state::render(ff::dxgi::command_context_base& context, ff::render_targets& targets)
{
    this->next_state->render(context, targets);
}
