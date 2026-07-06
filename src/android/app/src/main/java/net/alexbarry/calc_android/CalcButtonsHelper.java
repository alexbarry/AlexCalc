package net.alexbarry.calc_android;

import android.content.Context;
import android.telecom.Call;
import android.util.Log;
import android.view.HapticFeedbackConstants;
import android.view.View;
import android.widget.Button;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.Callable;

public class CalcButtonsHelper {

	private static final String TAG = "CalcButtonsHelper";

	// Right now the whitespace is necessary or it can stick to either side of "1 m" to "ft"
    // we don't want "1 mto ft", "1 m toft" or "1 mtoft"
	//private static final String TO_UNITS_TOKEN = " to ";
    // actually now I'm adding whitespace before and after every unit, so that's good enough
	// 2026-07-06 actually now that I added degrees/minutes/seconds, I don't support spaces
	// yet between those, so I'm adding a space at the beginning of "to" again.
    private static final String TO_UNITS_TOKEN = " to";

	public boolean experimentalModeEnabled = false;

	// originally I was going to add this state to
	// update the buttons to show options after pressing "to (units)".
	// But I'm not sure if the buttons should care strictly about
	// previously pressed button, but rather the
	// input that precedes cursor.
	// This way, if you access previous input from history, or just
	// move your cursor, you'll get the new options.
	//private TokenInfo prevInputTokenSent = null;

	private boolean prevInputTokenIsToUnits() {
		//return this.prevInputTokenSent != null && prevInputTokenSent.token == TO_UNITS_TOKEN;
		String prevInputToken = callback.getPrevInputToken();
		return prevInputToken != null && prevInputToken.equals(TO_UNITS_TOKEN);
	}

	private DegMinSecState degMinSecState = DegMinSecState.DEG;

	private enum DegMinSecState {
		DEG,
		MIN,
		SEC,
	}

	public enum CallbackEvent {
		CLEAR,
        CLEAR_SCREEN,
		BKSP,
        DELETE,
		ENTER,
		OPEN_VARS,
		OPEN_UNITS,

		LEFT,
		UP,
		DOWN,
		RIGHT,

        BEGIN,
        END,

		SET_POLAR,
		SET_RECT,
		SET_DEGREES,
		SET_RADIANS,
		SET_GRADIANS,

		DIGIT_DEG,
		DIGIT_MIN,
		DIGIT_SEC,
    }

	public enum HapticSetting {
		FOLLOW_SYSTEM,
		ENABLED,
		DISABLED,
	}

	public interface ButtonCallback {
		public void onEvent(CallbackEvent event);
		public void onTokenAdd(TokenType type, String token);
		public boolean checkButtonAllowed(ButtonId btnId);
		public String getPrevInputToken();
	}

	private void onTokenAdd(TokenType type, String token) {
		callback.onTokenAdd(type, token);
		//this.prevInputTokenSent = new TokenInfo(type, token);
	}

    enum ButtonId {
        CLEAR,
        BKSP,
		DEG_MIN_SEC,
        UP,
        LEFT,
        DOWN,
        RIGHT,
        INV,
        POLAR_RECT,
        DEGREE_RAD,
        ALT,
        LOG10,
        SIN,
        COS,
        TAN,
        POW,
        VAR,
        LN,
        LPAREN,
        DELIM,
        RPAREN,
        DIV,
        UNITS,
        SQRT,
        NUM7,
        NUM8,
        NUM9,
        MULT,
        VAR1,
        PI,
        NUM4,
        NUM5,
        NUM6,
        SUB,
        ANS,
        E,
        NEG,
        NUM1,
        NUM2,
        NUM3,
        ADD,
        STO,
        IMG_UNIT,
        NUM0,
        DECIMAL,
        EXP,
        ENTER
    }

	class TokenInfo {
		public final TokenType type;
		public final String    token;
		public TokenInfo(TokenType type, String token) {
			this.type  = type;
			this.token = token;
		}
	}

    private Map<ButtonId, Integer> button_id_to_android_layout_elem_id = new HashMap<>();

	/** maps to a func returning what these buttons should make appear in the input text (e.g. "sin(") */
    private Map<ButtonId, Callable<TokenInfo>>  button_id_to_token_func = new HashMap<>();
	/** maps to a func returning what these buttons should display on them (e.g. "sin") */
    private Map<ButtonId, Callable<String>>  button_id_to_display_txt_func = new HashMap<>();

	private boolean alt_state = false;
	private boolean inv_state = false;
	private CalcAndroid.AngleMode angleMode = CalcAndroid.AngleMode.RADIAN;
	private boolean is_polar  = false;

    private Context context;
    private View view;
    private ButtonCallback callback;

	private HapticSetting hapticSetting = HapticSetting.FOLLOW_SYSTEM;

	private final TokenInfo TOKEN_DEG = new TokenInfo(TokenType.DIGIT, "deg");
	private final TokenInfo TOKEN_MIN = new TokenInfo(TokenType.DIGIT, "min");
	private final TokenInfo TOKEN_SEC = new TokenInfo(TokenType.DIGIT, "sec");

    public CalcButtonsHelper(ButtonCallback callback) {
        this.callback = callback;
        button_id_to_android_layout_elem_id.put(ButtonId.CLEAR,         R.id.button_clear);
        button_id_to_android_layout_elem_id.put(ButtonId.BKSP,          R.id.button_bksp);
		button_id_to_android_layout_elem_id.put(ButtonId.DEG_MIN_SEC,   R.id.button_deg_min_sec);
        button_id_to_android_layout_elem_id.put(ButtonId.UP,            R.id.button_up);
        button_id_to_android_layout_elem_id.put(ButtonId.LEFT,          R.id.button_left);
        button_id_to_android_layout_elem_id.put(ButtonId.DOWN,          R.id.button_down);
        button_id_to_android_layout_elem_id.put(ButtonId.RIGHT,         R.id.button_right);
        button_id_to_android_layout_elem_id.put(ButtonId.INV,           R.id.button_inv);
        button_id_to_android_layout_elem_id.put(ButtonId.POLAR_RECT,    R.id.button_polar);
        button_id_to_android_layout_elem_id.put(ButtonId.DEGREE_RAD,    R.id.button_degree);
        button_id_to_android_layout_elem_id.put(ButtonId.ALT,           R.id.button_alt);
        button_id_to_android_layout_elem_id.put(ButtonId.LOG10,         R.id.button_log10);
        button_id_to_android_layout_elem_id.put(ButtonId.SIN,           R.id.button_sin);
        button_id_to_android_layout_elem_id.put(ButtonId.COS,           R.id.button_cos);
        button_id_to_android_layout_elem_id.put(ButtonId.TAN,           R.id.button_tan);
        button_id_to_android_layout_elem_id.put(ButtonId.POW,           R.id.button_pow);
        button_id_to_android_layout_elem_id.put(ButtonId.VAR,           R.id.button_var);
        button_id_to_android_layout_elem_id.put(ButtonId.LN,            R.id.button_ln);
        button_id_to_android_layout_elem_id.put(ButtonId.LPAREN,        R.id.button_lparen);
        //button_id_to_android_layout_elem_id.put(ButtonId.DELIM,         R.id.button_delim);
        button_id_to_android_layout_elem_id.put(ButtonId.RPAREN,        R.id.button_rparen);
        button_id_to_android_layout_elem_id.put(ButtonId.DIV,           R.id.button_div);
        button_id_to_android_layout_elem_id.put(ButtonId.UNITS,         R.id.button_units);
        button_id_to_android_layout_elem_id.put(ButtonId.SQRT,          R.id.button_sqrt);
        button_id_to_android_layout_elem_id.put(ButtonId.NUM7,          R.id.button_7);
        button_id_to_android_layout_elem_id.put(ButtonId.NUM8,          R.id.button_8);
        button_id_to_android_layout_elem_id.put(ButtonId.NUM9,          R.id.button_9);
        button_id_to_android_layout_elem_id.put(ButtonId.MULT,          R.id.button_mult);
        button_id_to_android_layout_elem_id.put(ButtonId.VAR1,          R.id.button_var1);
        button_id_to_android_layout_elem_id.put(ButtonId.PI,            R.id.button_pi);
        button_id_to_android_layout_elem_id.put(ButtonId.NUM4,          R.id.button_4);
        button_id_to_android_layout_elem_id.put(ButtonId.NUM5,          R.id.button_5);
        button_id_to_android_layout_elem_id.put(ButtonId.NUM6,          R.id.button_6);
        button_id_to_android_layout_elem_id.put(ButtonId.SUB,           R.id.button_sub);
        button_id_to_android_layout_elem_id.put(ButtonId.ANS,           R.id.button_ans);
        button_id_to_android_layout_elem_id.put(ButtonId.E,             R.id.button_e);
        button_id_to_android_layout_elem_id.put(ButtonId.NEG,           R.id.button_neg);
        button_id_to_android_layout_elem_id.put(ButtonId.NUM1,          R.id.button_1);
        button_id_to_android_layout_elem_id.put(ButtonId.NUM2,          R.id.button_2);
        button_id_to_android_layout_elem_id.put(ButtonId.NUM3,          R.id.button_3);
        button_id_to_android_layout_elem_id.put(ButtonId.ADD,           R.id.button_add);
        button_id_to_android_layout_elem_id.put(ButtonId.STO,           R.id.button_sto);
        button_id_to_android_layout_elem_id.put(ButtonId.IMG_UNIT,      R.id.button_img_unit);
        button_id_to_android_layout_elem_id.put(ButtonId.NUM0,          R.id.button_0);
        button_id_to_android_layout_elem_id.put(ButtonId.DECIMAL,       R.id.button_decimal);
        button_id_to_android_layout_elem_id.put(ButtonId.EXP,           R.id.button_exp);
        button_id_to_android_layout_elem_id.put(ButtonId.ENTER,         R.id.button_enter);

		TokenInfo sin   = new TokenInfo(TokenType.FUNC_CALL, "sin(");
		TokenInfo asin  = new TokenInfo(TokenType.FUNC_CALL, "asin(");
		TokenInfo sinh  = new TokenInfo(TokenType.FUNC_CALL, "sinh(");
		TokenInfo asinh = new TokenInfo(TokenType.FUNC_CALL, "asinh(");

		TokenInfo cos   = new TokenInfo(TokenType.FUNC_CALL, "cos(");
		TokenInfo acos  = new TokenInfo(TokenType.FUNC_CALL, "acos(");
		TokenInfo cosh  = new TokenInfo(TokenType.FUNC_CALL, "cosh(");
		TokenInfo acosh = new TokenInfo(TokenType.FUNC_CALL, "acosh(");

		TokenInfo tan   = new TokenInfo(TokenType.FUNC_CALL, "tan(");
		TokenInfo atan  = new TokenInfo(TokenType.FUNC_CALL, "atan(");
		TokenInfo tanh  = new TokenInfo(TokenType.FUNC_CALL, "tanh(");
		TokenInfo atanh = new TokenInfo(TokenType.FUNC_CALL, "atanh(");

		TokenInfo sqrt  = new TokenInfo(TokenType.FUNC_CALL, "sqrt(");
		TokenInfo pow2  = new TokenInfo(TokenType.OTHER,     "^2");
		TokenInfo cbrt  = new TokenInfo(TokenType.FUNC_CALL, "cbrt(");
		TokenInfo pow3  = new TokenInfo(TokenType.OTHER,     "^3");

		// TODO these "TokenInfo" classes should also contain the button label, rather than having
		// two separate maps for both
        button_id_to_token_func.put(ButtonId.LOG10,         getTokenInv(new TokenInfo(TokenType.FUNC_CALL, "log("), new TokenInfo(TokenType.OTHER, "10^(")));
        //button_id_to_token_func.put(ButtonId.SIN,           getTokenInvAlt(sin, asin, sinh, asinh));
        //button_id_to_token_func.put(ButtonId.COS,           getTokenInvAlt(cos, acos, cosh, acosh));
        //button_id_to_token_func.put(ButtonId.TAN,           getTokenInvAlt(tan, atan, tanh, atanh));
		button_id_to_token_func.put(ButtonId.SIN,           getTokenInv(sin, asin));
		button_id_to_token_func.put(ButtonId.COS,           getTokenInv(cos, acos));
		button_id_to_token_func.put(ButtonId.TAN,           getTokenInv(tan, atan));

        button_id_to_token_func.put(ButtonId.POW,           getTokenStateless(new TokenInfo(TokenType.OP,"^")));
        button_id_to_token_func.put(ButtonId.LN,            getTokenInv(new TokenInfo(TokenType.FUNC_CALL,"ln("), new TokenInfo(TokenType.FUNC_CALL,"e^(")));
        button_id_to_token_func.put(ButtonId.LPAREN,        getTokenStateless(new TokenInfo(TokenType.PAREN_OPEN,"(")));
        button_id_to_token_func.put(ButtonId.DELIM,         getTokenStateless(new TokenInfo(TokenType.OTHER,",")));
        button_id_to_token_func.put(ButtonId.RPAREN,        getTokenStateless(new TokenInfo(TokenType.PAREN_CLOSE,")")));
        button_id_to_token_func.put(ButtonId.DIV,           getTokenStateless(new TokenInfo(TokenType.OP,"/")));
        button_id_to_token_func.put(ButtonId.SQRT,          getTokenInvAlt(sqrt, pow2, cbrt, pow3));
        button_id_to_token_func.put(ButtonId.MULT,          getTokenStateless(new TokenInfo(TokenType.OP,"*")));
        button_id_to_token_func.put(ButtonId.VAR1,          getTokenAlt(new TokenInfo(TokenType.VAR,"x"), new TokenInfo(TokenType.VAR,"y")));
        button_id_to_token_func.put(ButtonId.PI,            getTokenAlt(new TokenInfo(TokenType.VAR,"pi"), new TokenInfo(TokenType.VAR,"z")));
        button_id_to_token_func.put(ButtonId.SUB,           getTokenStateless(new TokenInfo(TokenType.OP,"-")));
        button_id_to_token_func.put(ButtonId.ANS,           getTokenStateless(new TokenInfo(TokenType.VAR,"ans")));
        button_id_to_token_func.put(ButtonId.E,             getTokenAlt(new TokenInfo(TokenType.VAR,"e"), new TokenInfo(TokenType.VAR,"theta")));
        button_id_to_token_func.put(ButtonId.ADD,           getTokenStateless(new TokenInfo(TokenType.OP,"+")));
        button_id_to_token_func.put(ButtonId.STO,           getTokenStateless(new TokenInfo(TokenType.OTHER,"->")));

        button_id_to_token_func.put(ButtonId.NEG,           getTokenStateless(new TokenInfo(TokenType.DIGIT,"-")));
        button_id_to_token_func.put(ButtonId.NUM0,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"0")));
        button_id_to_token_func.put(ButtonId.NUM1,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"1")));
        button_id_to_token_func.put(ButtonId.NUM2,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"2")));
        button_id_to_token_func.put(ButtonId.NUM3,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"3")));
        button_id_to_token_func.put(ButtonId.NUM4,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"4")));
        button_id_to_token_func.put(ButtonId.NUM5,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"5")));
        button_id_to_token_func.put(ButtonId.NUM6,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"6")));
        //button_id_to_token_func.put(ButtonId.NUM7,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"7")));
		button_id_to_token_func.put(ButtonId.NUM7,          getTokenChangesOnToUnits(new TokenInfo(TokenType.DIGIT, "7"), new TokenInfo(TokenType.OTHER, " deg")));
        //button_id_to_token_func.put(ButtonId.NUM8,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"8")));
		button_id_to_token_func.put(ButtonId.NUM8,          getTokenChangesOnToUnits(new TokenInfo(TokenType.DIGIT, "8"), new TokenInfo(TokenType.OTHER, " deg'")));
        //button_id_to_token_func.put(ButtonId.NUM9,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"9")));
		button_id_to_token_func.put(ButtonId.NUM9,          getTokenChangesOnToUnits(new TokenInfo(TokenType.DIGIT, "9"), new TokenInfo(TokenType.OTHER, " deg''")));

        button_id_to_token_func.put(ButtonId.IMG_UNIT,      getTokenAlt(new TokenInfo(TokenType.VAR,"i"), new TokenInfo(TokenType.OTHER,"angle")));
        button_id_to_token_func.put(ButtonId.DECIMAL,       getTokenStateless(new TokenInfo(TokenType.DIGIT,".")));
        button_id_to_token_func.put(ButtonId.EXP,           getTokenStateless(new TokenInfo(TokenType.DIGIT,"E")));

        button_id_to_display_txt_func.put(ButtonId.LOG10,         getStringInv("log10", "10^x"));
        //button_id_to_display_txt_func.put(ButtonId.SIN,           getStringInvAlt("sin", "asin", "sinh", "asinh"));
        //button_id_to_display_txt_func.put(ButtonId.COS,           getStringInvAlt("cos", "acos", "cosh", "acosh"));
        //button_id_to_display_txt_func.put(ButtonId.TAN,           getStringInvAlt("tan", "atan", "tanh", "atanh"));
		button_id_to_display_txt_func.put(ButtonId.SIN,           getStringInv("sin", "asin"));
		button_id_to_display_txt_func.put(ButtonId.COS,           getStringInv("cos", "acos"));
		button_id_to_display_txt_func.put(ButtonId.TAN,           getStringInv("tan", "atan"));
        button_id_to_display_txt_func.put(ButtonId.LN,            getStringInv("ln", "e^x"));
        button_id_to_display_txt_func.put(ButtonId.SQRT,          getStringInvAlt("sqrt", "x^2", "cbrt", "x^3"));
        button_id_to_display_txt_func.put(ButtonId.VAR1,          getStringAlt("x", "y"));
        button_id_to_display_txt_func.put(ButtonId.PI,            getStringAlt("pi", "z"));
        button_id_to_display_txt_func.put(ButtonId.E,             getStringAlt("e", "theta"));

        button_id_to_display_txt_func.put(ButtonId.IMG_UNIT,      getStringAlt("i", "angle"));
        button_id_to_display_txt_func.put(ButtonId.LEFT,          getStringAndroidAlt(R.string.arrow_left,  R.string.arrow_begin));
        button_id_to_display_txt_func.put(ButtonId.RIGHT,         getStringAndroidAlt(R.string.arrow_right, R.string.arrow_end));
        button_id_to_display_txt_func.put(ButtonId.CLEAR,         getStringAndroidAlt(R.string.clear,       R.string.clear_screen));
        button_id_to_display_txt_func.put(ButtonId.BKSP,          getStringAndroidAlt(R.string.bksp,        R.string.delete));
		button_id_to_display_txt_func.put(ButtonId.DEG_MIN_SEC,   getStringDegMinSec());
		button_id_to_display_txt_func.put(ButtonId.UNITS,         getStringAndroidAlt(R.string.units,       R.string.to_units));

		button_id_to_display_txt_func.put(ButtonId.NUM7,          getStringAndroidChangesOnToUnits(R.string.num7, R.string.btn_text_deg));
        button_id_to_display_txt_func.put(ButtonId.NUM8,          getStringAndroidChangesOnToUnits(R.string.num8, R.string.btn_text_deg_min));
		button_id_to_display_txt_func.put(ButtonId.NUM9,          getStringAndroidChangesOnToUnits(R.string.num9, R.string.btn_text_deg_min_sec));


    }

    public void viewReady(Context context, View v) {
		this.context = context;
        this.view = v;
        this.addButtonListeners();
    }

	public void setAngleMode(CalcAndroid.AngleMode angleMode) {
		this.angleMode = angleMode;
		updateDegreeRadBtnText();
	}

	public void setPolar(boolean is_polar) {
    	this.is_polar = is_polar;
    	updatePolarRectBtnText();
	}

	private void addButtonListeners() {
		for (final ButtonId internal_btn_id : button_id_to_android_layout_elem_id.keySet()) {
			Integer android_btn_id = button_id_to_android_layout_elem_id.get(internal_btn_id);
			Button btn = (Button)view.findViewById(android_btn_id);
			if (btn == null) {
				continue;
			}
			if (hapticSetting != HapticSetting.DISABLED) {
				btn.setHapticFeedbackEnabled(true);
			}
			btn.setOnClickListener(new View.OnClickListener() {
			    @Override
				public void onClick(View v) {
					if (hapticSetting != HapticSetting.DISABLED) {
						int flags = 0;
						switch (hapticSetting) {
							case ENABLED: flags = HapticFeedbackConstants.FLAG_IGNORE_GLOBAL_SETTING; break;

							// May not need this, but I can't figure out how to enable the global setting
							// on my phone
							case FOLLOW_SYSTEM: flags = HapticFeedbackConstants.FLAG_IGNORE_VIEW_SETTING; break;
						}
						v.performHapticFeedback(HapticFeedbackConstants.VIRTUAL_KEY, flags);
					}
					handleButtonEvent(internal_btn_id);
				}
			});
		}

			
	}

	private Callable<TokenInfo> getTokenStateless(final TokenInfo s) {
		return new Callable<TokenInfo> () {
            public TokenInfo call() {
                return s;
            }
        };
	}

	private Callable<TokenInfo> getTokenChangesOnToUnits(final TokenInfo normal, final  TokenInfo onToUnits) {
		return new Callable<TokenInfo> () {
			public TokenInfo call() {
				if (CalcButtonsHelper.this.experimentalModeEnabled && CalcButtonsHelper.this.prevInputTokenIsToUnits()) {
					return onToUnits;
				} else {
					return normal;
				}
			}
		};
	}

	private Callable<String> getStringAndroidChangesOnToUnits(final int normal, final int onToUnits) {
		return new Callable<String> () {
			public String call() {
				if (CalcButtonsHelper.this.experimentalModeEnabled && CalcButtonsHelper.this.prevInputTokenIsToUnits()) {
					return context.getString(onToUnits);
				} else {
					return context.getString(normal);
				}
			}
		};
	}

	private Callable<String> getStringDegMinSec() {
		return new Callable<String> () {
			public String call() {
				switch (degMinSecState) {
					case DEG: return context.getString(R.string.btn_text_deg);
					case MIN: return context.getString(R.string.btn_text_min);
					case SEC: return context.getString(R.string.btn_text_sec);
				}
				Log.e(TAG, "unhandled deg min sec state");
				return context.getString(R.string.btn_text_deg);
			}
		};
	}

	private Callable<TokenInfo> getTokenInv(final TokenInfo normal, final TokenInfo inv) {
		return new Callable<TokenInfo> () {
            public TokenInfo call() {
				if (!inv_state) {
					return normal;
				} else {
					return inv;
				}
			}
		};
	}

	private Callable<TokenInfo> getTokenAlt(final TokenInfo normal, final TokenInfo alt) {
		return new Callable<TokenInfo> () {
            public TokenInfo call() {
				if (!alt_state) {
					return normal;
				} else {
					return alt;
				}
			}
		};
	}

	private Callable<TokenInfo> getTokenInvAlt(final TokenInfo     normal, final TokenInfo     inv_normal,
	                                           final TokenInfo alt_normal, final TokenInfo alt_inv_normal) {
		return new Callable<TokenInfo> () {
            public TokenInfo call() {
				if (!alt_state) {
					if (!inv_state) {
						return normal;
					} else {
						return inv_normal;
					}
				} else {
					if (!inv_state) {
						return alt_normal;
					} else {
						return alt_inv_normal;
					}
				}
			}
		};
	}

	private Callable<String> getStringInv(final String normal, final String inv) {
		return new Callable<String> () {
            public String call() {
				if (!inv_state) {
					return normal;
				} else {
					return inv;
				}
			}
		};
	}

	private Callable<String> getStringAlt(final String normal, final String alt) {
		return new Callable<String> () {
            public String call() {
				if (!alt_state) {
					return normal;
				} else {
					return alt;
				}
			}
		};
	}

	private Callable<String> getStringAndroidAlt(final int normal, final int alt) {
		return new Callable<String> () {
            public String call() {
				if (!alt_state) {
					return context.getString(normal);
				} else {
					return context.getString(alt);
				}
			}
		};
	}

	private Callable<String> getStringInvAlt(final String     normal, final String     inv_normal,
	                                         final String alt_normal, final String alt_inv_normal) {
		return new Callable<String> () {
            public String call() {
				if (!alt_state) {
					if (!inv_state) {
						return normal;
					} else {
						return inv_normal;
					}
				} else {
					if (!inv_state) {
						return alt_normal;
					} else {
						return alt_inv_normal;
					}
				}
			}
		};
	}


	private void handleButtonEvent(ButtonId btn_id) {
		if (!callback.checkButtonAllowed(btn_id)) {
			return;
		}
        sendButtonEventToCallback(btn_id);
		Log.i(TAG, String.format("handleButtonEvent(btn_id=%s); prevInputTokenIsToUnits=%b", btn_id.name(), prevInputTokenIsToUnits()));
		handleBtnPressDegMinSec(callback, btn_id);
        if (btn_id != ButtonId.INV && btn_id != ButtonId.ALT) {
            this.alt_state = false;
            this.inv_state = false;
            updateButtonText();
        }
    }

	private CalcAndroid.AngleMode nextAngleMode(CalcAndroid.AngleMode angleMode) {
		switch (angleMode) {
			case RADIAN: return CalcAndroid.AngleMode.DEGREE;
			case DEGREE:
				if (experimentalModeEnabled) {
					return CalcAndroid.AngleMode.GRADIAN;
				} else {
					return CalcAndroid.AngleMode.RADIAN;
				}

			case GRADIAN: return CalcAndroid.AngleMode.RADIAN;
		}
		throw new RuntimeException();
	}

    private void sendButtonEventToCallback(ButtonId btn_id) {
		switch(btn_id) {
			case ENTER:
				callback.onEvent(CallbackEvent.ENTER);
				return;
			case VAR:
				callback.onEvent(CallbackEvent.OPEN_VARS);
				return;
			case UNITS:
				if (!this.alt_state) {
					callback.onEvent(CallbackEvent.OPEN_UNITS);
				} else {
					onTokenAdd(TokenType.OTHER, TO_UNITS_TOKEN);
				}
				return;

			case CLEAR:
				if (!this.alt_state) {
					callback.onEvent(CallbackEvent.CLEAR);
				} else {
					callback.onEvent(CallbackEvent.CLEAR_SCREEN);
				}
				return;

			case BKSP:
				if (!this.alt_state) {
					callback.onEvent(CallbackEvent.BKSP);
				} else {
					callback.onEvent(CallbackEvent.DELETE);
				}
				return;

			case LEFT:
				if (!this.alt_state) {
					callback.onEvent(CallbackEvent.LEFT);
				} else {
					callback.onEvent(CallbackEvent.BEGIN);
				}
				return;
			case RIGHT:
				if (!this.alt_state) {
					callback.onEvent(CallbackEvent.RIGHT);
				} else {
					callback.onEvent(CallbackEvent.END);
				}
				return;
			case UP:
				callback.onEvent(CallbackEvent.UP);
				return;
			case DOWN:
				callback.onEvent(CallbackEvent.DOWN);
				return;

			case POLAR_RECT: {
				this.is_polar = !this.is_polar;
				CallbackEvent event;
				if (this.is_polar) {
					event = CallbackEvent.SET_POLAR;
				} else {
					event = CallbackEvent.SET_RECT;
				}
				updatePolarRectBtnText();
				callback.onEvent(event);
				return;
			}

			case DEGREE_RAD: {
				//this.is_degree = !this.is_degree;
				this.angleMode = nextAngleMode(this.angleMode);
				CallbackEvent event;
				switch (this.angleMode) {
					case RADIAN:
						event = CallbackEvent.SET_RADIANS;
						break;
					case DEGREE:
						event = CallbackEvent.SET_DEGREES;
						break;
					case GRADIAN:
						event = CallbackEvent.SET_GRADIANS;
						break;
					default:
						throw new RuntimeException();
				}
				updateDegreeRadBtnText();
				callback.onEvent(event);
				return;
			}
		}

		if (button_id_to_token_func.containsKey(btn_id)) {
			try {
				TokenInfo info = button_id_to_token_func.get(btn_id).call();
				onTokenAdd(info.type, info.token);
			} catch (Exception ex) {
				Log.e(TAG, "btn id " + btn_id + " caused exception", ex);
			}
			return;
		}

		switch(btn_id) {
			case ALT:
				this.alt_state = !this.alt_state;
				updateButtonText();
				return;

			case INV:
				this.inv_state = !this.inv_state;
				updateButtonText();
				return;
		}

		Log.e(TAG, "Unhandled button id: " + btn_id);
	}

	private TokenInfo getTokenFromBtnId(ButtonId btnId) {
		try {
			return button_id_to_token_func.get(btnId).call();
		} catch (Exception ex) {
			Log.e(TAG, String.format("handleBtnPressDegMinSec: button id %s caused exception", btnId));
			return null;
		}
	}

	private void handleBtnPressDegMinSec(ButtonCallback callback, ButtonId btnId) {
		TokenInfo token = getTokenFromBtnId(btnId);

		if (btnId == ButtonId.DEG_MIN_SEC) {
			switch (degMinSecState) {
				case DEG: {
					callback.onEvent(CallbackEvent.DIGIT_DEG);
					degMinSecState = DegMinSecState.MIN;
					break;
				}
				case MIN: {
					callback.onEvent(CallbackEvent.DIGIT_MIN);
					degMinSecState = DegMinSecState.SEC;
					break;
				}
				case SEC: {
					callback.onEvent(CallbackEvent.DIGIT_SEC);
					degMinSecState = DegMinSecState.DEG;
					break;
				}
			}
		} else if (token != null && token.type == TokenType.DIGIT) {
			// do nothing.
			// Main flow for entering degrees, minutes, seconds is to
			// enter a number first, followed by degrees, then another number, then minutes, etc
		} else if (btnId == ButtonId.BKSP || btnId == ButtonId.LEFT || btnId == ButtonId.RIGHT) {
			// do nothing.
			// Users can move cursor around without resetting deg/min/sec button.
		} else {
			this.degMinSecState = DegMinSecState.DEG;
		}
	}

	private void updatePolarRectBtnText() {
    	if (!this.is_polar) {
			setButtonText(R.id.button_polar, R.string.polar);
		} else {
			setButtonText(R.id.button_polar, R.string.rect);
		}
	}

	private void updateDegreeRadBtnText() {
		switch (this.angleMode) {
			case RADIAN:
				setButtonText(R.id.button_degree, R.string.radian);
			break;
			case DEGREE:
				setButtonText(R.id.button_degree, R.string.degree);
			break;
			case GRADIAN:
				setButtonText(R.id.button_degree, R.string.gradian);
				break;
		}
	}

	private void updateButtonText() {
		for (ButtonId internal_btn_id : button_id_to_display_txt_func.keySet()) {
			String btnText;
		    try {
			    btnText = button_id_to_display_txt_func.get(internal_btn_id).call();
			} catch (Exception ex) {
				Log.e(TAG, "btn id " + internal_btn_id + " get button text generated exception", ex);
				continue;
			}
			Integer android_btn_id = button_id_to_android_layout_elem_id.get(internal_btn_id);
			if (android_btn_id == null) {
				Log.e(TAG, "button id " + internal_btn_id + " not defined in layout elem id map");
				continue;
			}
			Button btn = (Button)view.findViewById(android_btn_id);
			if (btn != null) {
				btn.setText(btnText);
			}
		}
	}

	private void setButtonText(int android_btn_id, int android_string_id) {
		Button btn = (Button)view.findViewById(android_btn_id);
		String btnText = context.getString(android_string_id);
		if (btn != null) {
			btn.setText(btnText);
		}
	}

	public void setHapticSetting(HapticSetting hapticSetting) {
		Log.i(TAG, String.format("setHapticSetting is now %s", hapticSetting));
		this.hapticSetting = hapticSetting;
	}

	public void updateButtonsVisibility() {
		//int androidBtnId = button_id_to_android_layout_elem_id.get(ButtonId.DEG_MIN_SEC);
		int androidBtnId = R.id.button_deg_min_sec;
		View button = view.findViewById(androidBtnId);
		button.setVisibility(this.experimentalModeEnabled ? View.VISIBLE : View.INVISIBLE);
	}

}
