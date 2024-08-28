#include "pch.h"
#include "source/ui/set_panel_child_focus_action.h"

NS_IMPLEMENT_REFLECTION_(retron::set_panel_child_focus_action, "retron.set_panel_child_focus_action")

Noesis::Ptr<Noesis::Freezable> retron::set_panel_child_focus_action::CreateInstanceCore() const
{
    return *new set_panel_child_focus_action();
}

void retron::set_panel_child_focus_action::Invoke(Noesis::BaseComponent*)
{
    Noesis::UIElement* element = this->GetTarget();

    if (element)
    {
        Noesis::Panel* panel = Noesis::DynamicCast<Noesis::Panel*>(element->GetUIParent());
        if (panel && panel->GetChildren())
        {
            for (int i = 0; i < panel->GetChildren()->Count(); i++)
            {
                Noesis::UIElement* child = panel->GetChildren()->Get(static_cast<unsigned int>(i));
                if (child->GetIsFocused())
                {
                    // No need to change focus
                    element = nullptr;
                    break;
                }
            }
        }
    }

    if (element)
    {
        element->Focus();
    }
}
