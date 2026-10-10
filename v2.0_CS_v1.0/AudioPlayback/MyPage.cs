/*
	WAVE Audio Playback for Windows
	Version 4.0 (Interop C# version 1.0)

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

using System;
using System.Windows;
using System.Windows.Controls;

namespace AudioPlayback;

public abstract class MyPage : Page
{
	public virtual void Initialize()
	{
		this.Width = ((MainWindow) Application.Current.MainWindow).Width;
		this.Height = ((MainWindow) Application.Current.MainWindow).Height;
	}

	public virtual void Deinitialize()
	{
	}

	protected abstract void align();

	public void Proc_OnWindowSizeChanged(object sender, SizeChangedEventArgs e)
	{
		this.Width = ((MainWindow) Application.Current.MainWindow).Width;
		this.Height = ((MainWindow) Application.Current.MainWindow).Height;
		this.align();
	}
}
