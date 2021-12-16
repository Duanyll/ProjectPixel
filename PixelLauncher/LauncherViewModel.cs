using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;
using System.IO;

using Microsoft.Win32;
using Newtonsoft.Json;

namespace PixelLauncher
{
    public class LevelConfig
    {
        public int version;
        public string displayName { get; set; }
        public string description { get; set; }
        public string terrainType { get; set; }
        public int xSize { get; set; }
        public int ySize { get; set; }
        public int zSize { get; set; }
        public string goal { get; set; }
}
    public class Level
    {
        public string DisplayName { get; set; } = "";
        public string Path { get; set; } = "";
        public LevelConfig Config { get; set; }
    }

    public class LauncherViewModel : BaseViewModel
    {
        public ICommand LaunchGameCommand { get; set; }
        public ICommand LoadLevelsCommand { get; set; }
        public ICommand SaveSettingsCommand { get; set; }
        public ObservableCollection<Level> Levels { get; set; } = new ObservableCollection<Level>();
        Level? selectedLevel = null;
        public Level? SelectedLevel
        {
            get { return selectedLevel; }
            set
            {
                SetProperty(ref selectedLevel, value);
                OnPropertyChanged(nameof(LevelDetailVisibility));
            }
        }

        public LauncherViewModel()
        {
            var uiThread = Dispatcher.CurrentDispatcher;

            LaunchGameCommand = new CommandHandler(() =>
            {
                var level = selectedLevel;
                if (level == null) return;
                Task.Run(() =>
                {
                    var game = new Process();
                    game.StartInfo.FileName = GameExecutablePath;
                    game.StartInfo.WorkingDirectory = GameWorkingDirectory;
                    game.StartInfo.Arguments = level.Path;
                    game.StartInfo.CreateNoWindow = true;
                    uiThread.Invoke(() => IsGameRunning = true);
                    game.Start();
                    game.WaitForExit();
                    uiThread.Invoke(() => IsGameRunning = false);
                });
            }, () => !isGameRunning && selectedLevel != null);

            LoadLevelsCommand = new CommandHandler(() =>
            {
                SelectedLevel = null;
                Levels.Clear();      
                var levelFiles = Directory.EnumerateFiles(System.IO.Path.Combine(gameWorkingDirectory, "levels"), "*.json");
                foreach (var levelFile in levelFiles)
                {
                    try
                    {
                        var content = File.ReadAllText(levelFile);
                        var config = JsonConvert.DeserializeObject<LevelConfig>(content);
                        if (config != null)
                        {
                            Levels.Add(new Level()
                            {
                                DisplayName = config.displayName,
                                Path = "levels/" + System.IO.Path.GetFileName(levelFile),
                                Config = config
                            });
                        }
                    } catch (Exception ex)
                    {
                        Debug.WriteLine(ex.Message);
                    }
                }
            }, () => true);

            SaveSettingsCommand = new CommandHandler(() =>
            {
                Settings1.Default.GameExecutablePath = GameExecutablePath;
                Settings1.Default.GameWorkingDirectory = GameWorkingDirectory;
                Settings1.Default.Save();
                LoadLevelsCommand.Execute(null);
            }, () => true);

            BrowseGameExecultableCommand = new CommandHandler(() =>
            {
                OpenFileDialog dialog = new OpenFileDialog();
                dialog.Filter = "ProjectPixel.exe|ProjectPixel.exe";
                if (dialog.ShowDialog() == true)
                {
                    GameExecutablePath = dialog.FileName;
                }
            }, () => true);

            BrowseWorkingDirectoryCommand = new CommandHandler(() =>
            {
                using (var dialog = new System.Windows.Forms.FolderBrowserDialog())
                {
                    System.Windows.Forms.DialogResult result = dialog.ShowDialog();
                    if (result == System.Windows.Forms.DialogResult.OK)
                    {
                        GameWorkingDirectory = dialog.SelectedPath;
                    }
                }
            }, () => true);
        }

        bool isGameRunning = false;
        public bool IsGameRunning
        {
            get { return isGameRunning; }
            set { SetProperty(ref isGameRunning, value); }
        }

        string gameExecutablePath = Settings1.Default.GameExecutablePath;
        public string GameExecutablePath
        {
            get { return gameExecutablePath; }
            set { SetProperty(ref gameExecutablePath, value); }
        }

        public ICommand BrowseGameExecultableCommand { get; set; }

        string gameWorkingDirectory = Settings1.Default.GameWorkingDirectory;
        public string GameWorkingDirectory
        {
            get { return gameWorkingDirectory; }
            set { SetProperty(ref gameWorkingDirectory, value); }
        }
        public ICommand BrowseWorkingDirectoryCommand { get; set; }

        public Visibility LevelDetailVisibility { get => selectedLevel != null ? Visibility.Visible : Visibility.Collapsed; }
    }
}
