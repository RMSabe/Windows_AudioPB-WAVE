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

namespace AudioPlayback;

public partial class PlaybackFinishedPage : MyPage
{
	public PlaybackFinishedPage()
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

	private void onButtonReturnClicked(object sender, RoutedEventArgs e)
	{
		((MainWindow) Application.Current.MainWindow).GoToChooseFilePage();
	}
}
