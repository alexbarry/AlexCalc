package net.alexbarry.calc_android;

import android.content.Context;
import android.os.Bundle;
import android.provider.Settings;
import android.util.Log;
import android.view.MenuItem;

import androidx.annotation.NonNull;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;

public class SettingsFragment extends PreferenceFragmentCompat {

    private static final String TAG = "SettingsFragment";

    interface SettingsCallbacks {
        public void downloadWebviewLogsToFile();
        public void shareWebviewLogsFile();
    }

    private SettingsCallbacks listener = null;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        setPreferencesFromResource(R.xml.main_prefs, rootKey);

        Preference downloadWebviewLogs = findPreference(getString(R.string.pref_key_download_webview_logs));
        if (downloadWebviewLogs != null) {
            downloadWebviewLogs.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference preference) {
                    if (listener != null) {
                        listener.downloadWebviewLogsToFile();
                    } else {
                        Log.w(TAG, "listener is null");
                    }
                    return true;
                }
            });
        }

        Preference shareWebviewLogs = findPreference(getString(R.string.pref_key_share_webview_logs));
        if (shareWebviewLogs != null) {
            shareWebviewLogs.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener() {
                @Override
                public boolean onPreferenceClick(Preference preference) {
                    if (listener != null) {
                        listener.shareWebviewLogsFile();
                    } else {
                        Log.w(TAG, "listener is null");
                    }
                    return true;
                }
            });
        }
    }

    @Override
    public void onAttach(@NonNull Context context) {
        super.onAttach(context);

        SettingsCallbacks callbacks = (SettingsCallbacks)context;
        if (callbacks == null) {
            Log.i(TAG, "Main activity does not implement SettingsCallbacks?");
            return;
        }
        this.listener = callbacks;
    }
}
