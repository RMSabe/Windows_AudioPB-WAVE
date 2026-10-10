/*
	WAVE Audio Playback for Windows
	Version 4.0 (Interop C# version 1.0)

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

using System;
using System.Collections.Generic;
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

using System.Runtime.InteropServices;

namespace AudioPlayback;

public partial class ChooseAudioDevicePage : MyPage
{
	public ChooseAudioDevicePage()
	{
		this.InitializeComponent();
		this.Initialize();
	}

	public override void Initialize()
	{
		nuint pEntry = Win32Aux.NULL;
		nint nEntryCount;
		nint nEntry;

		base.Initialize();

		if(Core.LoadAudioDeviceList() == 0)
		{
			Win32Aux.MessageBox(Core.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		nEntryCount = Core.GetAudioDeviceListEntryCount();
		if(nEntryCount < 0)
		{
			Win32Aux.MessageBox(Core.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		nEntry = 0;
		while(nEntry < nEntryCount)
		{
			pEntry = Core.GetAudioDeviceListEntry_FriendlyName((nuint) nEntry);

			if(pEntry == Win32Aux.NULL)
			{
				Win32Aux.MessageBox(Core.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
				return;
			}

			this.ListBox1.Items.Add(Marshal.PtrToStringUni((nint) pEntry));
			nEntry++;
		}

		this.align();
	}

	protected override void align()
	{
		this.ListBox1.Width = this.Width - 2.0*(this.ListBox1.Margin.Left);
		this.ListBox1.Height = ((double) this.ListBox1.Items.Count)*20.0;
	}

	private void onButtonCancelClicked(object sender, RoutedEventArgs e)
	{
		((MainWindow) Application.Current.MainWindow).GoToChooseFilePage();
	}

	private void onButtonChooseSelectedClicked(object sender, RoutedEventArgs e)
	{
		int nSel = this.ListBox1.SelectedIndex;

		if(nSel < 0)
		{
			Win32Aux.MessageBox("Error: no device selected.", "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		if(Core.ChooseDevice((nuint) nSel) == 0)
		{
			Win32Aux.MessageBox(Core.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		RuntimeHandler.Proc_InitAudioObj_RunPlayback();
	}

	private void onButtonChooseDefaultClicked(object sender, RoutedEventArgs e)
	{
		if(Core.ChooseDefaultDevice() == 0)
		{
			Win32Aux.MessageBox(Core.GetLastErrorMessage(), "ERROR", (Win32Aux.MB_ICONEXCLAMATION | Win32Aux.MB_OK));
			return;
		}

		RuntimeHandler.Proc_InitAudioObj_RunPlayback();
	}
}
