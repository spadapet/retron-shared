#include "pch.h"
#include "TitlePage.h"

NS_IMPLEMENT_REFLECTION(ReTron::TitlePage, "ReTron.TitlePage")
{
}

ReTron::TitlePage::TitlePage()
{
	Noesis::GUI::LoadComponent(this, "TitlePage.xaml");
}

ReTron::TitlePage::~TitlePage()
{
}

std::shared_ptr<ff::State> ReTron::TitlePage::GetPendingState() const
{
	return _pendingState;
}

bool ReTron::TitlePage::ConnectEvent(BaseComponent* source, const char* event, const char* handler)
{
	NS_CONNECT_EVENT(Noesis::Button, Click, OnClickPlayGame);
	return false;
}

void ReTron::TitlePage::OnClickPlayGame(Noesis::BaseComponent* sender, const Noesis::RoutedEventArgs& args)
{
	// _pendingState = std::make_shared<PlayGameState>(_appService, _appService->GetGameOptions());
}
