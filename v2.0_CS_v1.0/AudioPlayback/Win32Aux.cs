/*
	WAVE Audio Playback for Windows
	Version 4.0 (Interop C# version 1.0)

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

using System;
using System.Printing;
using System.Runtime.InteropServices;

namespace AudioPlayback;

public static class Win32Aux
{
	public const uint EXITCODE_SUCCESS = 0U;
	public const uint EXITCODE_ERROR_GENERIC = unchecked((uint) -1);

	public const uint MB_OK = 0x0U;
	public const uint MB_OKCANCEL = 0x1U;
	public const uint MB_ABORTRETRYIGNORE = 0x2U;
	public const uint MB_YESNOCANCEL = 0x3U;
	public const uint MB_YESNO = 0x4U;
	public const uint MB_RETRYCANCEL = 0x5U;
	public const uint MB_CANCELTRYCONTINUE = 0x6U;

	public const uint MB_ICONSTOP = 0x10U;
	public const uint MB_ICONQUESTION = 0x20U;
	public const uint MB_ICONEXCLAMATION = 0x30U;
	public const uint MB_ICONINFORMATION = 0x40U;

	public const uint MB_HELP = 0x4000U;

	public const int IDOK = 1;
	public const int IDCANCEL = 2;
	public const int IDABORT = 3;
	public const int IDRETRY = 4;
	public const int IDIGNORE = 5;
	public const int IDYES = 6;
	public const int IDNO = 7;
	public const int IDTRYAGAIN = 10;
	public const int IDCONTINUE = 11;

	public const uint COINIT_MULTITHREADED = 0x0U;
	public const uint COINIT_APARTMENTTHREADED = 0x2U;
	public const uint COINIT_DISABLE_OLE1DDE = 0x4U;
	public const uint COINIT_SPEED_OVER_MEMORY = 0x8U;

	public const int S_OK = 0x0;
	public const int S_FALSE = 0x1;

	public const UIntPtr NULL = 0U;

	[DllImport("kernel32.dll")] public static extern void Sleep(UInt32 dwMilliseconds);
	[DllImport("kernel32.dll")] public static extern void ExitProcess(UInt32 nExitCode);

	[DllImport("ole32.dll")] public static extern Int32 CoInitializeEx(UIntPtr pvReserved, UInt32 dwCoInit);
	[DllImport("ole32.dll")] public static extern void CoUninitialize();

	[DllImport("user32.dll", CharSet = CharSet.Unicode)] public static extern Int32 MessageBox(UIntPtr hWnd, UIntPtr lpText, UIntPtr lpCaption, UInt32 uType);

	public static unsafe int MessageBox(string text, string caption, uint type)
	{
		int nRet;

		fixed(char *p_text = text)
		{
			fixed(char *p_caption = caption)
			{ nRet = MessageBox(Win32Aux.NULL, (nuint) p_text, (nuint) p_caption, type); }
		}

		return nRet;
	}

	public static unsafe int MessageBox(nuint ptext, string caption, uint type)
	{
		int nRet;

		fixed(char *p_caption = caption) { nRet = MessageBox(Win32Aux.NULL, ptext, (nuint) p_caption, type); }

		return nRet;
	}
}
