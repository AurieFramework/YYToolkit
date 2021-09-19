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
            OpenFileDialog dialog = new OpenFileDialog();

            dialog.InitialDirectory = Environment.ExpandEnvironmentVariables(StartPath);
            dialog.Title = Title;
            dialog.Filter = Filter;
            dialog.FilterIndex = FilterIndex;
            dialog.RestoreDirectory = true;

            return dialog;
        }

        public static string[] GetPluginsFromGameDirectory(string GameDirPath)
        {
            string[] Plugins = { };

            if (!Directory.Exists(GameDirPath) || !Directory.Exists(GameDirPath + "\\autoexec"))
                return Plugins;

            return Directory.GetFiles(GameDirPath + "\\autoexec", "*.dll");
        }
    }
}
