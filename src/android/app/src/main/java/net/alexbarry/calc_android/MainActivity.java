package net.alexbarry.calc_android;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.os.Bundle;

import com.google.android.material.dialog.MaterialAlertDialogBuilder;
import com.google.android.material.floatingactionbutton.FloatingActionButton;
import com.google.android.material.snackbar.Snackbar;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AppCompatDelegate;
import androidx.appcompat.widget.Toolbar;
import androidx.fragment.app.Fragment;
import androidx.navigation.fragment.NavHostFragment;
import androidx.preference.PreferenceManager;

import android.util.Log;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.webkit.WebView;

//import com.google.android.gms.oss.licenses.OssLicensesMenuActivity;
import com.mikepenz.aboutlibraries.LibsBuilder;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "CalcActivity";

    final SharedPreferences.OnSharedPreferenceChangeListener sharedPrefsListener =
            new SharedPreferences.OnSharedPreferenceChangeListener() {
                @Override
                public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String key) {
                    if (key.equals(getString(R.string.preference_key_theme_select))) {
                        Log.i(TAG, "updating theme due to shared preference change");
                        updateTheme(MainActivity.this);
                        updateNightMode(MainActivity.this);
                        MainActivity.this.recreate();
                    } else {
                        Log.d(TAG, String.format("unhandled pref \"%s\"", key));
                    }
                }
            };

    final LicensePopup.LicenseShowCallback licenseShowCallback = new LicensePopup.LicenseShowCallback() {
        @Override
        public void show_android_licenses() {
            // https://developers.google.com/android/guides/opensource#java
            String title = getString(R.string.open_source_licenses_title);
            /*
            OssLicensesMenuActivity.setActivityTitle(title);
            Intent showLicensesIntent = new Intent(MainActivity.this, OssLicensesMenuActivity.class);
            startActivity(showLicensesIntent);
             */
            LibsBuilder libsBuilder = new LibsBuilder();
            libsBuilder.start(MainActivity.this);
        }

        @Override
        public void show_other_licenses() {
            otherLicensesPopup.show(MainActivity.this, null);
        }
    };

    private final AboutPopup aboutPopup = new AboutPopup();
    private final LicensePopup licensePopup = new LicensePopup(licenseShowCallback);
    private final OtherLicensesPopup otherLicensesPopup = new OtherLicensesPopup();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate");

        /*
        updateTheme(this);
        updateNightMode(this);
         */
        super.onCreate(savedInstanceState);

        updateTheme(this);
        updateNightMode(this);


        setContentView(R.layout.activity_main);

        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        NavHostFragment navHostFragment = (NavHostFragment)getSupportFragmentManager().findFragmentById(R.id.nav_host_fragment);
        Fragment frag = navHostFragment.getChildFragmentManager().findFragmentById(R.id.FirstFragment);
        Log.i(TAG, "getting child fragment returned: " + frag);
        Log.i(TAG, "other attempt: " + getSupportFragmentManager().findFragmentById(R.id.FirstFragment));


        /*
        FloatingActionButton fab = findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG)
                        .setAction("Action", null).show();
            }
        });
         */

        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        prefs.registerOnSharedPreferenceChangeListener(sharedPrefsListener);


    }

    @Override
    public void onPause() {
        Log.d(TAG, "onPause");
        super.onPause();
        //Log.d(TAG, "onPause complete");
    }

    @Override
    public void onStop() {
        Log.d(TAG, "onStop");
        super.onStop();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            showPreferences();
            return true;
        } else if (id == R.id.action_about) {
            showAbout();
            return true;
        } else if (id == R.id.help_about) {
            showHelpPopup(MainActivity.this, null);
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    private void showPreferences() {
        NavHostFragment.findNavController(getSupportFragmentManager().findFragmentById(R.id.nav_host_fragment))
                .navigate(R.id.SettingsFragment);
    }

    private void showAbout() {
        aboutPopup.init(new Runnable() {
            @Override
            public void run() {
                licensePopup.show(MainActivity.this,null);
            }
        });
        aboutPopup.show(this);
    }

    private void updateTheme(Context context) {
        //AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_FOLLOW_SYSTEM);
        ThemeType theme = FirstFragment.getDesiredTheme(context);

        switch (theme) {
            case LIGHT:
            case DARK:
                setTheme(R.style.AppTheme);
                break;
            case VERYDARK:
                setTheme(R.style.Theme_VeryDark);
                break;
        }
    }

    private void updateNightMode(Context context) {
        ThemeType theme = FirstFragment.getDesiredTheme(context);
        switch(theme) {
            case LIGHT:
            //case VERYDARK:
                AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_NO);
                break;
            case DARK:
            case VERYDARK:
                AppCompatDelegate.setDefaultNightMode(AppCompatDelegate.MODE_NIGHT_YES);
                break;
        }
    }

    private void showHelpPopup(Context context, ViewGroup root) {
        MaterialAlertDialogBuilder builder = new MaterialAlertDialogBuilder(context, R.style.MyMaterialAlertDialogBackground);

        View view = View.inflate(context, R.layout.webview_wrapper_view, root);
        WebView webView = view.findViewById(R.id.web_view);
        webView.loadUrl("file:///android_asset/html/help.html");

        builder.setView(view);


        builder.setNegativeButton(R.string.back, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                // do nothing
            }
        });

        AlertDialog dialog = builder.create();
        dialog.show();
    }
}
