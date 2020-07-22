using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;
using System.Windows.Interactivity;

namespace ff
{
    public class BoolToVisibleConverter : IValueConverter
    {
        object IValueConverter.Convert(object value, Type targetType, object parameter, CultureInfo culture) => new NotImplementedException();
        object IValueConverter.ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) => new NotImplementedException();
    }

    public class BoolToCollapsedConverter : IValueConverter
    {
        object IValueConverter.Convert(object value, Type targetType, object parameter, CultureInfo culture) => new NotImplementedException();
        object IValueConverter.ConvertBack(object value, Type targetType, object parameter, CultureInfo culture) => new NotImplementedException();
    }

    public class SetPanelChildFocusAction : TargetedTriggerAction<UIElement>
    {
        protected override void Invoke(object parameter)
        {
            _ = this.Target?.Focus();
        }
    }
}
