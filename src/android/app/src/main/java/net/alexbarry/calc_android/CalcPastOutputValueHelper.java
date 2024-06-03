package net.alexbarry.calc_android;

import java.util.ArrayList;
import java.util.List;

/**
 * Store a list of past values, so they can easily be converted between rect/polar and rad/degree,
 * and perhaps have the number of decimal places / sci / eng notation changed in the future
 */
class CalcPastOutputValueHelper<ValueT> {
    private List<ValueT> outputValues = new ArrayList<>();

    public void addOutputValue(ValueT value) {
        outputValues.add(value);
    }

    public ValueT getLastOutput() {
        if (outputValues.size() == 0) {
            return null;
        } else {
            return outputValues.get(outputValues.size()-1);
        }
    }

    public void clear() {
        outputValues.clear();
    }
}
