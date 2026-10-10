/*
	WAVE Audio Playback for Windows
	Version 4.0 (Interop C# version 1.0)

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

using System;
using System.Runtime.InteropServices;

namespace AudioPlayback;

public static class Core
{
	public const string LIBCORE_FILE = "core.dll";

	public const int STATUS_ERROR_INVALIDPARAMS = -5;
	public const int STATUS_ERROR_MEMORY = -4;
	public const int STATUS_ERROR_AUDIOHW = -3;
	public const int STATUS_ERROR_NOFILE = -2;
	public const int STATUS_ERROR_GENERIC = -1;
	public const int STATUS_UNINITIALIZED = 0;
	public const int STATUS_READY = 1;
	public const int STATUS_RUNNING = 2;
	public const int STATUS_PAUSED = 3;
	public const int STATUS_STOPPED = 4;

	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern Int32 Initialize();
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern void Deinitialize();
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern UIntPtr GetLastErrorMessage();
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern Int32 SetFileInDirectory(UIntPtr fdir);
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern Int32 LoadFile_CreateAudioObject();
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern Int32 LoadAudioDeviceList();
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern IntPtr GetAudioDeviceListEntryCount();
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern UIntPtr GetAudioDeviceListEntry_FriendlyName(UIntPtr index);
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern Int32 ChooseDevice(UIntPtr index);
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern Int32 ChooseDefaultDevice();
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern Int32 InitializeAudioObject();
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern Int32 GetStatus();
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern Int32 RunPlayback();
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern void PausePlayback();
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern void ResumePlayback();
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern void StopPlayback();
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern Int64 GetAudioDataSizeFrames();
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern Int64 GetAudioDataPositionFrames();
	[DllImport(LIBCORE_FILE, CallingConvention = CallingConvention.StdCall)] public static extern Int32 SetAudioDataPositionFrames(UInt64 position);
}
