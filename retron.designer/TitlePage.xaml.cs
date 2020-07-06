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
        public ICommand SoundCommand => new DelegateCommand(() => this.ChangeDifficulty());
        public ICommand FullScreenCommand => new DelegateCommand(() => this.ChangeDifficulty());

        private GameDifficulty difficulty = GameDifficulty.Normal;
        public string DifficultyText => this.difficulty.ToString();
        public GameDifficulty Difficulty
        {
            get => this.difficulty;
            set
            {
                if (this.SetPropertyValue(ref this.difficulty, value))
                {
                    this.OnPropertyChanged(nameof(this.DifficultyText));
                }
            }
        }

        public void ChangeDifficulty(bool forward = true)
        {
            if (forward)
            {
                this.Difficulty = (this.Difficulty == GameDifficulty.Hard)
                    ? GameDifficulty.Easy
                    : this.Difficulty + 1;
            }
            else
            {
                this.Difficulty = (this.Difficulty == GameDifficulty.Easy)
                    ? GameDifficulty.Hard
                    : this.Difficulty - 1;
            }
        }

        private int players = 0;
        public string PlayersText => (this.players + 1).ToString();
        public int Players
        {
            get => this.players;
            set
            {
                if (this.SetPropertyValue(ref this.players, value))
                {
                    this.OnPropertyChanged(nameof(this.PlayersText));
                }
            }
        }

        public void ChangePlayers(bool forward = true)
        {
            if (forward)
            {
                this.Players = (this.Players + 1) % 4;
            }
            else
            {
                this.Players = (this.Players + 3) % 4;
            }
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
