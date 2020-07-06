using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace ReTron
{
    public class TitlePageViewModel : PropertyNotifier
    {
        public ICommand StartGameCommand => null;
        public ICommand OptionsCommand => null;
        public ICommand HighScoresCommand => null;
        public ICommand AboutCommand => null;

        public ICommand PlayersCommand => new DelegateCommand((object param) => this.ChangePlayers(param is bool forward && forward));
        public ICommand DifficultyCommand => new DelegateCommand((object param) => this.ChangeDifficulty(param is bool forward && forward));
        public ICommand SoundCommand => new DelegateCommand(() => this.ChangeSound());
        public ICommand FullScreenCommand => new DelegateCommand(() => this.ChangeFullScreen());

        private int players = 0;
        public string PlayersText => (this.players + 1).ToString();
        public void ChangePlayers(bool forward = true)
        {
            this.players = forward ? (this.players + 1) % 4 : (this.players + 3) % 4;
            this.OnPropertyChanged(nameof(this.PlayersText));
        }

        private int difficulty = 1;
        public string DifficultyText => this.difficulty.ToString();
        public void ChangeDifficulty(bool forward = true)
        {
            this.difficulty = (this.difficulty + (forward ? 1 : 2)) % 3;
            this.OnPropertyChanged(nameof(this.DifficultyText));
        }

        private bool sound = true;
        public string SoundText => this.sound ? "On" : "Off";
        public void ChangeSound()
        {
            this.sound = !this.sound;
            this.OnPropertyChanged(nameof(this.SoundText));
        }

        private bool fullScreen = true;
        public string FullScreenText => this.fullScreen ? "On" : "Off";
        public void ChangeFullScreen()
        {
            this.fullScreen = !this.fullScreen;
            this.OnPropertyChanged(nameof(this.FullScreenText));
        }
    }

    partial class TitlePage : UserControl
    {
        public TitlePageViewModel ViewModel { get; } = new TitlePageViewModel();

        public TitlePage()
        {
            this.InitializeComponent();
        }

        public ICommand FocusLeftRightCommand => new DelegateCommand((object param) =>
        {
            if (Keyboard.FocusedElement is Button button && button.Command != null)
            {
                button.Command.Execute(param);
            }
        });
    }
}
