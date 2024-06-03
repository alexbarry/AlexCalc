package net.alexbarry.calc_android;

import android.util.Log;
import android.webkit.WebView;

import java.util.List;

public class CalcOutputDisplayHelper {

    private static final String TAG = "CalcOutputDisplayHelper";

    private final WebView webView;
    private boolean isReady = false;

    public CalcOutputDisplayHelper(WebView webView) {
        this.webView = webView;
    }

    private String escapeStringForJs(String str) {
        str = str.replaceAll("\\\\", "\\\\\\\\");
        str = str.replaceAll("\"", "\\\\\"");
        return str;
    }

    public void updateWipDisplay(String tex) {
        evaluateJavascript("update_wip_tex(\"" + escapeStringForJs(tex) + "\");");
    }

    public void editLastOutputLine(String output_tex) {
        evaluateJavascript(String.format("edit_last_output_line_tex(\"%s\");",
                escapeStringForJs(output_tex)));
    }

    public void addOutputLine(String output_tex) {
        evaluateJavascript("add_output_line_tex(\"" + escapeStringForJs(output_tex) + "\");");
    }
    public void clearWipDisplay() {
        evaluateJavascript("clear_wip_tex();");
    }

    public void clearOutputDisplay() {
        evaluateJavascript("clear_output_display();");
        clearWipDisplay();
    }

    public void addOutputLineErr(String err_msg) {
        evaluateJavascript(String.format("add_output_line_err(\"%s\");",
                escapeStringForJs(err_msg)));
    }

    public void addOutputLineMsg(String msg) {
        evaluateJavascript(String.format("add_output_line_msg(\"%s\");",
                escapeStringForJs(msg)));
    }

    public void setTheme(ThemeType type) {
        Log.i(TAG, "setting theme to " + type.name());
        String type_string = null;
        switch(type) {
            case LIGHT: type_string = "light"; break;
            case DARK:  type_string = "dark";  break;
            case VERYDARK: type_string = "verydark"; break;
        }
        if (type_string == null) {
            Log.e(TAG, "unexpected theme type");
            return;
        }
        evaluateJavascript(String.format("set_theme(\"%s\");",
                escapeStringForJs(type_string)));
    }

    private void evaluateJavascript(String js_cmd) {
        Log.d(TAG, "evaluateJavascript(\"" + js_cmd + "\")");
        //webView.loadUrl("javascript:" + js_cmd);
        webView.evaluateJavascript(js_cmd, null);
    }

    public void loadState(List<CalcHistoryHelper.HistoryEntry> inputs) {
        Log.d(TAG, "loadState");
        clearOutputDisplay();
        for (CalcHistoryHelper.HistoryEntry input : inputs) {
            Log.v(TAG, String.format("loadState: input=%s, output=%s, outputType=%s",
                    input.tex_input, input.tex_output, input.output_type.name()));
            addOutputLine(input.tex_input);
            // tex_output should already have the equals
            switch (input.output_type) {
                case NORMAL: addOutputLine(input.tex_output); break;
                case ERROR_MSG: addOutputLineErr(input.tex_output); break;
            }
        }
    }

    public boolean isReady() {
        return this.isReady;
    }

    public void setReady() {
        this.isReady = true;
    }
}
