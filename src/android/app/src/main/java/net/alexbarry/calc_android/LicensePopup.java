package net.alexbarry.calc_android;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;

class LicensePopup {

    private static final String TAG = "LicensePopup";

    private final LicenseShowCallback callback;

    private AlertDialog dialog = null;

    interface LicenseShowCallback {
        public void show_android_licenses();
        public void show_other_licenses();
    }

    LicensePopup(LicenseShowCallback callback) {
        this.callback = callback;
    }



    private AlertDialog build(Context context, ViewGroup root) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);

        View view = View.inflate(context, R.layout.license_picker, root);
        builder.setView(view);

        view.findViewById(R.id.licenses_android_show).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                callback.show_android_licenses();
            }
        });

        view.findViewById(R.id.licenses_other_show).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                callback.show_other_licenses();
            }
        });

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
