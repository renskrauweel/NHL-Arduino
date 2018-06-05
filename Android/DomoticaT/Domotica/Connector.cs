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

using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.Threading.Tasks;
using Domotica;


namespace Domotica
{
    //Connects to the Arduino and start Sender and Receiver threads.
	public class Connector
	{
		private  Thread _connectorThread;
		private  byte _threadStarted;
		private readonly MainActivity mainActivity;
		private  string ipAddress;
		private  string port;
		private  string error;
		private  Socket socket;
		private Receiver _receiver;
		private Sender _sender;

		public Connector (MainActivity mainAct)
		{
			//Add a reference to MainActivity (so the GUI can be updated from within this thread).
			mainActivity = mainAct; 
		}

		//Start the Connector thread.
		public void StartConnector (string ip, string prt)
		{
			ipAddress = ip;
			port = prt;

			if (_connectorThread == null) {
				//Create a new thread.
				_connectorThread = new Thread (ConnectorThreadWorker);
				_connectorThread.IsBackground = true;
				_connectorThread.Start ();
			} else {
				//Stop the thread If the Connector thread is already started.
				StopConnector ();
			}
		}

		//Stop the Connector thread.
		public void StopConnector ()
		{
			//Stop the Test Controller if it started.
			if (_connectorThread != null) {
				Thread.VolatileWrite (ref _threadStarted, 0);
			}
		}

		//Check if the Connector thread is running.
		public bool CheckStarted ()
		{
			if (_connectorThread != null) { 
                return _connectorThread.IsAlive;
			} else return false;
		}

		//Thread worker.
		private void ConnectorThreadWorker ()
		{
			Thread.VolatileWrite (ref _threadStarted, 1);
			try {
                IPAddress ip = IPAddress.Parse(ipAddress);   // IPAddress ipAddress = IPAddress.Parse("192.168.1.105");
				IPEndPoint ipEndPoint = new IPEndPoint (ip, Convert.ToInt32 (port));

				mainActivity.UpdateConnectionState(1, "Connecting...");

				//Try to connect to the Arduino.
				socket = new Socket (AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
				socket.Connect (ipEndPoint);

				//Check if connection is active -> if so, start receiver and sender
				if (socket.Connected) {
					mainActivity.UpdateConnectionState(2, "Connected");

					//Create Receiver instance.
					_receiver = new Receiver (mainActivity, socket);
					_receiver.StartReceiver ();

					//Create Sender instance.
					_sender = new Sender (socket);
					_sender.StartSender ();

					//Check if connection is active. 
					while ((Thread.VolatileRead (ref _threadStarted) == 1) && socket.Connected) {
						//Ask for pin status (from Arduino) update every second.
						socket.Send (Encoding.ASCII.GetBytes ("s"));  // protocol: s: get status information
						Thread.Sleep (1000);
					}
				}
			} catch (Exception exception) {
				error = exception.Message;
			} finally {
				//Always close the socket.
				if (socket != null) {
					socket.Close ();
				}

				//Stop Sender and Receive when the connection closes.
				if (_sender != null) {
					_sender.StopSender ();
					if (_sender.GetThread () != null) {
						_sender.GetThread ().Join ();
					}
				}
				if (_receiver != null) {
					_receiver.StopReceiver ();

					if (_receiver.GetThread () != null) {
						_receiver.GetThread ().Join ();
					}			
				}
			}

			if (error != null) {
				mainActivity.UpdateConnectionState(3, "Error: " + error + ".");
			} else {
				mainActivity.UpdateConnectionState(4, "Disconnected.");              
			}
			_connectorThread = null;
		}

		//Send a message to the Sender instance.
		public void SendMessage (string message)
		{
			_sender.SetMessage (message);
		}

		//Stop threads forcibly (for debugging only).
		public void Abort ()
		{
			if (_receiver != null) {
				if (_receiver.GetThread () != null) {

					_receiver.GetThread ().Abort ();
				}
			}
			if (_sender != null) {
				if (_sender.GetThread () != null) {

					_sender.GetThread ().Abort ();
				}
			}
			if (_connectorThread != null) {
				_connectorThread.Abort ();
				_connectorThread = null;
			}
			mainActivity.UpdateConnectionState(4, "Disconnected.");              
		}
	}
}