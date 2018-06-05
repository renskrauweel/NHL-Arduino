using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Android.App;
using Android.Content;
using Android.OS;
using Android.Runtime;
using Android.Views;
using Android.Widget;

using System.ComponentModel;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

namespace Domotica
{
	//Send network packages to the Arduino.
	public class Sender
	{
		private Thread senderThread;
		private byte threadStarted;
		private string message;
		private readonly Socket socket;

		public Sender (Socket soc)
		{
			socket = soc;
		}

		//Start the Sender thread.
		public void StartSender ()
		{
			if (senderThread == null) {
				//Create a new thread.
				senderThread = new Thread (SenderThreadWorker);
				senderThread.IsBackground = true;
				senderThread.Start ();
			}
		}

		//Stop the Sender thread.
		public void StopSender ()
		{
			//Stop the Sender if it started.
			if (senderThread != null && senderThread.IsAlive) {
				Thread.VolatileWrite (ref threadStarted, 0);
			}
		}

		//Thread worker.
		private void SenderThreadWorker ()
		{
			try {				
				Thread.VolatileWrite (ref threadStarted, 1);

				//Check if connection is still active.
				while ((Thread.VolatileRead (ref threadStarted) == 1) && socket.Connected) {

					if (message != null) {
						//Encode the character to a byte.
						byte[] msg = Encoding.ASCII.GetBytes (message);

						//Send the byte to the Arduino server
						socket.Send (msg);
                        // clear message
						message = null;
					}
					Thread.Sleep (100);
				}
			} catch (Exception exception) {
				//Show an exception message in the console.
				Console.WriteLine ("Sender Error:" + exception.Message);
			} finally {
				senderThread = null;
			}
		}

		//Set the message.
		public void SetMessage (string msg)
		{
			message = msg;
		}

		//Returns the thread.
		public Thread GetThread ()
		{
			return senderThread;
		}
	}
}

