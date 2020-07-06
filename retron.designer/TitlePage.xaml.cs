using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace ReTron
{
    public enum GameDifficulty
    {
        Baby,
        Easy,
        Normal,
        Hard,
    }

    public class TitlePageViewModel : PropertyNotifier
    {
        public ICommand StartGameCommand => null;
        public ICommand OptionsCommand => null;
        public ICommand HighScoresCommand => null;
        public ICommand AboutCommand => null;

        public ICommand PlayersCommand => new DelegateCommand(() => this.ChangePlayers());
        public ICommand DifficultyCommand => new DelegateCommand(() => this.ChangeDifficulty());
        public ICommand SoundCommand => new DelegateCommand(() => this.ChangeSound());
        public ICommand FullScreenCommand => new DelegateCommand(() => this.ChangeFullScreen());

        private int players = 0;
        public string PlayersText => (this.players + 1).ToString();
        public void ChangePlayers(bool forward = true)
        {
            this.players = forward ? (this.players + 1) % 4 : (this.players + 3) % 4;
            this.OnPropertyChanged(nameof(this.PlayersText));
        }

        private GameDifficulty difficulty = GameDifficulty.Normal;
        public string DifficultyText => this.difficulty.ToString();
        public void ChangeDifficulty(bool forward = true)
        {
            this.difficulty = forward
                ? (this.difficulty == GameDifficulty.Hard)
                    ? GameDifficulty.Easy
                    : this.difficulty + 1
                : (this.difficulty == GameDifficulty.Easy)
                    ? GameDifficulty.Hard
                    : this.difficulty - 1;

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

        private void OnPlayersKeyDown(object sender, KeyEventArgs args)
        {
            if (args.Key == Key.Left || args.Key == Key.Right)
            {
                this.ViewModel.ChangePlayers(args.Key == Key.Right);
            }
        }

        private void OnDifficultyKeyDown(object sender, KeyEventArgs args)
        {
            if (args.Key == Key.Left || args.Key == Key.Right)
            {
                this.ViewModel.ChangeDifficulty(args.Key == Key.Right);
            }
        }

        private void OnSoundKeyDown(object sender, KeyEventArgs args)
        {
            if (args.Key == Key.Left || args.Key == Key.Right)
            {
                this.ViewModel.ChangeSound();
            }
        }

        private void OnFullScreenKeyDown(object sender, KeyEventArgs args)
        {
            if (args.Key == Key.Left || args.Key == Key.Right)
            {
                this.ViewModel.ChangeFullScreen();
            }
        }

        private void OnLoaded(object sender, RoutedEventArgs args)
        {
            this.startGameButton.Focus();
        }

        private void OnOptionMouseEnter(object sender, MouseEventArgs args)
        {
            ((FrameworkElement)sender).Focus();
        }
    }
}
