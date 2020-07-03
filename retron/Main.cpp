#include "pch.h"
#include "MainUtilInclude.h"
#include "Module/Module.h"
#include "String/StringUtil.h"

// {58BC855E-352F-4314-AF08-75C87C2F00DB}
static const GUID MODULE_ID = { 0x58bc855e, 0x352f, 0x4314, { 0xaf, 0x8, 0x75, 0xc8, 0x7c, 0x2f, 0x0, 0xdb } };
static ff::StaticString MODULE_NAME(L"ReTron");
static ff::ModuleFactory RegisterThisModule(MODULE_NAME, MODULE_ID, ff::GetMainInstance, ff::GetModuleStartup);

#if METRO_APP

#include "App.xaml.h"

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^ args)
{
	ff::SetMainModule(MODULE_NAME, MODULE_ID, ff::GetMainInstance());

	auto callbackFunc = [](Windows::UI::Xaml::ApplicationInitializationCallbackParams^ args)
	{
		auto app = ref new ::ReTron::App();
	};

	auto callback = ref new Windows::UI::Xaml::ApplicationInitializationCallback(callbackFunc);
	Windows::UI::Xaml::Application::Start(callback);

	return 0;
}

#else

#include "Globals/DesktopGlobals.h"
#include "States/AppState.h"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR commandLine, int showCommand)
{
	ff::SetMainModule(MODULE_NAME, MODULE_ID, instance);

	std::shared_ptr<ff::IAppGlobalsHelper> state = std::make_shared<ReTron::AppState>();
	return ff::DesktopGlobals::RunWithWindow(ff::AppGlobalsFlags::All, state) ? 0 : 1;
}

#endif
