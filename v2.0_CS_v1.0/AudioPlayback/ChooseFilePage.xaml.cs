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

using Microsoft.Win32;

namespace AudioPlayback;

public partial class ChooseFilePage : MyPage
{
	public ChooseFilePage()
	{
		this.InitializeComponent();
		this.Initialize();
	}

	public override void Initialize()
	{
		base.Initialize();
		this.align();
	}

	protected override void align()
	{
	}

	private void onButtonBrowseFilesClicked(object sender, RoutedEventArgs e)
	{
		this.proc_OpenFileDialog();
	}

	private unsafe void proc_OpenFileDialog()
	{
		OpenFileDialog ofdlg = new OpenFileDialog();
		bool? bRet = null;

		ofdlg.Filter = "Wave Files|*.wav|All Files|*.*";
		ofdlg.DefaultExt = ".wav";
		ofdlg.FilterIndex = 1;

		bRet = ofdlg.ShowDialog();

		if(bRet == null) return;
		if(bRet == false) return;

		fixed(char *fdir = ofdlg.FileName) { Core.SetFileInDirectory((nuint) fdir); }

		RuntimeHandler.Proc_LoadFile_CreateAudioObj();
	}
}
