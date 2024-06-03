package net.alexbarry.calc_android;

import android.app.Dialog;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.graphics.Color;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.WindowManager;
import android.view.inputmethod.EditorInfo;
import android.webkit.WebViewClient;
import android.widget.Button;
import android.widget.EditText;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.widget.PopupWindow;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.preference.PreferenceManager;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.SortedMap;
import java.util.TreeMap;

// TODO rename this to "CalcFragment" or something.
// Maybe split up buttons layout XML and stuff into their own fragment layouts?
// TODO implement history (with a new helper)
// TODO var creator/selector
// TODO units creator/selector
public class FirstFragment extends Fragment {

	private static final String TAG = "CalcFragment";

	private final static String PRESERVED_STATE_FILENAME = "calc_state.json";
	private static final int MAX_ERRS_ON_ENTER_PRESS = 5;

	private UnitSelectorHelper.EventListener unitSelectorEventListener = new UnitSelectorHelper.EventListener() {
		@Override
		public void onInsertUnit(String token) {
			// adding whitespace before and after the unit as a bit of a hack, so that
			// pressing [1] [m] [s] doesn't result in 1 ms, m and s are separate units
			//
			addToken(TokenType.OTHER, " " + token + " ", true);
			calcAndroid.addRecentlyUsedUnit(token);
			unitSelectorHelper.updateRecentlyUsedUnits(calcAndroid.getRecentlyUsedUnits());

		}

		@Override
		public void onClearRecentlyUsedUnits() {
			dbHelper.deleteRecentlyUsedUnits();
			calcAndroid.deleteRecentlyUsedUnits();
			unitSelectorHelper.updateRecentlyUsedUnits(calcAndroid.getRecentlyUsedUnits());
			// TODO note that if any variables have units in them, they will show up
			// in recently used units if the app is closed and re-opened.
			// This is because the way variables are loaded into the calculator from the saved DB
			// on init is by using the normal "calc" API that the user uses.
			// This could be fixed by adding a separate API or a parameter to the normal one
			// that avoids saving units referenced there into "recently used units".
		}
	};

	PersistentState savedState = null;

	private CalcAndroid calcAndroid;
	private CalcButtonsHelper calcButtonsHelper;
	private CalcInputHelper calcInputHelper;
	private CalcHistoryHelper calcHistoryHelper = new CalcHistoryHelper();
	private CalcOutputDisplayHelper calcOutputDisplayHelper = null;
	private UnitSelectorHelper unitSelectorHelper = new UnitSelectorHelper(unitSelectorEventListener);
	private CalcPastOutputValueHelper<CalcAndroid.Value> pastOutputsHelper = new CalcPastOutputValueHelper();
	private PersistentStateDbHelper dbHelper;
	private int errsFromEnterPress = 0;
	private final List<String> errsOnInit = new ArrayList<>();

	private VarPopupHelper.Callback onInsertVarCallback = new VarPopupHelper.Callback() {
		@Override
		public void onInsertVar(String varName) {
			addToken(TokenType.VAR, varName, false);
		}

		@Override
		public void onDeleteVars() {
			dbHelper.deleteVars();
			calcAndroid.deleteVars();
			varPopupHelper.setVars(calcAndroid.getVars());
		}
	};

	private VarPopupHelper varPopupHelper = new VarPopupHelper(onInsertVarCallback );

	private WebView outputDisplayWebview;
	private EditText inputEditText;
	View view = null;


	private final CalcButtonsHelper.ButtonCallback buttonCallback = new CalcButtonsHelper.ButtonCallback() {
		@Override
		public void onEvent(CalcButtonsHelper.CallbackEvent event) {
			switch(event) {
				case CLEAR:  calcInputHelper.clear();     break;
				case LEFT:   calcInputHelper.inc_pos(-1); break;
				case RIGHT:  calcInputHelper.inc_pos(1);  break;
				case BKSP:   calcInputHelper.backspace(); break;
				case DELETE: calcInputHelper.del();       break;
				case BEGIN:  calcInputHelper.jump_pos_to_back(); break;
				case END:    calcInputHelper.jump_pos_to_front(); break;
			}

			switch(event) {
				case UP:
				case DOWN:
					if (!calcHistoryHelper.isTraversingHistory()) {
						CalcHistoryHelper.HistoryEntry historyEntry = new CalcHistoryHelper.HistoryEntry();
						historyEntry.inputTokens = calcInputHelper.getCurrentInputTokens();
						calcHistoryHelper.setCurrentEntry(historyEntry);
					}
			}

			switch(event) {
				case UP: calcHistoryHelper.adjustHistoryPos(-1); break;
				case DOWN: calcHistoryHelper.adjustHistoryPos(1); break;
			}

			switch(event) {
				case UP:
				case DOWN:
					CalcHistoryHelper.HistoryEntry inputHistoryEntry = calcHistoryHelper.getHistoryEntry();
					if (inputHistoryEntry != null) {
						calcInputHelper.setInputTokens(inputHistoryEntry.inputTokens);
						// when changing history positions, always put the cursor at the end
						calcInputHelper.jump_pos_to_front();
						updateInputText();
						updateLatexWipDisplay();
					}
					break;
			}

			switch(event) {
				case LEFT:
				case RIGHT:
				case UP:
				case DOWN:
				case DELETE:
				case BKSP:
				case BEGIN:
				case END:
					updateInputText();
					updateLatexWipDisplay();
					break;
				case CLEAR:
					updateInputText();
					clearLatexWipDisplay();
			}



			switch (event) {
				case SET_POLAR:   calcAndroid.setPolar(true);   break;
				case SET_RECT:    calcAndroid.setPolar(false);  break;
				case SET_DEGREES: calcAndroid.setDegree(true);  break;
				case SET_RADIANS: calcAndroid.setDegree(false); break;
			}

			if (event == CalcButtonsHelper.CallbackEvent.SET_RADIANS ||
					event == CalcButtonsHelper.CallbackEvent.SET_DEGREES ||
					event == CalcButtonsHelper.CallbackEvent.SET_POLAR ||
					event == CalcButtonsHelper.CallbackEvent.SET_RECT) {
				Log.i(TAG, String.format("Input str is \"%s\", size:%d",
						calcInputHelper.get_input_str(), calcInputHelper.get_input_str().length()));
				if (calcInputHelper.get_input_str().length() > 0) {
					Log.i(TAG, String.format("input str is present, updating wip display"));
					updateLatexWipDisplay();
				} else {
					Log.i(TAG, "no input str present, so updating last output");
					reformatLastOutputLine();
				}
			}

			if (event == CalcButtonsHelper.CallbackEvent.CLEAR_SCREEN) {
				calcOutputDisplayHelper.clearOutputDisplay();
				updateInputText();
				updateLatexWipDisplay();
				dbHelper.clearHistoryEntries(0);
			}

			switch(event) {
				case OPEN_VARS: varPopupHelper.open(); break;
				case OPEN_UNITS: unitSelectorHelper.show(); break;
			}

			if (event == CalcButtonsHelper.CallbackEvent.ENTER) {
				handleEnter();
			}

		}

		@Override
		public void onTokenAdd(TokenType type, String token) {
			addToken(type, token, false);
		}
	};

	// This textWatcher is removed and re-added whenever the EditText is changed programmatically
	// So the code is only executed when the user manually edits the text here
	private TextWatcher inputTextWatcher = new TextWatcher() {
		@Override
		public void beforeTextChanged(CharSequence s, int start, int count, int after) {

		}

		@Override
		public void onTextChanged(CharSequence s, int start, int before, int count) {

		}

		@Override
		public void afterTextChanged(Editable s) {
			Log.d(TAG, String.format("afterTextChanged: %s", s.toString()));
			int pos = inputEditText.getSelectionStart();
			userDirectlyChangedEditText(s.toString(), pos);
		}
	};

	@Override
	public void onCreate(Bundle bundle) {
		Log.d(TAG, "onCreate");
		super.onCreate(bundle);
		this.dbHelper = new PersistentStateDbHelper(getContext());
	}

	@Override
	public void onPause() {
		Log.d(TAG, "onPause");
		super.onPause();
	}

	@Override
	public void onStop() {
		Log.d(TAG, "onStop");
		super.onStop();
	}

	@Override
	public void onActivityCreated(Bundle bundle) {
		Log.d(TAG, "onActivityCreated");
		super.onActivityCreated(bundle);
		//this.savedState = loadStateFromFile();
		try {
			this.savedState = dbHelper.getPersistentState();
		} catch (Exception ex) {
			Log.e(TAG, "Exception loading persistent state from DB", ex);
			addErr("Error when loading state: " + ex.toString());
			return;
		}
		if (this.savedState != null) {
			loadPersistentState(this.savedState);
			applySavedStateToOutputDisplay(this.savedState);
		}
		//flushErrs();
	}

	@Override
	public void onSaveInstanceState(Bundle bundle) {
		Log.d(TAG, "onSaveInstanceState");
		super.onSaveInstanceState(bundle);
		//saveStateToFile(getPersistentState());
	}

    @Override
    public View onCreateView(
            LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState
    ) {
        // Inflate the layout for this fragment
        return inflater.inflate(R.layout.fragment_first, container, false);
    }

    public void onViewCreated(@NonNull View view, Bundle savedInstanceState) {
		Log.d(TAG, "onViewCreated");
        super.onViewCreated(view, savedInstanceState);

        /*
        view.findViewById(R.id.button_display).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                NavHostFragment.findNavController(FirstFragment.this)
                        .navigate(R.id.action_FirstFragment_to_SecondFragment);
            }
        });
         */

		this.view = view;
		inputEditText = (EditText)view.findViewById(R.id.txt_input);
		inputEditText.addTextChangedListener(inputTextWatcher);
		// this forces the view to be 2 lines big, but they don't get used
		//inputEditText.setMinLines(2);
		inputEditText.setOnEditorActionListener(new TextView.OnEditorActionListener() {
			@Override
			public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {
				if (actionId == EditorInfo.IME_ACTION_GO) {
					handleEnter();
					return true;
				} else {
					Log.w(TAG, String.format("unexpected EditorAction %d", actionId));
					return false;
				}
			}
		});

        // I want to show the cursor.
        // On its own, requestFocus would show the cursor, but also
        // bring up the on screen keyboard.
        //editText.requestFocus();
        // but this line will supposedly not show the on screen keyboard?
        //editText.setShowSoftInputOnFocus(false);
        //editText.requestFocus();

        // Somehow, using all 3 of these will show the keyboard initially,
        // but it won't show up later.
        // Maybe the only way to make this work is to find a way to disable the keyboard entirely,
        // then have a button to let the user bring it up.
        //editText.setShowSoftInputOnFocus(false);
        //editText.setCursorVisible(true);
        //editText.requestFocus();

		outputDisplayWebview = view.findViewById(R.id.output_display_webivew);
        WebSettings webSettings = outputDisplayWebview.getSettings();
        // this seems to keep the webview from flashing white when dark mode is enabled
        outputDisplayWebview.setBackgroundColor(Color.argb(1,0,0,0));
        webSettings.setJavaScriptEnabled(true);
		this.calcOutputDisplayHelper = new CalcOutputDisplayHelper(outputDisplayWebview);
        //outputDisplayWebview.loadUrl("https://calc.alexbarry.net/dev");
        outputDisplayWebview.loadUrl("file:///android_asset/html/js_display.html");
        final Context context = getContext();
        outputDisplayWebview.setWebViewClient(new WebViewClient() {
        	@Override
        	public void onPageFinished(WebView view, String url) {
        		Log.d(TAG, "calcOutputDisplayWebview.onPageFinished");
				calcOutputDisplayHelper.setTheme(getDesiredTheme(context));
				if (savedState != null) {
					Log.d(TAG, "loading saved output state onPageFinished");
					applySavedStateToOutputDisplay(savedState);
				} else {
					Log.d(TAG, "no saved state found onPageFinished");
				}
				flushErrs();
				calcOutputDisplayHelper.setReady();
				calcOutputDisplayHelper.addOutputLineMsg(context.getString(R.string.small_msg_terms));
			}
		});
        this.calcInputHelper = new CalcInputHelper();
        // TODO clean up. Just testing for now
        this.calcButtonsHelper = new CalcButtonsHelper(buttonCallback);
		this.calcButtonsHelper.viewReady(getContext(), view);
		this.varPopupHelper.init(requireActivity());
		this.unitSelectorHelper.init(requireActivity());

		this.calcAndroid = new CalcAndroid();
		this.calcAndroid.init();

		if (this.savedState != null) {
			loadPersistentState(this.savedState);
		}
		//loadPersistentState(dbHelper.getPersistentState());


		//String unitInfo = CalcAndroid.getUnitInfo();
		List<CalcAndroid.UnitGroup> unitGroups = CalcAndroid.getUnitGroups();
		Log.i(TAG, "unitInfo: " + unitGroups);

		SortedMap<String, String> vars = calcAndroid.getVars();
		Log.i(TAG, "recvd vars" + vars);
		varPopupHelper.setVars(vars);
    }

    private void addToken(TokenType type, String token, boolean is_unit) {
		calcInputHelper.add_token(type, token, is_unit);
		updateInputText();
		updateLatexWipDisplay();
	}

    @Override
    public void onDestroyView() {
    	super.onDestroyView();
    	this.calcAndroid.delete();
	}

	void addErr(String msg) {
		Log.i(TAG, "addErr: " + msg);
		errsOnInit.add(msg);
		if (calcOutputDisplayHelper == null || !calcOutputDisplayHelper.isReady()) {
			Log.d(TAG, "calcOutputDisplayHelper is null, adding to queue");
		} else {
			Log.d(TAG, "calcOutputDisplayHelper is ready, flushing queue");
			flushErrs();
		}
	}

	private void flushErrs() {
		Log.i(TAG, String.format("flushErrs: errsOnInit len=%d", errsOnInit.size()));
		for (String err_msg : errsOnInit) {
			Log.i(TAG, "adding display warning from errsOnInit: " + err_msg);
			calcOutputDisplayHelper.addOutputLineErr(err_msg);
		}
		errsOnInit.clear();
	}



	private void setInputText(String output, int pos) {
		inputEditText.removeTextChangedListener(inputTextWatcher);
		try {
			inputEditText.setText(output);
			inputEditText.setSelection(pos);
		} finally {
			inputEditText.addTextChangedListener(inputTextWatcher);
		}
	}

	private void userDirectlyChangedEditText(String txt, int pos) {
    	Log.i(TAG, String.format("userDirectlyChangedEditText, pos: %d, txt: \"%s\"", pos, txt));
		calcInputHelper.setRawInputText(txt, pos);
		updateLatexWipDisplay();
	}

	private void updateInputText() {
		String input_str = calcInputHelper.get_input_str();
		int input_pos = calcInputHelper.get_input_pos();
		setInputText(input_str, input_pos);
	}

	private void updateLatexWipDisplay() {
		try {
			String input_str = calcInputHelper.get_input_str();
			int input_pos = calcInputHelper.get_input_pos();
			String tex = calcAndroid.to_latex(input_str, true, input_pos);
			calcOutputDisplayHelper.updateWipDisplay(tex);
		} catch (Exception ex) {
			calcOutputDisplayHelper.updateWipDisplay("\\text{err}");
			Log.e(TAG, "ex when updating wip display", ex);
		}
	}

	private void clearLatexWipDisplay() {
		try {
			calcOutputDisplayHelper.updateWipDisplay("");
		} catch (Exception ex) {
			Log.e(TAG, "ex when updating clearing display", ex);
		}
	}

	private void handleEnter() {
    	String input_str = calcInputHelper.get_input_str();
    	Log.i(TAG, String.format("processing input_str=\"%s\"", input_str));
    	boolean is_err;
    	String output;
    	String texInput = "LaTeX error";
    	try {
			JSONObject calcOutput = this.calcAndroid.calc(input_str);
			Log.i(TAG, "received CalcOutput: " + calcOutput.toString());
			texInput = this.calcAndroid.to_latex(input_str);
			int rc = calcOutput.getInt("rc");
			Log.d(TAG, String.format("received rc=%d", rc));
			if (rc == 0) {
				is_err = false;
				SortedMap<String, String> vars = calcAndroid.getVars();
				Log.i(TAG, "recvd vars" + vars);
				varPopupHelper.setVars(vars);
				output = calcOutput.getString("val_str");

				// TODO this should be done inside CalcAndroid.
				// CalcAndroid should return a structure without any references to JSON
				CalcAndroid.Value value = CalcAndroid.getValueFromCalcOutput(calcOutput);
				pastOutputsHelper.addOutputValue(value);
			} else {
				String err_str = calcOutput.getString("msg");
				output = err_str;
				is_err = true;
			}
		} catch (JSONException ex) {
    		is_err = true;
    		Log.e(TAG, "calcAndroid JSON exception", ex);
    		output = "err";
		}

        Log.d(TAG, String.format("ENTER; is_err:%b tex:%s, output:%s", is_err, texInput, output));
        calcOutputDisplayHelper.addOutputLine(texInput);
        if (!is_err) {
			output = formatOutputLine(output);
			calcOutputDisplayHelper.addOutputLine(output);
		} else {
        	calcOutputDisplayHelper.addOutputLineErr(output);
		}
        // TODO only do this on successful calc eval?
		CalcHistoryHelper.HistoryEntry historyEntry = new CalcHistoryHelper.HistoryEntry();
		historyEntry.inputTokens = calcInputHelper.getCurrentInputTokens();
		historyEntry.tex_input = texInput;
		historyEntry.tex_output = output;
		historyEntry.output_type = is_err ? CalcHistoryHelper.EntryType.ERROR_MSG : CalcHistoryHelper.EntryType.NORMAL;
		calcHistoryHelper.addHistoryEntry(historyEntry);
		calcHistoryHelper.setCurrentEntry(null);
		calcHistoryHelper.historyPosReset();
        calcOutputDisplayHelper.clearWipDisplay();
        calcInputHelper.clear();
        updateInputText();
		List<String> recentlyUsedUnits = calcAndroid.getRecentlyUsedUnits();
		unitSelectorHelper.updateRecentlyUsedUnits(recentlyUsedUnits);


		// TODO do all this in a background thread
		// Also maybe share a single reference to the writeable DB, if that makes it a difference?
		try {
			// TODO I haven't tested what happens if an exception is thrown
			Log.d(TAG, "updating DB");
			dbHelper.addInput(historyEntry);

			SortedMap<String, String> vars = calcAndroid.getVars();
			dbHelper.setVars(vars);


			for (String unit : recentlyUsedUnits) {
				dbHelper.addRecentlyUsedUnit(unit);
			}

			CalcAndroid.CalcData calcData = calcAndroid.getCalcdata();
			dbHelper.setDegree(calcData.is_degree);
			dbHelper.setPolar(calcData.is_polar);
			Log.d(TAG, "done updating DB");
		} catch (Exception ex) {
			Log.e(TAG, "Exception writing to DB on enter press", ex);
			errsFromEnterPress++;
			if (errsFromEnterPress <= MAX_ERRS_ON_ENTER_PRESS) {
				addErr("Error adding entry to persistent state: " + ex.toString());
				if (errsFromEnterPress == MAX_ERRS_ON_ENTER_PRESS) {
					addErr(String.format("%d errors from enter press, will stop logging to " +
							"main display", MAX_ERRS_ON_ENTER_PRESS));
				}
			}
		}
	}

	// TODO move to outputDisplayHelper
	static String formatOutputLine(String output) {
    	return String.format("= %s", output);
	}
	/**
	 * reformats the last output line for cases like switching between polar/rect, rad/degree,
	 * num decimal places changed,
	 */
	private void reformatLastOutputLine() {
		CalcAndroid.Value pastValue = pastOutputsHelper.getLastOutput();
		if (pastValue == null) {
			return;
		}
		try {
			String new_output_line = calcAndroid.format_output(pastValue);
			new_output_line = formatOutputLine(new_output_line);
			Log.i(TAG, String.format("updating last output {re:%f, im:%f} to new str: %s",
					pastValue.re, pastValue.im, new_output_line));
			calcOutputDisplayHelper.editLastOutputLine(new_output_line);
		} catch (JSONException ex) {
			Log.e(TAG, "JSONException in format_output", ex);
		}
	}

	static ThemeType getDesiredTheme(Context context) {
		final String DEFAULT_THEME = context.getString(R.string.colour_theme_follow_system);
		//SharedPreferences prefs = getActivity().getPreferences(Context.MODE_PRIVATE);
		SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
		String themePrefString = prefs.getString(context.getString(R.string.preference_key_theme_select), DEFAULT_THEME);

		final String THEME_SYSTEM = context.getString(R.string.colour_theme_follow_system);
		final String THEME_LIGHT = context.getString(R.string.colour_theme_light);
		final String THEME_DARK = context.getString(R.string.colour_theme_dark);
		final String THEME_VERYDARK = context.getString(R.string.colour_theme_verydark);

		Log.i(TAG, String.format("theme is %s", themePrefString));

		if (themePrefString.equals(THEME_SYSTEM)) {
			ThemeType theme = getSystemTheme(context);
			Log.i(TAG, String.format("following system theme of %s", theme.name()));
			return theme;
		} else if (themePrefString.equals(THEME_LIGHT)) { return ThemeType.LIGHT; }
		else if (themePrefString.equals(THEME_DARK)) { return ThemeType.DARK; }
		else if (themePrefString.equals(THEME_VERYDARK)) { return ThemeType.VERYDARK; }
		else {
			Log.e(TAG, String.format("unexpected theme pref %s", themePrefString));
			return getSystemTheme(context);
		}
	}

	static ThemeType getSystemTheme(Context context) {
		int currentNightMode = context.getResources().getConfiguration().uiMode & Configuration.UI_MODE_NIGHT_MASK;
		switch (currentNightMode) {
			case Configuration.UI_MODE_NIGHT_NO:  return ThemeType.LIGHT;
			case Configuration.UI_MODE_NIGHT_YES: return ThemeType.DARK;
		}
		Log.e(TAG, String.format("unexpected currentNightMode %#x", currentNightMode));
		return ThemeType.LIGHT;
	}

	void loadPersistentState(PersistentState state) {
		Log.d(TAG, "loadPersistentState");
		calcAndroid.setDegree(state.is_degree);
		calcAndroid.setPolar(state.is_polar);
		calcAndroid.setVars(state.vars);
		calcAndroid.setRecentlyUsedUnits(state.units);

		CalcAndroid.CalcData calcData = calcAndroid.getCalcdata();

		// Note that this one doesn't work when the view is created
		// calcOutputDisplayHelper.loadState(state.inputs);
		calcButtonsHelper.setDegree(state.is_degree);
		calcButtonsHelper.setPolar(state.is_polar);
		calcHistoryHelper.loadState(state.inputs);
		calcInputHelper.setInputTokens(state.currentInput);
		updateInputText();
		varPopupHelper.setVars(state.vars);
		//unitSelectorHelper.updateRecentlyUsedUnits(state.units);
		unitSelectorHelper.updateRecentlyUsedUnits(calcData.recentlyUsedUnits);
	}

	PersistentState getPersistentState() {
		PersistentState state = new PersistentState();
		CalcAndroid.CalcData calcData = calcAndroid.getCalcdata();
		state.is_polar = calcData.is_polar;
		state.is_degree = calcData.is_degree;
		state.units = calcAndroid.getRecentlyUsedUnits();
		state.vars = calcAndroid.getVars();
		state.inputs = calcHistoryHelper.getState();
		for (CalcHistoryHelper.HistoryEntry historyEntry : state.inputs) {
			Log.v(TAG, "recvd history entry " + historyEntry.tex_input);
		}
		state.currentInput = calcInputHelper.getCurrentInputTokens();
		return state;
	}


	/*
	private PersistentState loadStateFromFile() {
		File state_file = new File(getContext().getFilesDir(), PRESERVED_STATE_FILENAME);

		try {
			Log.d(TAG, "Reading state from file " + state_file.getAbsolutePath());
			InputStream f_instream = new FileInputStream(state_file);
			PersistentState state = PersistentStateFileHelper.GetStateFromFile(f_instream);
			f_instream.close();
			Log.d(TAG, "Successfully read state from file " + state_file.getCanonicalPath());
			return state;
		} catch (FileNotFoundException ex) {
			Log.i(TAG, "Saved state file not found (expected when app is launched for first time)");
		} catch (IOException | IllegalStateException ex) {
			Log.e(TAG, String.format("Error reading state from file %s", state_file.getAbsolutePath()), ex);
			this.addErr(String.format("Error reading prev state from file: %s", ex.toString()));
		}
		return null;
	}*/

	/*
	void saveStateToFile(PersistentState state) {
		String filename = PRESERVED_STATE_FILENAME;
		try {
			File state_file = new File(getContext().getFilesDir(), filename);
			Log.d(TAG, "Starting to write state to file " + state_file.getAbsolutePath());
			OutputStream f_outstream = new FileOutputStream(state_file);
			PersistentStateFileHelper.WriteStateToFile(state, f_outstream);
			f_outstream.flush();
			f_outstream.close();
			Log.d(TAG, String.format("successfully saved state to file %s", state_file.getAbsolutePath()));
		} catch (IOException ex) {
			Log.e(TAG, String.format("error saving state to file %s", filename), ex);
		}
	}*/

	private void applySavedStateToOutputDisplay(PersistentState state) {
		if (state != null) {
			calcOutputDisplayHelper.loadState(state.inputs);
			if (calcInputHelper.get_input_str().length() > 0) {
				updateLatexWipDisplay();
			}
		}
	}
}
