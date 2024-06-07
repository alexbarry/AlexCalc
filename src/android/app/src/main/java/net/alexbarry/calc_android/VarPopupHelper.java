package net.alexbarry.calc_android;

import static net.alexbarry.calc_android.Utils.strMapToString;

import android.app.ActionBar;
import android.app.Activity;
import android.app.Dialog;
import android.content.DialogInterface;
import android.graphics.drawable.Drawable;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.widget.Button;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.view.ContextThemeWrapper;
import androidx.constraintlayout.widget.ConstraintLayout;

import com.google.android.material.dialog.MaterialAlertDialogBuilder;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.SortedMap;

public class VarPopupHelper {

	private static final String TAG = "VarPopupHelper";
	private static final Map<Integer, String> existing_btn_to_var_name = new HashMap<>();

	public interface Callback {
		public void onInsertVar(String varName);
		public void onDeleteVars();
	}

	static {
		existing_btn_to_var_name.put(R.id.btn_new_var_x,       "x");
		existing_btn_to_var_name.put(R.id.btn_new_var_y,       "y");
		existing_btn_to_var_name.put(R.id.btn_new_var_z,       "z");
		existing_btn_to_var_name.put(R.id.btn_new_var_theta,   "theta");

		existing_btn_to_var_name.put(R.id.btn_new_var_alpha,   "alpha");
		existing_btn_to_var_name.put(R.id.btn_new_var_beta,    "beta");
		existing_btn_to_var_name.put(R.id.btn_new_var_gamma,   "gamma");
		existing_btn_to_var_name.put(R.id.btn_new_var_epsilon, "epsilon");

		existing_btn_to_var_name.put(R.id.btn_new_var_a,       "a");
		existing_btn_to_var_name.put(R.id.btn_new_var_b,       "b");
		existing_btn_to_var_name.put(R.id.btn_new_var_c,       "c");
		existing_btn_to_var_name.put(R.id.btn_new_var_d,       "d");

		existing_btn_to_var_name.put(R.id.btn_new_var_r1,      "R_1");
		existing_btn_to_var_name.put(R.id.btn_new_var_v1,      "V_1");
		existing_btn_to_var_name.put(R.id.btn_new_var_i1,      "I_1");
		existing_btn_to_var_name.put(R.id.btn_new_var_L,       "L");

		existing_btn_to_var_name.put(R.id.btn_new_var_r2,      "R_2");
		existing_btn_to_var_name.put(R.id.btn_new_var_v2,      "V_2");
		existing_btn_to_var_name.put(R.id.btn_new_var_i2,      "I_2");
		existing_btn_to_var_name.put(R.id.btn_new_var_C,       "C");

		existing_btn_to_var_name.put(R.id.btn_new_var_omega,   "omega");
		existing_btn_to_var_name.put(R.id.btn_new_var_f,       "f");
		existing_btn_to_var_name.put(R.id.btn_new_var_p,       "P");
		existing_btn_to_var_name.put(R.id.btn_new_var_g,       "G");
	}

	private Tab activeTab = Tab.EXISTING_VAR_SEL;
	private Dialog dialog_existing_var;
	private Dialog dialog_new_var;
	private View existing_var_selector;
	private Button existing_var_selector_btn;
	private View new_var_selector;
	private TableRow existingVarSelectedTableRow = null;
	private Map<View, String> viewToVarNameMap = new HashMap<>();
	private EditText custom_var_edittext;
	private Button btn_insert_custom_var_name;
	private Activity activity;

	private final Callback onInsertVarCallback;


	public VarPopupHelper(Callback onInsertVarCallback) {
		this.onInsertVarCallback = onInsertVarCallback;
	}

	public void init(final Activity activity) {
		this.activity = activity;
        // TODO need an intermediate fragment or something (maybe some form of tabs?) to switch between "(new) var selector" and
        // existing vars. Maybe show existing vars first, with a link or text directing to the tab to define new ones
		MaterialAlertDialogBuilder builder_existing_var = new MaterialAlertDialogBuilder(activity, R.style.MyMaterialAlertDialogBackground);
		MaterialAlertDialogBuilder builder_new_var = new MaterialAlertDialogBuilder(activity, R.style.MyMaterialAlertDialogBackground);
		LayoutInflater inflater = activity.getLayoutInflater();

    	// TODO existing vars should have a button that should switch from existing_vars fragment
		//  to var_selector fragment (should be renamed to "new var selector")
    	this.existing_var_selector = inflater.inflate(R.layout.existing_vars_fragment, null);
		builder_existing_var.setView(existing_var_selector);
		this.new_var_selector = inflater.inflate(R.layout.var_selector_fragment, null);
		builder_new_var.setView(new_var_selector);

		this.custom_var_edittext = (EditText)new_var_selector.findViewById(R.id.custom_var_name_edittext);

		custom_var_edittext.setOnEditorActionListener(new TextView.OnEditorActionListener() {
			@Override
			public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
				if (actionId == EditorInfo.IME_ACTION_GO) {
					insert_custom_var();
					return true;
				} else {
					Log.w(TAG, String.format("unexpected EditorAction %d", actionId));
					return false;
				}
			}
		});

		existing_var_selector.findViewById(R.id.delete_vars_btn).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				showDeleteVarsConfirmationDialog();
			}
		});


		existing_var_selector.findViewById(R.id.new_var_btn).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				setActiveTab(Tab.NEW_VAR_DEFINE);
			}
		});
		existing_var_selector.findViewById(R.id.existing_var_cancel).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				hide();
			}
		});

		new_var_selector.findViewById(R.id.btn_insert_var_cancel).setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				setActiveTab(Tab.EXISTING_VAR_SEL);
			}
		});

		this.btn_insert_custom_var_name = (Button)new_var_selector.findViewById(R.id.btn_insert_custom_var_name);
		btn_insert_custom_var_name.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				insert_custom_var();
			}
		});

		custom_var_edittext.addTextChangedListener(new TextWatcher() {
			@Override
			public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

			@Override
			public void onTextChanged(CharSequence s, int start, int before, int count) {}

			@Override
			public void afterTextChanged(Editable s) {
				btn_insert_custom_var_name.setEnabled(s.length() > 0);
			}
		});


		existing_var_selector_btn = existing_var_selector.findViewById(R.id.existing_var_insert);
		existing_var_selector_btn.setEnabled(false);
		existing_var_selector_btn.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				onExistingVarInsertBtnPress();
			}
		});

		for (int id : existing_btn_to_var_name.keySet()) {
			final String var_name = existing_btn_to_var_name.get(id);
			new_var_selector.findViewById(id).setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					onInsertVarCallback.onInsertVar(var_name);
					//setActiveTab(Tab.EXISTING_VAR_SEL);
					activeTab = Tab.EXISTING_VAR_SEL;
					hide();
				}
			});
		}

		this.dialog_existing_var = builder_existing_var.create();
		this.dialog_new_var = builder_new_var.create();
	}

	public void open() {
    	//this.dialog.show();
		this.activeTab = Tab.EXISTING_VAR_SEL;
		existing_var_selector_btn.setEnabled(false);
		setActiveTab(activeTab);
	}

	public void hide() {
		this.dialog_existing_var.hide();
		this.dialog_new_var.hide();
		updateCustomVarNameInsertBtnEnabled();
	}

	public void setVars(SortedMap<String, String> vars) {
		Log.d(TAG, String.format("setVars: %s", strMapToString(vars)));

		TableLayout varTable = (TableLayout)this.existing_var_selector.findViewById(R.id.existing_var_table);
		/* while (varTable.getChildAt(0) != null) {
			View row = varTable.getChildAt(0);
			//varTable.removeV

		}*/
		varTable.removeAllViewsInLayout();
		viewToVarNameMap.clear();

		for (String varName : vars.keySet()) {
			String varValue = vars.get(varName);
			ContextThemeWrapper contextRow = new ContextThemeWrapper(activity, R.style.ExistingVarsRow);
			ContextThemeWrapper contextColName = new ContextThemeWrapper(activity, R.style.ExistingVarsColName);
			ContextThemeWrapper contextColVal = new ContextThemeWrapper(activity, R.style.ExistingVarsColVal);
			TableRow tableRow = new TableRow(contextRow);
			TextView varNameCell = new TextView(contextColName);
			TextView varValCell = new TextView(contextColVal);

			//tableRow.setBackground(R.);
			// NOTE that despite the style defining gravity=right and layout_weight=1,
			// it needs to be set here too.
			varValCell.setGravity(Gravity.RIGHT);
			TableRow.LayoutParams params = new TableRow.LayoutParams(TableLayout.LayoutParams.WRAP_CONTENT, TableLayout.LayoutParams.WRAP_CONTENT, 1);
			varValCell.setLayoutParams(params);

			varNameCell.setText(varName);
			varValCell.setText(varValue);

			tableRow.addView(varNameCell);
			tableRow.addView(varValCell);

			varTable.addView(tableRow);

			tableRow.setOnClickListener(new View.OnClickListener() {
				@Override
				public void onClick(View v) {
					onTableRowSelect((TableRow)v);
				}
			});
			tableRow.setClickable(true);
			viewToVarNameMap.put(tableRow, varName);
		}
		existing_var_selector_btn.setEnabled(false);

	}

	private void onTableRowSelect(TableRow tableRowSelected) {
		int unselectedColor = this.activity.getColor(R.color.windowBackground);
		int selectedColor = this.activity.getColor(R.color.btn_important);
		Drawable unselectedBackground = this.activity.getDrawable(android.R.drawable.btn_default);
		if (existingVarSelectedTableRow != null) {
			//existingVarSelectedTableRow.setBackground(this.existing_var_selector.getBackground());
			//existingVarSelectedTableRow.setBackgroundColor(-1);
			existingVarSelectedTableRow.setBackgroundColor(unselectedColor);
			existingVarSelectedTableRow = null;
		}

		tableRowSelected.setBackgroundColor(selectedColor);
		existingVarSelectedTableRow = tableRowSelected;
		existing_var_selector_btn.setEnabled(true);
	}

	private enum Tab {
		EXISTING_VAR_SEL,
		NEW_VAR_DEFINE,
	}

	private void setActiveTab(Tab tab) {
		hide();
		switch(tab) {
			case EXISTING_VAR_SEL: dialog_existing_var.show(); break;
			case NEW_VAR_DEFINE: dialog_new_var.show(); break;
		}

	}

	private void updateCustomVarNameInsertBtnEnabled() {
		btn_insert_custom_var_name.setEnabled(custom_var_edittext.length() > 0);
	}

	private void insert_custom_var() {
		String var_name = custom_var_edittext.getText().toString().trim();
		custom_var_edittext.getText().clear();
		if (var_name.length() > 0) {
			onInsertVarCallback.onInsertVar(var_name);
		}
		hide();
	}

	private void onExistingVarInsertBtnPress() {
		if (existingVarSelectedTableRow == null) {
			Log.e(TAG, "no row selected");
			return;
		}

		if (!viewToVarNameMap.containsKey(existingVarSelectedTableRow)) {
			Log.e(TAG, String.format("row %s has no entry", existingVarSelectedTableRow));
			return;
		}

		String varName = viewToVarNameMap.get(existingVarSelectedTableRow);

		onInsertVarCallback.onInsertVar(varName);
		hide();
	}

	private void showDeleteVarsConfirmationDialog() {
		AlertDialog.Builder builder = new AlertDialog.Builder(this.activity);
		builder.setTitle(activity.getString(R.string.delete_vars_popup_title));
		builder.setMessage(activity.getString(R.string.delete_vars_popup_msg));
		builder.setPositiveButton(R.string.yes, new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
				onInsertVarCallback.onDeleteVars();
			}
		});
		builder.setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
			@Override
			public void onClick(DialogInterface dialog, int which) {
			}
		});
		builder.show();
	}
}
