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
        private GameDifficulty difficulty = GameDifficulty.Normal;
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

        public string DifficultyText => this.difficulty.ToString();

        public ICommand DifficultyCommand => new DelegateCommand(() => this.ChangeDifficulty());

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
    }

    partial class TitlePage : UserControl
    {
        public TitlePageViewModel ViewModel { get; } = new TitlePageViewModel();

        public TitlePage()
        {
            this.InitializeComponent();
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
            this.difficultyButton.Focus();
        }

        private void OnDifficultyMouseEnter(object sender, MouseEventArgs args)
        {
            ((Button)sender).Focus();
        }
    }
}
