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

using System.Windows.Threading;

namespace AudioPlayback;

public partial class PlaybackRunningPage : MyPage
{
	private Thread? timeCursorThread = null;
	private double timeCursorValue = 0.0;
	private bool timeCursorThreadStop = false;
	private bool timeCursorIsAdjusting = false;

	public PlaybackRunningPage()
	{
		this.InitializeComponent();
		this.Initialize();
		
		this.timeCursorThreadStop = false;
		this.timeCursorIsAdjusting = false;
		
		if(this.timeCursorThread != null) this.timeCursorThread.Start();
	}

	public override void Initialize()
	{
		long audioDataSizeFrames;

		base.Initialize();

		this.Button_PauseResumePlayback.Content = "Pause Playback";

		audioDataSizeFrames = Core.GetAudioDataSizeFrames();
		if(audioDataSizeFrames > 0L)
		{
			this.Slider_TimeCursor.Maximum = (double) audioDataSizeFrames;
			this.Slider_TimeCursor.Minimum = 0.0;
			this.Slider_TimeCursor.Value = 0.0;

			this.timeCursorThread = new Thread(this.proc_TimeCursorThread);
		}
		else
		{
			this.Slider_TimeCursor.Visibility = Visibility.Hidden;
			this.Slider_TimeCursor.IsEnabled = false;
		}

		this.align();
	}

	public override void Deinitialize()
	{
		base.Deinitialize();

		this.timeCursorThreadStop = true;
		if(this.timeCursorThread != null) this.timeCursorThread.Join();
	}

	protected override void align()
	{
	}

	private void sliderTimeCursor_UpdateValue()
	{
		long audioDataPos;

		audioDataPos = Core.GetAudioDataPositionFrames();
		if(audioDataPos >= 0L)
		{
			this.Slider_TimeCursor.IsEnabled = false;
			this.Slider_TimeCursor.Value = (double) audioDataPos;
			this.Slider_TimeCursor.IsEnabled = true;
		}
	}

	private void onButtonStopPlaybackClicked(object sender, RoutedEventArgs e)
	{
		Core.StopPlayback();
	}

	private void onButtonPauseResumePlaybackClicked(object sender, RoutedEventArgs e)
	{
		switch(Core.GetStatus())
		{
			case Core.STATUS_RUNNING:
				this.Button_PauseResumePlayback.Content = "Resume Playback";
				Core.PausePlayback();
				break;

			case Core.STATUS_PAUSED:
				this.Button_PauseResumePlayback.Content = "Pause Playback";
				Core.ResumePlayback();
				break;
		}
	}

	private void onSliderTimeCursorValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
	{
		this.timeCursorValue = e.NewValue;

		if(!this.timeCursorIsAdjusting) Core.SetAudioDataPositionFrames((ulong) this.timeCursorValue);
	}

	private void onSliderTimeCursorThumbDragStarted(object sender, System.Windows.Controls.Primitives.DragStartedEventArgs e)
	{
		this.timeCursorIsAdjusting = true;
	}

	private void onSliderTimeCursorThumbDragCompleted(object sender, System.Windows.Controls.Primitives.DragCompletedEventArgs e)
	{
		Core.SetAudioDataPositionFrames((ulong) this.timeCursorValue);
		this.timeCursorIsAdjusting = false;
	}

	private void proc_TimeCursorThread()
	{
		while(!this.timeCursorThreadStop)
		{
			switch(Core.GetStatus())
			{
				case Core.STATUS_RUNNING:
				case Core.STATUS_PAUSED:

					Dispatcher.FromThread(RuntimeHandler.MainThread).BeginInvoke(this.sliderTimeCursor_UpdateValue, null);
					break;

				default:
					this.timeCursorThreadStop = true;
					return;
			}

			Thread.Sleep(1024);
		}
	}
}
