using System;
using System.Globalization;
using System.Windows.Data;

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
}
