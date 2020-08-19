#include "pch.h"
#include "Core/Options.h"
#include "UI/Components.h"
#include "UI/DebugPage.xaml.h"
#include "UI/TitlePage.xaml.h"

void ReTron::RegisterNoesisComponents()
{
	Noesis::RegisterComponent<Noesis::EnumConverter<ReTron::GameFlags>>();
	Noesis::RegisterComponent<Noesis::EnumConverter<ReTron::GamePlayers>>();
	Noesis::RegisterComponent<Noesis::EnumConverter<ReTron::GameDifficulty>>();

	Noesis::RegisterComponent(Noesis::TypeOf<ReTron::DebugPage>(), nullptr);
	Noesis::RegisterComponent(Noesis::TypeOf<ReTron::DebugPageViewModel>(), nullptr);
	Noesis::RegisterComponent(Noesis::TypeOf<ReTron::TitlePage>(), nullptr);
	Noesis::RegisterComponent(Noesis::TypeOf<ReTron::TitlePageViewModel>(), nullptr);
}
