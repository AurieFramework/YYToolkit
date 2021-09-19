using System;
using System.Windows.Forms;
using System.Diagnostics;
using System.Net;
using System.IO;
using System.Threading;
using System.Runtime.InteropServices;
using Launcher;

namespace Launcher
{
    public partial class MainWindow : Form
    {
        private string sRunnerFilePath; // Fully qualified file path towards selected game executable
        private string sRunnerFileName; // File name of the game executable
        private string sDataFilePath; // Fully qualified file path towards selected data file - empty = default

        public MainWindow()
        {
            InitializeComponent();
        }

        private void btRunnerPick_Click(object sender, EventArgs e)
        {
            using (OpenFileDialog fileDialog = Utils.CreateFileDialog("%systemdrive%", "Find the game executable", "Executables|*.exe", 1))
            {
                if (fileDialog.ShowDialog() == DialogResult.OK)
                {
                    sRunnerFilePath = fileDialog.FileName;
                    sRunnerFileName = fileDialog.SafeFileName;

                    Directory.SetCurrentDirectory(fileDialog.FileName.Substring(0, fileDialog.FileName.LastIndexOf("\\")));

                    txtRunner.ResetText();
                    txtRunner.AppendText(sRunnerFilePath);
                }
            }
        }

        private void btYYTKLaunch_Click(object sender, EventArgs e)
        {
            string TempPath = Environment.ExpandEnvironmentVariables("%temp%\\YYToolkit.dll");

            if (string.IsNullOrEmpty(sRunnerFileName))
            {
                MessageBox.Show("Please select the game executable first!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            
            using (var Browser = new WebClient())
            {
                if (File.Exists(TempPath))
                    File.Delete(TempPath);

                Browser.DownloadFile("https://github.com/Archie-osu/YYToolkit/releases/latest/download/YYToolkit.dll", TempPath);
            }

            Process p;

            if (!string.IsNullOrEmpty(sDataFilePath))
                p = Process.Start(sRunnerFilePath, "-game \"" + sDataFilePath + "\"");
            else
                p = Process.Start(sRunnerFilePath);

            while (string.IsNullOrEmpty(p.MainWindowTitle))
            {
                Thread.Sleep(500);
                p.Refresh();
            }

            Utils.Inject(p, TempPath);
        }

        private void btNoModLaunch_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(sRunnerFileName))
            {
                MessageBox.Show("Please select the game executable first!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            if (!string.IsNullOrEmpty(sDataFilePath))
                Process.Start(sRunnerFilePath, "-game \"" + sDataFilePath + "\"");
            else
                Process.Start(sRunnerFilePath);
        }

        private void btResetRunner_Click(object sender, EventArgs e)
        {
            txtRunner.Text = "<none selected>";
            sRunnerFileName = "";
            sRunnerFilePath = "";
        }

        private void btResetData_Click(object sender, EventArgs e)
        {
            txtDataFile.Text = "<use default>";
            sDataFilePath = "";
        }

        private void btDataPick_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(sRunnerFileName))
            {
                MessageBox.Show("Please select the game executable first!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            using (OpenFileDialog fileDialog = Utils.CreateFileDialog("%systemdrive%", "Find the data.win file", "GameMaker Data Files|*.win", 1))
            {
                if (fileDialog.ShowDialog() == DialogResult.OK)
                {
                    sDataFilePath = fileDialog.FileName;

                    txtDataFile.ResetText();
                    txtDataFile.AppendText(sDataFilePath);
                }
            }
        }

        private void btOpenPluginFolder_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(sRunnerFileName))
            {
                MessageBox.Show("Please select the game executable first!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            try
            {
                if (!Directory.Exists("autoexec"))
                    Directory.CreateDirectory("autoexec");
            }
            catch (System.UnauthorizedAccessException Exception)
            {
                MessageBox.Show(Exception.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }


            Process.Start("explorer", "\"" + Directory.GetCurrentDirectory() + "\\autoexec\"");
        }

        private void btOpenGitHub_Click(object sender, EventArgs e)
        {
            Process.Start("explorer.exe", $"\"https://github.com/Archie-osu/YYToolkit\"");
        }

        private void btDownloadSrc_Click(object sender, EventArgs e)
        {
            using (var FolderBrowser = new FolderBrowserDialog())
            {
                FolderBrowser.Description = "Where do you want to save the source tree?";

                if (FolderBrowser.ShowDialog() == DialogResult.OK)
                {
                    using (var Browser = new WebClient())
                    {
                        try
                        {
                            if (rbStable.Checked)
                                Browser.DownloadFile("https://github.com/Archie-osu/YYToolkit/archive/refs/heads/stable.zip", FolderBrowser.SelectedPath + "\\YYToolkit-stable.zip");
                            else
                                Browser.DownloadFile("https://github.com/Archie-osu/YYToolkit/archive/refs/heads/beta.zip", FolderBrowser.SelectedPath + "\\YYToolkit-beta.zip");
                        }
                        catch (System.Exception Exception)
                        {
                            MessageBox.Show(Exception.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                            return;
                        }
                    }
                    MessageBox.Show("Download complete.", "Source code download", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
            }
        }
    }
}