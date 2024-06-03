package net.alexbarry.calc_android;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.ListIterator;

public class UnitAdjusterHelper {
    private static final String TAG = "UnitAdjusterHelper";

    private static final LinkedHashMap<String, Integer> SI_PREFIX_TO_POW = new LinkedHashMap<>();
    private static final List<String> SI_PREFIXES = new ArrayList<>();
    private static final List<String> SI_PREFIX_OPTION_STRS = new ArrayList<>();
    private static final int SI_PREFIX_ZERO_POS;
    private static final List<String> POW_OPTIONS = new ArrayList<>();
    private static final List<Integer> POW_OPTIONS_VAL = new ArrayList<>();
    private static final int POW_OPTIONS_ZERO_POS;

    static {
        SI_PREFIX_TO_POW.put("Y",   24 );
        SI_PREFIX_TO_POW.put("Z",   21 );
        SI_PREFIX_TO_POW.put("E",   18 );
        SI_PREFIX_TO_POW.put("P",   15 );
        SI_PREFIX_TO_POW.put("T",   12 );
        SI_PREFIX_TO_POW.put("G",    9 );
        SI_PREFIX_TO_POW.put("M",    6 );
        SI_PREFIX_TO_POW.put("k",    3 );
        SI_PREFIX_TO_POW.put("h",    2 );
        SI_PREFIX_TO_POW.put("da",   1 );

        SI_PREFIX_TO_POW.put("d",   -1 );
        SI_PREFIX_TO_POW.put("c",   -2 );
        SI_PREFIX_TO_POW.put("m",   -3 );
        SI_PREFIX_TO_POW.put("u",   -6 );
        SI_PREFIX_TO_POW.put("n",   -9 );
        SI_PREFIX_TO_POW.put("p",  -12 );
        SI_PREFIX_TO_POW.put("f",  -15 );
        SI_PREFIX_TO_POW.put("a",  -18 );
        SI_PREFIX_TO_POW.put("z",  -21 );
        SI_PREFIX_TO_POW.put("y",  -14 );

        {
            int prev_pow = 24;
            int zero_idx = 0;
            int idx = 0;
            for (String siPrefix : SI_PREFIX_TO_POW.keySet()) {
                int pow = SI_PREFIX_TO_POW.get(siPrefix);
                if (prev_pow > 0 && pow < 0) {
                    SI_PREFIX_OPTION_STRS.add("");
                    SI_PREFIXES.add("");
                    zero_idx = idx;
                }
                SI_PREFIXES.add(siPrefix);
                SI_PREFIX_OPTION_STRS.add(String.format("(10^%d), %s", pow, siPrefix));
                idx++;
                prev_pow = pow;
            }
            SI_PREFIX_ZERO_POS = zero_idx;
        }

        {
            int idx = 0;
            int default_pos = 0;
            for (int pow = 10; pow >= -10; pow--) {
                if (pow == 0) {
                    continue;
                }
                String str;
                if (pow == 1) {
                    str = "";
                    default_pos = idx;
                } else {
                    str = String.format("^%d", pow);
                }
                POW_OPTIONS.add(str);
                POW_OPTIONS_VAL.add(pow);
                idx++;
            }
            POW_OPTIONS_ZERO_POS = default_pos;
        }
    }
    private SelectIncDecViewHelper prefix = new SelectIncDecViewHelper();
    private SelectIncDecViewHelper base = new SelectIncDecViewHelper();
    private SelectIncDecViewHelper pow = new SelectIncDecViewHelper();
    private TextView nice_name_label;
    private CalcAndroid.UnitInfo unitInfo;


    public interface EventListener {
        public void onBack();
        public void onInsertUnitPressed(String unitToken);
    }

    public View init(Context context, LayoutInflater inflater, final EventListener eventListener) {
        View view = inflater.inflate(R.layout.unit_selector_unit_adjuster, null);
        view.findViewById(R.id.btn_back).setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                eventListener.onBack();
            }
        });

        view.findViewById(R.id.insert_unit).setOnClickListener(new Button.OnClickListener() {
            @Override
            public void onClick(View v) {
                String unitToken = getUnitToken();
                eventListener.onInsertUnitPressed(unitToken);
            }
        });

        nice_name_label = view.findViewById(R.id.unit_name);

        prefix.init_views(context, view,
                R.id.unit_prefix_inc_btn, R.id.unit_prefix_spinner, R.id.unit_prefix_dec_btn,
                R.id.unit_prefix_label);
        base.init_views(context, view,
                R.id.unit_base_inc_btn, R.id.unit_base_spinner, R.id.unit_base_dec_btn,
                R.id.unit_base_label);
        pow.init_views(context, view,
                R.id.unit_pow_inc_btn, R.id.unit_pow_spinner, R.id.unit_pow_dec_btn,
                R.id.unit_pow_label);

        return view;
    }

    private String getUnitToken() {
        String prefix = "";
        if (unitInfo.is_si_prefixable) {
            int idx = this.prefix.getSelection();
            prefix = SI_PREFIXES.get(idx);
        }

        String base = unitInfo.getUnitNames().get(this.base.getSelection()).toString();

        String pow_str = "";
        if (this.pow.getSelection() != POW_OPTIONS_ZERO_POS && this.pow.getSelection() != -1) {
            //Log.v(TAG, String.format("user selected pow idx %d", this.pow.getSelection()));
            pow_str = String.format("^%d", POW_OPTIONS_VAL.get(this.pow.getSelection()));
        }

        String unitStr = prefix + base + pow_str;
        Log.i(TAG, String.format("getting unit token \"%s\"", unitStr));
        return unitStr;
    }

    public void setUnitInfo(CalcAndroid.UnitInfo unitInfo) {
        this.unitInfo = unitInfo;
        nice_name_label.setText(unitInfo.nice_name);

        if (unitInfo.is_si_prefixable) {
            prefix.setVisibility(View.VISIBLE);
            prefix.setChoices(SI_PREFIX_OPTION_STRS);
            prefix.setCurrentChoice(SI_PREFIX_ZERO_POS);
        } else {
            prefix.setChoices(new ArrayList<String>());
            prefix.setVisibility(View.INVISIBLE);
        }

        List<String> unitValueStrs = new ArrayList<>();
        for (CalcAndroid.UnitValue unitValue : unitInfo.getUnitNames()) {
            unitValueStrs.add(unitValue.toString());
        }
        base.setChoices(unitValueStrs);

        if (allowChangingPow(unitInfo)) {
            pow.setChoices(POW_OPTIONS);
            pow.setCurrentChoice(POW_OPTIONS_ZERO_POS);
            pow.setVisibility(View.VISIBLE);
        } else {
            pow.setChoices(new ArrayList<String>());
            pow.setVisibility(View.INVISIBLE);
        }
    }

    private static boolean allowChangingPow(CalcAndroid.UnitInfo unitInfo) {
        for (CalcAndroid.UnitValue unitValue : unitInfo.getUnitNames()) {
            if (unitValue.getFactors().size() > 1 || unitValue.getFactors().get(0).unit_pow != 1) {
                return false;
            }
        }
        return true;
    }
}
