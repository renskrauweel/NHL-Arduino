package com.example.justu.opdrachtav8;


import android.graphics.Color;
import android.os.Bundle;
import android.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Switch;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;


/**
 * A simple {@link Fragment} subclass.
 */
public class MainFragment extends Fragment {

    public static int POWEROUTLETS_AMOUNT = 3;
    public MainFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View rootView = inflater.inflate(R.layout.fragment_main, container, false);
        TableLayout lyPowerOutlets = (TableLayout)rootView.findViewById(R.id.lyPowerOutlets);
        for (int i = 0; i <  POWEROUTLETS_AMOUNT; i++)
        {
            TableRow tableRow = new TableRow(getContext());
            TextView lblPowerOutletStatus = new TextView(getContext());
            lblPowerOutletStatus.setText(String.format("Power outlet  %1$s", i));
            lblPowerOutletStatus.setTextColor(Color.RED);
            ViewGroup.LayoutParams params = new TableRow.LayoutParams();
            TableRow.LayoutParams layoutParams = new TableRow.LayoutParams(TableRow.LayoutParams.WRAP_CONTENT,TableRow.LayoutParams.WRAP_CONTENT);
            layoutParams.setMargins(300, 0 ,0 , 0);
            Switch swPowerOutlet = new Switch(getContext());
            swPowerOutlet.setLayoutParams(layoutParams);
            tableRow.addView(lblPowerOutletStatus);
            tableRow.addView(swPowerOutlet);
            lyPowerOutlets.addView(tableRow);
        }
        return rootView;
    }


}
