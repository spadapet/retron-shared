#include "pch.h"
#include "App.xaml.h"
#include "States/AppState.h"
#include "Thread/ThreadDispatch.h"
#include "UI/FailurePage.xaml.h"

ReTron::App::App()
{
	InitializeComponent();
}

ReTron::App::~App()
{
	_globals.Shutdown();
	_processGlobals.Shutdown();
}

void ReTron::App::OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ args)
{
	if (!_processGlobals.IsValid())
	{
		_processGlobals.Startup();

		Platform::WeakReference weakThis(this);
		ff::GetMainThreadDispatch()->Post([weakThis]()
			{
				weakThis.Resolve<App>()->InitializeGlobals();
			});
	}
}

void ReTron::App::InitializeGlobals()
{
	auto panel = ref new Windows::UI::Xaml::Controls::SwapChainPanel();
	auto page = ref new Windows::UI::Xaml::Controls::Page();
	page->Content = panel;

	auto window = Windows::UI::Xaml::Window::Current;
	window->Content = page;

	_appState = std::make_shared<ReTron::AppState>();

	if (!_globals.Startup(ff::AppGlobalsFlags::All, panel, _appState.get()))
	{
		window->Content = ref new ReTron::FailurePage(&_globals);
	}

	window->Activate();
}
