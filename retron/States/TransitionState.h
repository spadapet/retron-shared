#pragma once

#include "Resource/ResourceValue.h"
#include "State/State.h"

namespace ff
{
	class ITexture;
}

namespace ReTron
{
	class IAppService;
	class IGameService;
	class TitlePage;

	class TransitionState : public ff::State
	{
	public:
		TransitionState(IAppService* appService, std::shared_ptr<ff::State> oldState, std::shared_ptr<ff::State> newState, ff::StringRef imageResource);

		virtual std::shared_ptr<ff::State> Advance(ff::AppGlobals* globals) override;
		virtual void Render(ff::AppGlobals* globals, ff::IRenderTarget* target, ff::IRenderDepth* depth) override;

	private:
		IAppService* _appService;
		std::shared_ptr<ff::State> _oldState;
		std::shared_ptr<ff::State> _newState;
		ff::TypedResource<ff::ITexture> _image;
		ff::ComPtr<ff::ITexture> _texture;
		ff::ComPtr<ff::ITexture> _texture2;
		ff::ComPtr<ff::IRenderTarget> _target;
		ff::ComPtr<ff::IRenderTarget> _target2;
		ff::ComPtr<ff::IRenderDepth> _depth;
		ff::FixedInt _counter;
	};
}
