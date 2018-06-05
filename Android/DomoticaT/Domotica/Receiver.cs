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
using System.Text.RegularExpressions;
using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;

using Domotica;

namespace Domotica
{
	//Receive network packages from the Arduino.
	public class Receiver
	{
		private Thread _receiverThread;
		private byte _threadStarted;
		private readonly MainActivity mainActivity;
		private readonly Socket socket;

        
        // Constructor
        public Receiver(MainActivity ma, Socket soc)
        {
            mainActivity = ma;
            socket = soc;
        }

        //Start the Receiver thread.
        public void StartReceiver ()
		{
			if (_receiverThread == null) {
				//Create a new thread.
				_receiverThread = new Thread (ReceiverThreadWorker);
				_receiverThread.IsBackground = true;
				_receiverThread.Start ();
			}
		}

		//Stop the Receiver thread.
		public void StopReceiver ()
		{
			//Stop the Test Controller if it started.
			if (_receiverThread != null) {
				Thread.VolatileWrite (ref _threadStarted, 0);
			}
		}

		//Thread worker.
		private void ReceiverThreadWorker ()
		{
            string result = "---";
            try {
				Thread.VolatileWrite (ref _threadStarted, 1);

				//Store received bytes
				byte[] buffer = new byte[4];

				while ((Thread.VolatileRead (ref _threadStarted) == 1) && socket.Connected) {
					//Read byte if available, else block until available.
					int bytesRead = socket.Receive (buffer);

					//Read all available bytes.
					while (socket.Available > 0) bytesRead = socket.Receive (buffer);

                    if (bytesRead == 4) { 
                        result = Encoding.ASCII.GetString(buffer, 0, bytesRead - 1); // remove \n
                        if (result == "OFF" || result == " ON") {
                            mainActivity.UpdateGUI(result, mainActivity.textViewChangePinStateValue);
                        }
					}
					Thread.Sleep (10);
				}
			} catch (Exception exception) {
				Console.WriteLine ("Receiver Error:" + exception.Message + ".");
			} finally {
				_receiverThread = null;
			}
		}

		//Returns the thread.
		public Thread GetThread ()
		{
			return _receiverThread;
		}
	}
}