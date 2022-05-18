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

            listPlugins.ContextMenuStrip.Items.Add("Install plugin").Click += new EventHandler(PluginContextMenu_InstallPlugin);
            listPlugins.ContextMenuStrip.Items.Add("Uninstall plugin").Click += new EventHandler(PluginContextMenu_UninstallPlugin);
            listPlugins.ContextMenuStrip.Items.Add("Enable plugin").Click += new EventHandler(PluginContextMenu_EnablePlugin);
            listPlugins.ContextMenuStrip.Items.Add("Disable plugin").Click += new EventHandler(PluginContextMenu_DisablePlugin);
            listPlugins.ContextMenuStrip.Items.Add("Refresh list").Click += new EventHandler(PluginContextMenu_Refresh);
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
            if (string.IsNullOrEmpty(sRunnerFileName))
            {
                MessageBox.Show("Please select the game executable first!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            string YYTKPath = GetYYTKPath(cbForceUpdates.Checked, cbUseRandomFilename.Checked ? Path.GetRandomFileName().Replace(".", "").Substring(0, 6) : "YYToolkit");

            if (string.IsNullOrEmpty(YYTKPath))
                return;

            if (cbUsePreloading.Checked)
            {
                Utils.StartPreloaded(sRunnerFilePath, sDataFilePath, YYTKPath);
                return;
            }

            
            Process[] ExistingProcesses = Process.GetProcessesByName(Path.GetFileNameWithoutExtension(sRunnerFilePath));
            if (ExistingProcesses.Length > 0)
            {
                // There might be several apps named the same, running from a different path?
                foreach (Process process in ExistingProcesses)
                {
                    if (!process.MainModule.FileName.Equals(sRunnerFilePath))
                        continue;

                    DialogResult dialog = MessageBox.Show($"A runner process with PID {process.Id} already exists.\nInject into it?\n", "Runner process exists", MessageBoxButtons.YesNo, MessageBoxIcon.Information);
                    if (dialog != DialogResult.Yes)
                        break;

                    // TODO: Check if YYTK's already injected
                    Utils.WaitUntilWindow(process);

                    Utils.Inject(process, YYTKPath);

                    if (cbAutoCleanup.Checked)
                    {
                        this.Hide();
                        process.WaitForExit();
                        File.Delete(YYTKPath);
                        this.Show();
                    }

                    return;
                }
            }

            Process p = Process.Start(sRunnerFilePath, string.IsNullOrEmpty(sDataFilePath) ? "" : $"-game \"{sDataFilePath}\"");

            Utils.WaitUntilWindow(p);

            Utils.Inject(p, YYTKPath);

            if (cbAutoCleanup.Checked)
            {
                this.Hide();
                p.WaitForExit();
                File.Delete(YYTKPath);
                this.Show();
            }
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

            if (MessageBox.Show("Please note this feature is designed for development purposes.\nProceed anyway?", "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No)
                return;

            using (OpenFileDialog fileDialog = Utils.CreateFileDialog("%systemdrive%", "Find the data.win file", "GameMaker Data Files|*.win", 1))
            {
                if (fileDialog.ShowDialog() != DialogResult.OK)
                    return;

                sDataFilePath = fileDialog.FileName;

                txtDataFile.ResetText();
                txtDataFile.AppendText(sDataFilePath);
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
                if (fileDialog.ShowDialog() != DialogResult.OK)
                    return;

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

        private void btLaunchCustomDLL_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(sRunnerFileName))
            {
                MessageBox.Show("Please select the game executable first!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            if (MessageBox.Show("Please note this feature is designed for development purposes.\nProceed anyway?", "Warning", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) == DialogResult.No)
                return;

            using (OpenFileDialog fileDialog = Utils.CreateFileDialog("%systemdrive%", "Find the custom DLL", "DLL files|*.dll", 1))
            {
                if (fileDialog.ShowDialog() != DialogResult.OK)
                    return;

                if (cbUsePreloading.Checked)
                {
                    Utils.StartPreloaded(sRunnerFilePath, sDataFilePath, fileDialog.FileName);
                    return;
                }

                Process p = Process.Start(sRunnerFilePath, string.IsNullOrEmpty(sDataFilePath) ? "" : $"-game \"{sDataFilePath}\"");

                Utils.WaitUntilWindow(p);

                Utils.Inject(p, Path.GetFullPath(fileDialog.FileName));
            }
        }

        private void cbForceUpdates_CheckedChanged(object sender, EventArgs e)
        {

        }
    }
}