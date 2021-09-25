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

            ServicePointManager.SecurityProtocol = SecurityProtocolType.Tls12; // Fix for Windows 7 downloads

            ContextMenu PluginManagerContextMenu = new ContextMenu();
            PluginManagerContextMenu.MenuItems.Add("Install plugin", new EventHandler(PluginContextMenu_InstallPlugin));
            PluginManagerContextMenu.MenuItems.Add("Uninstall plugin", new EventHandler(PluginContextMenu_UninstallPlugin));
            PluginManagerContextMenu.MenuItems.Add("Enable plugin", new EventHandler(PluginContextMenu_EnablePlugin));
            PluginManagerContextMenu.MenuItems.Add("Disable plugin", new EventHandler(PluginContextMenu_DisablePlugin));
            PluginManagerContextMenu.MenuItems.Add("Refresh list", new EventHandler(PluginContextMenu_Refresh));

            listPlugins.ContextMenu = PluginManagerContextMenu;

            ContextMenu btYYTKLaunchContextMenu = new ContextMenu();
            btYYTKLaunchContextMenu.MenuItems.Add("Launch with custom DLL", new EventHandler(btYYTKLaunchContextMenu_LaunchCustom));

            btYYTKLaunch.ContextMenu = btYYTKLaunchContextMenu;
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

                    listPlugins.Items.Clear();
                    listPlugins.Items.AddRange(Utils.GetPluginsFromGameDirectory(Directory.GetCurrentDirectory()));
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
                try
                {
                    Browser.DownloadFile("https://github.com/Archie-osu/YYToolkit/releases/latest/download/YYToolkit.dll", TempPath);
                }
                catch (System.Exception exception)
                {
                    MessageBox.Show("Couldn't auto-update!\n" + exception.Message, "Error", MessageBoxButtons.OKCancel, MessageBoxIcon.Error);
                }
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

        private void PluginContextMenu_InstallPlugin(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(sRunnerFileName))
            {
                MessageBox.Show("Please select the game executable first!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            using (OpenFileDialog fileDialog = Utils.CreateFileDialog("%systemdrive%", "Find the plugin file", "YYToolkit plugins|*.dll", 1))
            {
                if (fileDialog.ShowDialog() == DialogResult.OK)
                {
                    if (fileDialog.SafeFileName.ToLower() == "yytoolkit.dll")
                    {
                        if (MessageBox.Show("YYToolkit.dll might be the core module, and not a plugin. Add it anyway?", "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No)
                            return;
                    }

                    try
                    {
                        if (!Directory.Exists("autoexec"))
                            Directory.CreateDirectory("autoexec");

                        File.Copy(fileDialog.FileName, Directory.GetCurrentDirectory() + "\\autoexec\\" + fileDialog.SafeFileName);
                    }
                    catch (System.Exception Exception)
                    {
                        MessageBox.Show(Exception.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    }

                    // Refresh plugins
                    listPlugins.Items.Clear();
                    listPlugins.Items.AddRange(Utils.GetPluginsFromGameDirectory(Directory.GetCurrentDirectory()));
                }
            }
        }

        private void PluginContextMenu_UninstallPlugin(object sender, EventArgs e)
        {
            if (!IsReadyToManagePlugins(sRunnerFileName, listPlugins))
                return;

            try
            {
                File.Delete(Directory.GetCurrentDirectory() + "\\autoexec\\" + listPlugins.SelectedItem.ToString());
            }
            catch (System.Exception Exception)
            {
                MessageBox.Show(Exception.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            // Refresh plugins
            listPlugins.Items.Clear();
            listPlugins.Items.AddRange(Utils.GetPluginsFromGameDirectory(Directory.GetCurrentDirectory()));
        }

        private void PluginContextMenu_EnablePlugin(object sender, EventArgs e)
        {
            if (!IsReadyToManagePlugins(sRunnerFileName, listPlugins))
                return;

            var PluginName = listPlugins.SelectedItem.ToString();

            if (!PluginName.EndsWith(".dll.disabled"))
            {
                MessageBox.Show("Please pick a disabled plugin.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            var PluginPath = Directory.GetCurrentDirectory() + "\\autoexec\\" + PluginName;

            try
            {
                File.Move(PluginPath, PluginPath.Substring(0, PluginPath.LastIndexOf(".disabled")));
            }
            catch (System.Exception Exception)
            {
                MessageBox.Show(Exception.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            // Refresh plugins
            listPlugins.Items.Clear();
            listPlugins.Items.AddRange(Utils.GetPluginsFromGameDirectory(Directory.GetCurrentDirectory()));
        }

        private void PluginContextMenu_DisablePlugin(object sender, EventArgs e)
        {
            if (!IsReadyToManagePlugins(sRunnerFileName, listPlugins))
                return;

            var PluginName = listPlugins.SelectedItem.ToString();

            if (PluginName.EndsWith(".dll.disabled"))
            {
                MessageBox.Show("Please pick an enabled plugin.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            var PluginPath = Directory.GetCurrentDirectory() + "\\autoexec\\" + PluginName;

            try
            {
                File.Move(PluginPath, PluginPath + ".disabled");
            }
            catch (System.Exception Exception)
            {
                MessageBox.Show(Exception.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            // Refresh plugins
            listPlugins.Items.Clear();
            listPlugins.Items.AddRange(Utils.GetPluginsFromGameDirectory(Directory.GetCurrentDirectory()));
        }

        private void PluginContextMenu_Refresh(object sender, EventArgs e)
        {
            listPlugins.Items.Clear();
            listPlugins.Items.AddRange(Utils.GetPluginsFromGameDirectory(Directory.GetCurrentDirectory()));
        }

        private void btOpenData_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(sRunnerFileName))
            {
                MessageBox.Show("Please select the game executable first!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            if (string.IsNullOrEmpty(sDataFilePath))
                Process.Start("explorer.exe", $"\"{Directory.GetCurrentDirectory() + "\\data.win"}\"");
            else
                Process.Start("explorer.exe", $"\"{sDataFilePath}\"");
        }

        private void btYYTKLaunchContextMenu_LaunchCustom(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(sRunnerFileName))
            {
                MessageBox.Show("Please select the game executable first!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            if (MessageBox.Show("Please note this feature is designed for development purposes.\nTo launch with YYToolkit, use the other button.\nProceed anyway?", "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No)
                return;

            using (OpenFileDialog fileDialog = Utils.CreateFileDialog("%systemdrive%", "Find the custom DLL", "DLL files|*.dll", 1))
            {
                if (fileDialog.ShowDialog() == DialogResult.OK)
                {
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

                    Utils.Inject(p, fileDialog.FileName);
                }
            }
        }
    }
}