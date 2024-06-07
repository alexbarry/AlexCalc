package net.alexbarry.calc_android;

//import android.app.AlertDialog;
import androidx.appcompat.app.AlertDialog;
import com.google.android.material.dialog.MaterialAlertDialogBuilder;
import android.content.Context;
import android.content.DialogInterface;
import android.text.Html;
import android.text.Spanned;
import android.text.method.LinkMovementMethod;
import android.text.util.Linkify;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.TextView;

class AboutPopup {

    private static final String TAG = "AboutPopup";
    private AlertDialog dialog = null;
    private Runnable showLicenses = null;

    public void init(Runnable showLicenses) {
        this.showLicenses = showLicenses;
    }

    private AlertDialog build(Context context) {
        //AlertDialog.Builder builder = new AlertDialog.Builder(context);
        MaterialAlertDialogBuilder builder = new MaterialAlertDialogBuilder(context, R.style.MyMaterialAlertDialogBackground);

        builder.setTitle(R.string.about_title);
        LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View view = inflater.inflate(R.layout.text_view_about, null);
        TextView tv = (TextView)view.findViewById(R.id.text_view_about);
        CharSequence aboutText;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.N) {
            String htmlText = context.getString(R.string.about_text_html);
            aboutText = Html.fromHtml(htmlText, Html.FROM_HTML_MODE_COMPACT);
        } else {
            tv.setAutoLinkMask(Linkify.EMAIL_ADDRESSES | Linkify.WEB_URLS);
            aboutText = context.getString(R.string.about_text_raw);
        }
        tv.setLinksClickable(true);
        tv.setMovementMethod(LinkMovementMethod.getInstance());
        tv.setText(aboutText);
        builder.setView(view);

        builder.setPositiveButton(R.string.show_licenses, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                if (showLicenses != null) {
                    showLicenses.run();
                } else {
                    Log.e(TAG, "license runnable is null");
                }
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

    public void show(Context context) {
        if (dialog == null) {
            this.dialog = build(context);
        }
        dialog.show();
    }
}
