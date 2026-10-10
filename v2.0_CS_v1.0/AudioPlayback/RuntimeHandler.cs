/*
	WAVE Audio Playback for Windows
	Version 4.0 (Interop C# version 1.0)

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

using System;
using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Threading;

namespace AudioPlayback;

public static class RuntimeHandler
{
	public static Thread? MainThread = null;
	public static Thread? AudioThread = null;

	public static bool AppInit()
	{
		if(Core.Initialize() == 0)
		{
			Win32Aux.MessageBox(Core.GetLastErrorMessage(), "INIT ERROR", (Win32Aux.MB_ICONSTOP | Win32Aux.MB_OK));
			RuntimeHandler.AppDeinit();
			return false;
		}

		return true;
	}

	public static void AppDeinit()
	{
		Core.Deinitialize();
	}

	public static void AppExit(uint exitCode, string exitMsg)
	{
		RuntimeHandler.AppDeinit();

		if(exitMsg.Length > 0) Win32Aux.MessageBox(exitMsg, "PROCESS EXIT CALLED", (Win32Aux.MB_ICONSTOP | Win32Aux.MB_OK));

		Win32Aux.ExitProcess(exitCode);

		while(true) Win32Aux.Sleep(10U);
	}

	public static void Proc_LoadFile_CreateAudioObj()
	{
		if(Core.LoadFile_CreateAudioObject() == 0)
		{
			Win32Aux.MessageBox(Core.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		((MainWindow) Application.Current.MainWindow).GoToChooseAudioDevicePage();
	}

	public static void Proc_InitAudioObj_RunPlayback()
	{
		if(Core.InitializeAudioObject() == 0)
		{
			Win32Aux.MessageBox(Core.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		RuntimeHandler.AudioThread = new Thread(RuntimeHandler.Proc_AudioThread);
		RuntimeHandler.AudioThread.Start();

		((MainWindow) Application.Current.MainWindow).GoToPlaybackRunningPage();
	}

	public static void Proc_OnPlaybackFinished()
	{
		((MainWindow) Application.Current.MainWindow).GoToPlaybackFinishedPage();
	}

	public static void Proc_AudioThread()
	{
		if(Core.RunPlayback() == 0) Win32Aux.MessageBox(Core.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
		Dispatcher.FromThread(RuntimeHandler.MainThread).BeginInvoke(RuntimeHandler.Proc_OnPlaybackFinished, null);
	}
}
