/*
	WAVE Audio Playback for Windows
	Version 4.0 (Interop C# version 1.0)

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

using System.Text;
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

public partial class MainWindow : Window
{
	public MainWindow()
	{
		if(!RuntimeHandler.AppInit()) Win32Aux.ExitProcess(Win32Aux.EXITCODE_ERROR_GENERIC);

		this.InitializeComponent();
		RuntimeHandler.MainThread = Thread.CurrentThread;

		this.Content = new ChooseFilePage();
	}

	public void GoToChooseFilePage()
	{
		if(this.Content != null) ((MyPage) this.Content).Deinitialize();
		this.Content = new ChooseFilePage();
	}

	public void GoToChooseAudioDevicePage()
	{
		if(this.Content != null) ((MyPage) this.Content).Deinitialize();
		this.Content = new ChooseAudioDevicePage();
	}

	public void GoToPlaybackRunningPage()
	{
		if(this.Content != null) ((MyPage) this.Content).Deinitialize();
		this.Content = new PlaybackRunningPage();
	}

	public void GoToPlaybackFinishedPage()
	{
		if(this.Content != null) ((MyPage) this.Content).Deinitialize();
		this.Content = new PlaybackFinishedPage();
	}

	private void onWindowClosing(object sender, EventArgs e)
	{
		if(this.Content != null) ((MyPage) this.Content).Deinitialize();
	}

	private void onWindowClosed(object sender, EventArgs e)
	{
		RuntimeHandler.AppDeinit();
	}

	private void onWindowSizeChanged(object sender, SizeChangedEventArgs e)
	{
		if(this.Content != null) ((MyPage) this.Content).Proc_OnWindowSizeChanged(sender, e);
	}
}
