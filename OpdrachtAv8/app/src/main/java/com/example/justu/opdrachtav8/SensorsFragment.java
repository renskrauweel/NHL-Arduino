package com.example.justu.opdrachtav8;


import android.content.Context;
import android.os.Bundle;
import android.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.util.Log;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.github.mikephil.charting.charts.LineChart;
import java.util.HashMap;
import java.util.Map;


public class SensorsFragment extends Fragment implements DataReceivedListener {


    private LineChart mChart;
    public Map<TextView, TextView> SensorTextFields;
    public Map<String, Integer> Sensors = new HashMap<String, Integer>()
    {
        {
            put("Light sensor", PacketTypes.SENSOR_LIGHT);
            put("Temperature sensor", PacketTypes.SENSOR_TEMPATURE);
        }
    };

    public SensorsFragment() {
        Networking.netClient.addListener(this);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        SensorTextFields = new HashMap<TextView, TextView>();
        View rootView = inflater.inflate(R.layout.fragment_sensors, container, false);
        LinearLayout layout =  (LinearLayout)rootView.findViewById(R.id.lySensors);
        for (Map.Entry<String, Integer> entry : Sensors.entrySet()) {
            TextView txtTitle = new TextView(getContext());
            txtTitle.setTextSize(pxFromDp(15, getContext()));
            txtTitle.setText(entry.getKey().toString());
            TextView txtValue = new TextView(getContext());
            txtTitle.setTextSize(pxFromDp(10, getContext()));
            txtValue.setText(String.format("VALUE: %s0", entry.getValue()));
            SensorTextFields.put(txtTitle,txtValue);
            layout.addView(txtTitle);
            layout.addView(txtValue);
        }
        return rootView;
    }

    public static float pxFromDp(float dp, Context mContext) {
        return dp * mContext.getResources().getDisplayMetrics().density;
    }

    @Override
    public void DataReceived(int packetType, int[] data) {
        Log.d("myTag",  String.valueOf(data[0]));
    }


}
