package net.alexbarry.calc_android;

import android.content.Context;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;

import java.util.List;

public class SelectIncDecViewHelper {
    private Button inc_btn;
    private Button dec_btn;
    private Spinner spinner_select;
    private View label;
    private Context context;


    public void init_views(Context context, View parent,
                           int inc_btn_id, int select_id, int dec_btn_id, int label_id) {
        this.context = context;
        inc_btn = parent.findViewById(inc_btn_id);
        dec_btn = parent.findViewById(dec_btn_id);
        spinner_select = parent.findViewById(select_id);
        label = parent.findViewById(label_id);

        inc_btn.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                adjustSelection(-1);
            }
        });

        dec_btn.setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                adjustSelection(1);
            }
        });
    }

    public void setChoices(List<String> choices) {
        if (choices.size() == 1) {
            inc_btn.setEnabled(false);
            inc_btn.setVisibility(View.INVISIBLE);
            dec_btn.setEnabled(false);
            dec_btn.setVisibility(View.INVISIBLE);
        } else {
            inc_btn.setEnabled(true);
            inc_btn.setVisibility(View.VISIBLE);
            dec_btn.setEnabled(true);
            dec_btn.setVisibility(View.VISIBLE);
        }
        spinner_select.setAdapter(new ArrayAdapter<String>(context,
                android.R.layout.simple_list_item_1, choices));
    }

    public void setCurrentChoice(int idx) {
        spinner_select.setSelection(idx);
    }

    public void setVisibility(int visibility) {
        inc_btn.setVisibility(visibility);
        dec_btn.setVisibility(visibility);
        spinner_select.setVisibility(visibility);
        label.setVisibility(visibility);
    }

    private void adjustSelection(int diff) {
        int selectedIdx = this.spinner_select.getSelectedItemPosition();
        selectedIdx += diff;
        if (selectedIdx >= this.spinner_select.getCount()) {
            selectedIdx = this.spinner_select.getCount() - 1;
        } else if (selectedIdx < 0) {
            selectedIdx = 0;
        }
        this.spinner_select.setSelection(selectedIdx);
    }

    public int getSelection() {
        return this.spinner_select.getSelectedItemPosition();
    }
}
