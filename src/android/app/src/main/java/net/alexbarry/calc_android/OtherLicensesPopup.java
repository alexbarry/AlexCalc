package net.alexbarry.calc_android;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.Context;
import android.content.DialogInterface;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebView;

class OtherLicensesPopup {

    private Dialog dialog = null;


    private AlertDialog build(Context context, ViewGroup root) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);

        View view = View.inflate(context, R.layout.webview_wrapper_view, root);
        WebView webView = view.findViewById(R.id.web_view);
        webView.loadUrl("file:///android_asset/html/combined_other_licenses.html");

        builder.setView(view);


        builder.setNegativeButton(R.string.back, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                // do nothing
            }
        });

        return builder.create();
    }

    public void show(Context context, ViewGroup root) {
        if (dialog == null) {
            this.dialog = build(context, root);
        }
        dialog.show();
    }
}
