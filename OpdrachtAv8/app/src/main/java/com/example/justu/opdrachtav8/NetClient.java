package com.example.justu.opdrachtav8;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.ArrayList;
import java.util.List;

/**
 * Created by justu on 6/6/2018.
 */

public class NetClient {

    private List<DataReceivedListener> listeners;
    private Socket clientSocket;
    private boolean listening = true;
    private Thread receiveThread;

    public NetClient() throws IOException {
        listeners = new ArrayList<DataReceivedListener>();
    }

    public void addListener(DataReceivedListener dataReceivedListener) {
        listeners.add(dataReceivedListener);
    }

    public void connect(String ip, int port) throws IOException {
        clientSocket = new Socket(ip, port);
        receiveData();
    }

    public void sendData(byte[] data) throws IOException {
        DataOutputStream dOut = new DataOutputStream(clientSocket.getOutputStream());
        dOut.write(data);
    }

    public void receiveData() throws IOException {
        receiveThread = new Thread(){
            public void run() {
                try {
                    while (!listening) {
                        DataInputStream dIn = null;
                        try {
                            dIn = new DataInputStream(clientSocket.getInputStream());
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                        int packetType = dIn.readInt();
                        switch (packetType) {
                            case PacketTypes.SENSOR_LIGHT:
                                dataReceived(packetType, new int[]{dIn.readInt()});
                                break;
                            case PacketTypes.SENSOR_TEMPATURE:
                                dataReceived(packetType, new int[]{dIn.readInt()});
                                break;
                            case PacketTypes.POWER_OUTLET:
                                //  dIn.read
                                break;
                            default:
                        }
                    }
                } catch (IOException e) {
                e.printStackTrace();
            }
            }
        };
        receiveThread.start();
    }

    public void dataReceived(int packetType, int[] data)
    {
        for (DataReceivedListener l : listeners)
        l.DataReceived(packetType, data);
    }

    public void stopConnection() throws IOException {
        clientSocket.close();
    }
}
