package net.alexbarry.calc_android;

import android.content.Context;
import android.telecom.Call;
import android.util.Log;
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
    private static final String TO_UNITS_TOKEN = "to";

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
    }

	public interface ButtonCallback {
		public void onEvent(CallbackEvent event);
		public void onTokenAdd(TokenType type, String token);
	}

    private enum ButtonId {
        CLEAR,
        BKSP,
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
	private boolean is_degree = false;
	private boolean is_polar  = false;

    private Context context;
    private View view;
    private ButtonCallback callback;

    public CalcButtonsHelper(ButtonCallback callback) {
        this.callback = callback;
        button_id_to_android_layout_elem_id.put(ButtonId.CLEAR,         R.id.button_clear);
        button_id_to_android_layout_elem_id.put(ButtonId.BKSP,          R.id.button_bksp);
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
        button_id_to_android_layout_elem_id.put(ButtonId.DELIM,         R.id.button_delim);
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
        button_id_to_token_func.put(ButtonId.SQRT,          getTokenInv(new TokenInfo(TokenType.FUNC_CALL,"sqrt("), new TokenInfo(TokenType.OTHER,"^2")));
        button_id_to_token_func.put(ButtonId.MULT,          getTokenStateless(new TokenInfo(TokenType.OP,"*")));
        button_id_to_token_func.put(ButtonId.VAR1,          getTokenAlt(new TokenInfo(TokenType.VAR,"x"), new TokenInfo(TokenType.VAR,"y")));
        button_id_to_token_func.put(ButtonId.PI,            getTokenAlt(new TokenInfo(TokenType.VAR,"pi"), new TokenInfo(TokenType.VAR,"z")));
        button_id_to_token_func.put(ButtonId.SUB,           getTokenStateless(new TokenInfo(TokenType.OP,"-")));
        button_id_to_token_func.put(ButtonId.ANS,           getTokenStateless(new TokenInfo(TokenType.VAR,"ans")));
        button_id_to_token_func.put(ButtonId.E,             getTokenAlt(new TokenInfo(TokenType.VAR,"e"), new TokenInfo(TokenType.VAR,"theta")));
        button_id_to_token_func.put(ButtonId.ADD,           getTokenStateless(new TokenInfo(TokenType.OP,"+")));
        button_id_to_token_func.put(ButtonId.STO,           getTokenStateless(new TokenInfo(TokenType.OTHER,"->")));

        button_id_to_token_func.put(ButtonId.NUM0,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"0")));
        button_id_to_token_func.put(ButtonId.NUM1,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"1")));
        button_id_to_token_func.put(ButtonId.NUM2,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"2")));
        button_id_to_token_func.put(ButtonId.NUM3,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"3")));
        button_id_to_token_func.put(ButtonId.NUM4,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"4")));
        button_id_to_token_func.put(ButtonId.NUM5,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"5")));
        button_id_to_token_func.put(ButtonId.NUM6,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"6")));
        button_id_to_token_func.put(ButtonId.NUM7,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"7")));
        button_id_to_token_func.put(ButtonId.NUM8,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"8")));
        button_id_to_token_func.put(ButtonId.NUM9,          getTokenStateless(new TokenInfo(TokenType.DIGIT,"9")));

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
        button_id_to_display_txt_func.put(ButtonId.SQRT,          getStringInv("sqrt", "x^2"));
        button_id_to_display_txt_func.put(ButtonId.VAR1,          getStringAlt("x", "y"));
        button_id_to_display_txt_func.put(ButtonId.PI,            getStringAlt("pi", "z"));
        button_id_to_display_txt_func.put(ButtonId.E,             getStringAlt("e", "theta"));

        button_id_to_display_txt_func.put(ButtonId.IMG_UNIT,      getStringAlt("i", "angle"));
        button_id_to_display_txt_func.put(ButtonId.LEFT,          getStringAndroidAlt(R.string.arrow_left,  R.string.arrow_begin));
        button_id_to_display_txt_func.put(ButtonId.RIGHT,         getStringAndroidAlt(R.string.arrow_right, R.string.arrow_end));
        button_id_to_display_txt_func.put(ButtonId.CLEAR,         getStringAndroidAlt(R.string.clear,       R.string.clear_screen));
        button_id_to_display_txt_func.put(ButtonId.BKSP,          getStringAndroidAlt(R.string.bksp,        R.string.delete));
        button_id_to_display_txt_func.put(ButtonId.UNITS,         getStringAndroidAlt(R.string.units,       R.string.to_units));


    }

    public void viewReady(Context context, View v) {
		this.context = context;
        this.view = v;
        this.addButtonListeners();
    }

	public void setDegree(boolean is_degree) {
    	this.is_degree = is_degree;
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
			btn.setOnClickListener(new View.OnClickListener() {
			    @Override
				public void onClick(View v) {
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
        sendButtonEventToCallback(btn_id);
        if (btn_id != ButtonId.INV && btn_id != ButtonId.ALT) {
            this.alt_state = false;
            this.inv_state = false;
            updateButtonText();
        }
    }

    private void sendButtonEventToCallback(ButtonId btn_id) {
		switch(btn_id) {
			case ENTER:  callback.onEvent(CallbackEvent.ENTER);      return;
			case VAR:    callback.onEvent(CallbackEvent.OPEN_VARS);  return;
			case UNITS:
			    if (!this.alt_state) {
			        callback.onEvent(CallbackEvent.OPEN_UNITS);
                } else {
			        callback.onTokenAdd(TokenType.OTHER, TO_UNITS_TOKEN);
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
			case UP:     callback.onEvent(CallbackEvent.UP   );      return;
			case DOWN:   callback.onEvent(CallbackEvent.DOWN );      return;

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
				this.is_degree = !this.is_degree;
				CallbackEvent event;
				if (this.is_degree) {
					event = CallbackEvent.SET_DEGREES;
				} else {
					event = CallbackEvent.SET_RADIANS;
				}
				updateDegreeRadBtnText();
                callback.onEvent(event);
				return;
			}
		}

		if (button_id_to_token_func.containsKey(btn_id)) {
			try {
				TokenInfo info = button_id_to_token_func.get(btn_id).call();
				callback.onTokenAdd(info.type, info.token);
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

	private void updatePolarRectBtnText() {
    	if (!this.is_polar) {
			setButtonText(R.id.button_polar, R.string.polar);
		} else {
			setButtonText(R.id.button_polar, R.string.rect);
		}
	}

	private void updateDegreeRadBtnText() {
		if (!this.is_degree) {
			setButtonText(R.id.button_degree, R.string.degree);
		} else {
			setButtonText(R.id.button_degree, R.string.radian);
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
			btn.setText(btnText);
		}
	}

	private void setButtonText(int android_btn_id, int android_string_id) {
		Button btn = (Button)view.findViewById(android_btn_id);
		String btnText = context.getString(android_string_id);
		btn.setText(btnText);
	}

}
