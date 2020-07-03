#pragma once

namespace ff
{
	class State;
}

namespace ReTron
{
	class TitlePage : public Noesis::UserControl
	{
	public:
		TitlePage();
		virtual ~TitlePage() override;

		std::shared_ptr<ff::State> GetPendingState() const;

		virtual bool ConnectEvent(BaseComponent* source, const char* event, const char* handler) override;

	private:
		void OnClickPlayGame(Noesis::BaseComponent* sender, const Noesis::RoutedEventArgs& args);

		std::shared_ptr<ff::State> _pendingState;

		NS_DECLARE_REFLECTION(TitlePage, Noesis::UserControl);
	};
}
