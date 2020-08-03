using System.Windows.Controls;
using System.Windows.Input;

namespace ReTron
{
    public class DebugPageViewModel : PropertyNotifier
    {
        public ICommand RestartLevelCommand => null;
        public ICommand RestartGameCommand => null;
        public ICommand RebuildResourcesCommand => null;
        public ICommand CloseDebugCommand => null;
    }

    public partial class DebugPage : UserControl
    {
        public DebugPageViewModel ViewModel { get; } = new DebugPageViewModel();

        public DebugPage()
        {
            this.InitializeComponent();
        }
    }
}
