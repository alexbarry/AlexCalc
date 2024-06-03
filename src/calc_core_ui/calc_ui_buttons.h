
/* used when determining if a multiplication symbol should be inserted
 * when a button is pressed following a previous button press */
enum btn_type_e {
	/* If two of these buttons are pressed, they could represent
     * a single value, so no multiplication symbol should
     * be implicitly added between them.
     * e.g.: [0-9], ".", "E", or "-"
     */
	BTN_TYPE_DIGIT,
	/* a function call */
	BTN_TYPE_FUNC,
	/* a variable */
	BTN_TYPE_VAR,

	/* the presence of this means that the next button pressed
     * may implicitly generated a multiplication symbol before it. */
	BTN_TYPE_PAREN_OPEN,
	BTN_TYPE_PAREN_CLOSE,

	/* a binary operator, excluding "-" since that can mean negation */
	BTN_TYPE_OP,
};

/**
 *
 * These enums should refer to the buttons themselves-- UI code
 * should be dumb and simply send these IDs to some common code
 * that converts them into input tokens and a token type.
 *
 * I considered having a separate button identifier that gets translated into
 * an "action", e.g. separate IDs for sin, sinh, and asin... but it seems
 * like a pain to have two giant lists of buttons/actions, which are mostly
 * a one to one mapping. (The benefit is that custom UIs could be made,
 * perhaps on strange form factors (I haven't even considered horizontal
 * phones yet), where totally different buttons are shown that don't need
 * an alt/inv mode.)
 *
 *       1       2       3       4       5       6
 * 1  [    clear     ][ bksp ]        [ up   ]
 * 2  [ inv  ][polar ][degree][ left ][ down ][right ]
 * 3  [ alt  ][log10 ][ sin  ][ cos  ][ tan  ][  ^   ]
 * 4  [ var  ][  ln  ][  (   ][  ,   ][  )   ][  /   ]
 * 5  [units ][ sqrt ][  7   ][  8   ][  9   ][  *   ]
 * 6  [  x   ][  pi  ][  4   ][  5   ][  6   ][  -   ]
 * 7  [ ans  ][  e   ][  1   ][  2   ][  3   ][  +   ]
 * 8  [store ][  i   ][  0   ][  .   ][ EXP  ][enter ]
 *
 * Changes in alt mode:
 *    - "clear" (clear input) becomes "clear screen" (clear output),
 *    - "left"  becomes "begin",
 *    - "right" becomes "end",
 *    - "units" becomes "to units" (meaning insert "to" token to change output
 *                      _to_ another unit),
 *    - "sin", "cos", "tan" switch to hyperbolic. If inv is also pressed then
 *        they are both inverse and hyperbolic ("asinh", "acosh", "atanh")
 *    - "x", "pi", and "e" switch to "y", "z", and "theta"
 *
 * Changes in inv mode:
 *    - "log10" becomes "10^"
 *    - "ln" becomes "e^"
 *    - "sqrt" becomes "^2"
 *    - "sin", "cos", "tan" switch to inverse sin/cos/tan. If alt is also pressed
 *      then they are both inverse and hyperbolic.
 */
enum btn_id_e {
	BTN_CLEAR,
	BTN_BKSP,
	BTN_INV,
	BTN_ALT,
	BTN_POLAR_RECT,
	BTN_DEGREE_RAD,
	BTN_ENTER,
	BTN_STO, // "store", meaning "->" in "3*4 -> x" so store the result of "3*4" in x

	BTN_UP,
	BTN_LEFT, // switches to "begin" in alt mode
	BTN_RIGHT, // switches to "end" in alt mode
	BTN_DOWN,

	BTN_VAR,      // opens vars popup
	BTN_UNITS,    // opens units popup.
	              // Sends "to" token in alt mode, for converting result _to_ units

	BTN_LOG10,
	BTN_SIN, // the 3 trig functions sin/cos/tan switch to
	BTN_COS, // hyperbolic, inverse, or both hyp + inverse in alt/inv modes
	BTN_TAN,
	BTN_SQRT,

	// "var_x" could become "last used variable" some day
	BTN_VAR_X, // switches to "y" in alt mode
	BTN_PI,    // switches to "z" in alt mode
	BTN_E,     // Euler's constant, not to be confused with capital "E" for EXP like 1e3
	           // "e" switches to "theta" in alt mode
	BTN_ANS, // last computed result, or last "answer"

	BTN_POW,
	BTN_DIV,
	BTN_MULT,
	BTN_SUB, // used for binary subtraction or unary negation
	BTN_ADD,

	BTN_PAREN_L,
	BTN_DELIM,
	BTN_PAREN_R,

	BTN_0,
	BTN_1,
	BTN_2,
	BTN_3,
	BTN_4,
	BTN_5,
	BTN_6,
	BTN_7,
	BTN_8,
	BTN_9,
	BTN_DECIMAL,
	BTN_EXP, // capital "E", for "2E3" meaning 2*10**3 (though the lower case version works too)
	BTN_IMAG_UNIT, // "i" or "j", sqrt(-1). Switches to "angle" in alt mode

	// while "angle" is kind like any char in "-123.4e5", I think
	// it needs to be treated like an operator for the purposes of
	// automatically inserting multiplication symbols
	BTN_ANGLE,
};
