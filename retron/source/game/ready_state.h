#pragma once

namespace retron
{
    class game_service;
    class level_base;

    class ready_state : public ff::state
    {
    public:
        ready_state(const retron::game_service& game_service, std::shared_ptr<retron::level_base> level, std::shared_ptr<ff::state> under_state);

        // state
        virtual std::shared_ptr<ff::state> advance_time() override;
        virtual void render(ff::dxgi::command_context_base& context, ff::render_targets& targets) override;

    private:
        const retron::game_service& game_service;
        std::shared_ptr<retron::level_base> level;
        std::shared_ptr<ff::state> under_state;
        ff::auto_resource<ff::sprite_font> game_font;
        size_t counter;
    };
}
