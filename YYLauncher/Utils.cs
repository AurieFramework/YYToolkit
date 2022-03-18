using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using Launcher;

namespace Launcher
{
    public class Utils
    {
        public static void Inject(Process p, string pathToDLL)
        {
            IntPtr pLoadLib = WinAPI.GetProcAddress(WinAPI.GetModuleHandle("kernel32.dll"), "LoadLibraryA");

            IntPtr pRemoteString = WinAPI.VirtualAllocEx(p.Handle, IntPtr.Zero, (uint)((pathToDLL.Length + 1) * Marshal.SizeOf(typeof(char))), 12288u, 4u);

            WinAPI.WriteProcessMemory(p.Handle, pRemoteString, System.Text.Encoding.Default.GetBytes(pathToDLL), (uint)((pathToDLL.Length + 1) * Marshal.SizeOf(typeof(char))), out var _);

            WinAPI.CreateRemoteThread(p.Handle, IntPtr.Zero, 0u, pLoadLib, pRemoteString, 0u, IntPtr.Zero);
        }

        public static OpenFileDialog CreateFileDialog(string StartPath, string Title, string Filter, int FilterIndex)
        {
            OpenFileDialog dialog = new OpenFileDialog
            {
                //InitialDirectory = Environment.ExpandEnvironmentVariables(StartPath),
                Title = Title,
                Filter = Filter,
                FilterIndex = FilterIndex,
                RestoreDirectory = true
            };

            return dialog;
        }

        public static string[] GetPluginsFromGameDirectory(string GameDirPath)
        {
            if (!Directory.Exists(GameDirPath) || !Directory.Exists(GameDirPath + "\\autoexec"))
                return new string[0];

            var EnabledEntries = Directory.GetFiles(GameDirPath + "\\autoexec", "*.dll");
            var DisabledEntries = Directory.GetFiles(GameDirPath + "\\autoexec", "*.dll.disabled");

            string[] Plugins = new string[EnabledEntries.Length + DisabledEntries.Length];

            int n = 0;
            foreach (var file in EnabledEntries)
            {
                Plugins[n] = Path.GetFileName(file);
                n++;
            }

            foreach (var file in DisabledEntries)
            {
                Plugins[n] = Path.GetFileName(file);
                n++;
            }

            return Plugins;
        }
    }

    public partial class MainWindow : Form
    {
        public static bool IsReadyToManagePlugins(string sRunnerFilename, ListBox listPlugins)
        {
            if (string.IsNullOrEmpty(sRunnerFilename))
            {
                MessageBox.Show("Please select the game executable first!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return false;
            }

            if (!Directory.Exists("autoexec"))
            {
                MessageBox.Show("You can't uninstall something when there's nothing!", "What are you doing LOL", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }

            if (Utils.GetPluginsFromGameDirectory(Directory.GetCurrentDirectory()).Length == 0)
            {
                MessageBox.Show("No plugins were found in the 'autoexec' directory.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }

            if (listPlugins.SelectedItem == null)
            {
                MessageBox.Show("Please select a plugin first.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return false;
            }

            return true;
        }
    }
}
