using System;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Net;
using System.Runtime.InteropServices;
using System.Windows.Forms;

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

		public static void Inject(IntPtr Handle, string pathToDLL)
		{
			IntPtr pLoadLib = WinAPI.GetProcAddress(WinAPI.GetModuleHandle("kernel32.dll"), "LoadLibraryA");

			IntPtr pRemoteString = WinAPI.VirtualAllocEx(Handle, IntPtr.Zero, (uint)((pathToDLL.Length + 1) * Marshal.SizeOf(typeof(char))), 12288u, 4u);

			WinAPI.WriteProcessMemory(Handle, pRemoteString, System.Text.Encoding.Default.GetBytes(pathToDLL), (uint)((pathToDLL.Length + 1) * Marshal.SizeOf(typeof(char))), out var _);

			WinAPI.CreateRemoteThread(Handle, IntPtr.Zero, 0u, pLoadLib, pRemoteString, 0u, IntPtr.Zero);
		}

		public static void StartPreloaded(string sRunnerFilePath, string sDataFilePath, string sPathToDll)
		{
			WinAPI.PROCESS_INFORMATION pInfo = new WinAPI.PROCESS_INFORMATION();
			WinAPI.STARTUPINFO sInfo = new WinAPI.STARTUPINFO();
			WinAPI.SECURITY_ATTRIBUTES pSec = new WinAPI.SECURITY_ATTRIBUTES();
			WinAPI.SECURITY_ATTRIBUTES tSec = new WinAPI.SECURITY_ATTRIBUTES();
			pSec.nLength = Marshal.SizeOf(pSec);
			tSec.nLength = Marshal.SizeOf(tSec);

			bool Success = false;
			if (String.IsNullOrEmpty(sDataFilePath))
			{
				Success = WinAPI.CreateProcess(sRunnerFilePath, "", ref pSec, ref tSec, false,
				4 /* CREATE_SUSPENDED */, IntPtr.Zero, Path.GetDirectoryName(sRunnerFilePath), ref sInfo, out pInfo);
			}
			else
			{
				Success = WinAPI.CreateProcess(sRunnerFilePath, $"-game \"{sDataFilePath}\"", ref pSec, ref tSec, false,
				4 /* CREATE_SUSPENDED */, IntPtr.Zero, Path.GetDirectoryName(sRunnerFilePath), ref sInfo, out pInfo);
			}

			if (!Success)
			{
				MessageBox.Show("Failed to create a process.\nGetLastError() returned " + Marshal.GetLastWin32Error().ToString(), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
				return;
			}

			Inject(pInfo.hProcess, sPathToDll);
		}

		public static OpenFileDialog CreateFileDialog(string StartPath, string Title, string Filter, int FilterIndex)
		{
			OpenFileDialog dialog = new OpenFileDialog()
			{
				//InitialDirectory = Environment.ExpandEnvironmentVariables(StartPath),
				Title = Title,
				Filter = Filter,
				FilterIndex = FilterIndex,
				RestoreDirectory = true
			};

			return dialog;
		}

		public static void WaitUntilWindow(Process process)
		{
			process.WaitForInputIdle();

			while (!process.HasExited)
			{
				try
				{
					if (process.MainWindowHandle != IntPtr.Zero)
						break;
				}
				catch (InvalidOperationException)
				{
					if (!process.HasExited)
						throw;
				}
			}

			process.WaitForInputIdle();
		}
		public static string[] GetPluginsFromGameDirectory(string GameDirPath)
		{
			if (!Directory.Exists(GameDirPath) || !Directory.Exists(GameDirPath + "\\autoexec"))
				return Array.Empty<string>();

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
		// Downloads YYTK and returns the path it's at.
		public static string GetYYTKPath(bool ForceUpdates, string DLLName)
		{
			string DownloadPath = $"{Directory.GetCurrentDirectory()}\\{DLLName}.dll";

#pragma warning disable SYSLIB0014
			using (var Browser = new WebClient())
			{
				string URL = "https://github.com/Archie-osu/YYToolkit/releases/latest/download/YYToolkit.dll";

				try
				{
					// Delete the DLL 
					if (File.Exists(DownloadPath))
                    {
						// We already have a DLL and we don't force updates, so use that
						if (!ForceUpdates)
							return DownloadPath;

						File.Delete(DownloadPath);
					}
						
					Browser.DownloadFile(URL, DownloadPath);
				}
				catch (Exception ex1)
				{
					// Failed to download to current dir, try userprofile
					try
					{
						DownloadPath = $"{Environment.GetFolderPath(Environment.SpecialFolder.UserProfile)}\\{DLLName}.dll";

						// Delete the DLL 
						if (File.Exists(DownloadPath))
                        {
							if (!ForceUpdates)
								return DownloadPath;

							File.Delete(DownloadPath);
						}

						Browser.DownloadFile(URL, DownloadPath);
					}
					catch (Exception ex2)
					{
						string Exception1Message = "";
						string Exception2Message = "";

						while (ex1 != null)
						{
							Exception1Message += $"{ex1.Message}\n";
							ex1 = ex1.InnerException;
						}

						while (ex2 != null)
						{
							Exception2Message += $"{ex2.Message}\n";
							ex2 = ex2.InnerException;
						}

						MessageBox.Show(
							"Injection failed!\n" +
							"Please report this occurence to GitHub, as this is really shouldn't happen.\n\n" +
							$"First method: {Exception1Message}\n" +
							$"Second method: {Exception2Message}",
							"Error!", MessageBoxButtons.OK, MessageBoxIcon.Error
						);

						return "";
					}
				}
			}
			return DownloadPath;
		}
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
