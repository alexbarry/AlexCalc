package net.alexbarry.calc_android;

import java.util.List;
import java.util.Map;

public final class Utils {
    private Utils() {}

    public static String strAryToStr(List<String> list) {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("[");

        boolean first = true;
        for (String elem : list) {
            if (!first) {
                stringBuilder.append(", ");
            }
            stringBuilder.append("\"");
            stringBuilder.append(elem);
            stringBuilder.append("\"");
            first = false;
        }
        stringBuilder.append("]");
        return stringBuilder.toString();
    }

    public static String strMapToString(Map<String, String> map) {
        StringBuilder stringBuilder = new StringBuilder();
        stringBuilder.append("[");

        boolean first = true;
        for (String key : map.keySet()) {
            if (!first) {
                stringBuilder.append(", ");
            }
            stringBuilder.append("\"");
            stringBuilder.append(key);
            stringBuilder.append("\": ");

            stringBuilder.append("\"");
            stringBuilder.append(map.get(key));
            stringBuilder.append("\"");
            first = false;
        }
        stringBuilder.append("]");
        return stringBuilder.toString();
    }

}
