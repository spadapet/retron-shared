#pragma once

namespace retron
{
	class transition_state : public ff::state
	{
	public:
		transition_state(std::shared_ptr<ff::state> old_state, std::shared_ptr<ff::state> new_state, std::string_view image_resource, size_t speed = 4, size_t vertical_pixel_stop = 0);

        virtual std::shared_ptr<ff::state> advance_time() override;
        virtual void render(ff::dxgi::command_context_base& context, ff::render_targets& targets) override;

	private:
        ff::render_target temp_target;
		std::shared_ptr<ff::state> old_state;
		std::shared_ptr<ff::state> new_state;
		ff::auto_resource<ff::texture> image;
		std::shared_ptr<ff::texture> texture;
		std::shared_ptr<ff::texture> texture2;
		std::shared_ptr<ff::dxgi::target_base> target;
		std::shared_ptr<ff::dxgi::target_base> target2;
		ff::fixed_int counter;
		ff::fixed_int speed;
		ff::fixed_int offset_stop;
	};
}
