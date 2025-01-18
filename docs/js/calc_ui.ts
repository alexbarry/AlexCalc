//import {CalcUi, ui, set_on_tex_ready, calc_ui_unit_sel_set_visible, init_unit_sel, alexcalc_unit_referenced} from './calc_types.js';
import {CalcUi, CalcUnitSel, CalcParams, InputTokenT, CalcData, CalcState, CalcOutput} from './calc_types.js';

declare const ui: CalcUi;

export declare function set_on_tex_ready(callback: (raw_input: string, tex: string) => void): void;
export declare function calc_ui_unit_sel_set_visible(unit_sel: CalcUnitSel, is_visible: boolean): void;
export declare function init_unit_sel(unit_sel: CalcUnitSel): void;
export declare function alexcalc_unit_referenced(unit_str: string): void;

interface Token {
	str: string;
	type: TokenType;
}


const ANGLE_OP_SYMBOL = "angle"
const ENABLE_HYPERBOLIC_TRIG = false;

function init_ui_state() {
	return {
		/** "inverse" button, switches buttons e.g sine to arcsine */
		inv_state: false,
		/** "alternate" button, switches sin to sinh, i (img rect) to angle (img polar) */
		alt_state: false,

		polar_state: false,
		degree_state: false,

		show_raw_calc_io_state: false,

		show_var_display:  false,
		show_unit_display: false,

		/**
		 * one token per button press. Raw string that gets joined together to be
		 * fed to calculator engine.
		 * Not stored as string directly so backspace will undo a button
		 * press rather than a character */
		input_tokens: [],

		/**
		 * contains the raw calc input/output used to generated the LaTeX displays.
		 * Hidden by default but can be made visible with a debug setting */
		output_display_srcs: [],

		/**
		 *  if equal to length of input_history, user is not viewing history.
		 *  otherwise, user is viewing element of input_history
		 */
		input_history_pos: 0,

		/**
		 * new entries should be pushed to end of array
		 */ 
		input_history: [],

		/**
		 * in var popup, the button/table row that the user has selected.
		 * Should be inserted into input on "insert variable" button pressed,
		 * and cleared when the popup is closed (both from aborting or
		 * pressing "insert variable".
		 */
		var_btn_selected: null,

		new_var_btns_map: new Map(),

		/**
		 * position of cursor, with 0 being before the first element,
		 * length of input_tokens being after the last element.
		 * convert to string position by summing length of input_tokens from 0 to cursor_idx
		 */
		 cursor_idx: 0,

         show_raw_calc_io: false,
         show_about_popup: false,
	};
}

// TODO should combine this with the other token I defined, which includes TokenType
function InputToken(token_str: string, is_unit?: boolean): InputTokenT {
	if (is_unit == undefined) {
		is_unit = false;
	}
	let input_token = {
		str: token_str,
		is_unit: is_unit,
	};
	return input_token;
}

function get_calcstate(ui_state: CalcState): CalcParams {
	return { polar: ui_state.polar_state,
	         degree: ui_state.degree_state};
}

const BTN_ID_TO_GET_TOKEN_FUNC: Map<string, (ui: CalcUi) => Token> = new Map();

function ret_token_factory(token: Token): (ui: CalcUi) => Token {
	return function(ui) { return token; }
}

function get_trig_token(trig_base: string): (ui: CalcUi) => Token {
	return function(ui) {
		let hyp_suffix = "";
		let inv_prefix = "";
		if (ENABLE_HYPERBOLIC_TRIG && ui.state.alt_state) {
			hyp_suffix = "h";
		}
		if (ui.state.inv_state) {
			inv_prefix = "a";
		}
		const token_str = inv_prefix + trig_base + hyp_suffix + "(";

		return {
			str:  token_str,
			type: TokenType.FUNC_CALL,
		};
	};
}

function ret_token_factory_w_inv(token: Token, token_inv: Token): (ui: CalcUi) => Token {
	return function(ui) {
		if (!ui.state.inv_state) {
			return token;
		} else {
			return token_inv;
		}
	}
}

function ret_token_factory_w_alt(token: Token, token_alt: Token): (ui: CalcUi) => Token {
	return function(ui) {
		if (!ui.state.alt_state) {
			return token;
		} else {
			return token_alt;
		}
	}
}

function var_token_factory(var_btn_num: number): (ui: CalcUi) => Token {
	// TODO make this always point to some of the last used variables,
	// defined in ui.state
	return function (ui) {
		if (!ui.state.alt_state) {
			if (var_btn_num == 1) {
				return { str: "x", type: TokenType.VAR };
			} else if (var_btn_num == 2) {
				return { str: "y", type: TokenType.VAR };
			} else {
				throw new Error(`unhandled var num button ${var_btn_num}`);
			}
		} else {
			if (var_btn_num == 1) {
				return { str: "z", type: TokenType.VAR };
			} else if (var_btn_num == 2) {
				return { str: "theta", type: TokenType.VAR };
			} else {
				throw new Error(`unhandled var num button ${var_btn_num}`);
			}
		}
	};
}

enum TokenType {
    DIGIT,
    VAR,
    FUNC_CALL,
    PAREN_OPEN,
    PAREN_CLOSE,
    OP,
    OTHER,
};

const TOKEN_LOG10    = { str : "log(",          type : TokenType.FUNC_CALL };
const TOKEN_10_POW   = { str : "10^(",          type : TokenType.FUNC_CALL };
const TOKEN_POW      = { str : "^",             type : TokenType.OP };
const TOKEN_LN       = { str : "ln(",           type : TokenType.FUNC_CALL };
const TOKEN_E_POW    = { str : "e^(",           type : TokenType.FUNC_CALL };
const TOKEN_PAR_L    = { str : "(",             type : TokenType.PAREN_OPEN };
const TOKEN_DELIM    = { str : ",",             type : TokenType.OTHER };
const TOKEN_PAR_R    = { str : ")",             type : TokenType.PAREN_CLOSE };
const TOKEN_NEG      = { str : "-",             type : TokenType.DIGIT };
const TOKEN_DIV      = { str : "/",             type : TokenType.OP };
const TOKEN_SQRT     = { str : "sqrt(",         type : TokenType.FUNC_CALL };
const TOKEN_POW_2    = { str : "^2",            type : TokenType.OTHER };
const TOKEN_E        = { str : "e",             type : TokenType.VAR };
const TOKEN_THETA    = { str : "theta",         type : TokenType.VAR };
const TOKEN_7        = { str : "7",             type : TokenType.DIGIT };
const TOKEN_8        = { str : "8",             type : TokenType.DIGIT };
const TOKEN_9        = { str : "9",             type : TokenType.DIGIT };
const TOKEN_MULT     = { str : "*",             type : TokenType.OP };
const TOKEN_VAR_X    = { str : "x",             type : TokenType.VAR };
const TOKEN_VAR_Y    = { str : "y",             type : TokenType.VAR };
const TOKEN_4        = { str : "4",             type : TokenType.DIGIT };
const TOKEN_5        = { str : "5",             type : TokenType.DIGIT };
const TOKEN_6        = { str : "6",             type : TokenType.DIGIT };
const TOKEN_SUB      = { str : "-",             type : TokenType.OP };
const TOKEN_ANS      = { str : "ans",           type : TokenType.VAR };
const TOKEN_PI       = { str : "pi",            type : TokenType.VAR };
const TOKEN_Z        = { str : "z",             type : TokenType.VAR };
const TOKEN_DEGREE   = { str : "o",             type : TokenType.OTHER };
const TOKEN_1        = { str : "1",             type : TokenType.DIGIT };
const TOKEN_2        = { str : "2",             type : TokenType.DIGIT };
const TOKEN_3        = { str : "3",             type : TokenType.DIGIT };
const TOKEN_0        = { str : "0",             type : TokenType.DIGIT };
const TOKEN_ADD      = { str : "+",             type : TokenType.OP };
const TOKEN_STO      = { str : "->",            type : TokenType.OP };
const TOKEN_IMG_UNIT = { str : "i",             type : TokenType.DIGIT };
const TOKEN_ANGLE    = { str : ANGLE_OP_SYMBOL, type : TokenType.OTHER };
const TOKEN_DECIMAL  = { str : ".",             type : TokenType.DIGIT };
const TOKEN_EXP      = { str : "E",             type : TokenType.DIGIT };
const TOKEN_SIN      = { str : "sin(",          type : TokenType.FUNC_CALL };
const TOKEN_COS      = { str : "cos(",          type : TokenType.FUNC_CALL };
const TOKEN_TAN      = { str : "tan(",          type : TokenType.FUNC_CALL };
const TOKEN_SINH     = { str : "sinh(",         type : TokenType.FUNC_CALL };
const TOKEN_COSH     = { str : "cosh(",         type : TokenType.FUNC_CALL };
const TOKEN_TANH     = { str : "tanh(",         type : TokenType.FUNC_CALL };
const TOKEN_ASIN     = { str : "asin(",         type : TokenType.FUNC_CALL };
const TOKEN_ACOS     = { str : "acos(",         type : TokenType.FUNC_CALL };
const TOKEN_ATAN     = { str : "atan(",         type : TokenType.FUNC_CALL };
const TOKEN_ASINH    = { str : "asinh(",        type : TokenType.FUNC_CALL };
const TOKEN_ACOSH    = { str : "acosh(",        type : TokenType.FUNC_CALL };
const TOKEN_ATANH    = { str : "atanh(",        type : TokenType.FUNC_CALL };



BTN_ID_TO_GET_TOKEN_FUNC.set("btn_log",    ret_token_factory_w_inv(TOKEN_LOG10, TOKEN_10_POW));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_pow",    ret_token_factory(TOKEN_POW));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_ln",     ret_token_factory_w_inv(TOKEN_LN, TOKEN_E_POW));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_par_l",  ret_token_factory(TOKEN_PAR_L));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_delim",  ret_token_factory(TOKEN_DELIM));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_unary_neg",  ret_token_factory(TOKEN_NEG));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_par_r",  ret_token_factory(TOKEN_PAR_R));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_div",    ret_token_factory(TOKEN_DIV));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_sqrt",   ret_token_factory_w_inv(TOKEN_SQRT, TOKEN_POW_2));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_e",      ret_token_factory_w_alt(TOKEN_E, TOKEN_THETA));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_7",      ret_token_factory(TOKEN_7));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_8",      ret_token_factory(TOKEN_8));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_9",      ret_token_factory(TOKEN_9));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_mult",   ret_token_factory(TOKEN_MULT));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_var1",   ret_token_factory_w_alt(TOKEN_VAR_X, TOKEN_VAR_Y));
//BTN_ID_TO_GET_TOKEN_FUNC.set("btn_var2",   var_token_factory(2));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_4",      ret_token_factory(TOKEN_4));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_5",      ret_token_factory(TOKEN_5));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_6",      ret_token_factory(TOKEN_6));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_sub",    ret_token_factory(TOKEN_SUB));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_ans",    ret_token_factory(TOKEN_ANS));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_pi",     ret_token_factory_w_alt(TOKEN_PI, TOKEN_Z));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_degree", ret_token_factory(TOKEN_DEGREE));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_1",      ret_token_factory(TOKEN_1));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_2",      ret_token_factory(TOKEN_2));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_3",      ret_token_factory(TOKEN_3));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_add",    ret_token_factory(TOKEN_ADD));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_sto",    ret_token_factory(TOKEN_STO));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_i",      ret_token_factory_w_alt(TOKEN_IMG_UNIT, TOKEN_ANGLE));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_0",      ret_token_factory(TOKEN_0));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_decimal",ret_token_factory(TOKEN_DECIMAL));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_exp",    ret_token_factory(TOKEN_EXP));

BTN_ID_TO_GET_TOKEN_FUNC.set("btn_sin",    get_trig_token("sin"));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_cos",    get_trig_token("cos"));
BTN_ID_TO_GET_TOKEN_FUNC.set("btn_tan",    get_trig_token("tan"));


function debug_log(type: string, msg: string) {
	if (type == "ui") { return; }
	console.debug(type + ": " + msg);
}



function handle_btn_toggle_inv(ui: CalcUi) {
	ui.state.inv_state = !ui.state.inv_state;
	debug_log("ui", "Toggling inv state to: " + ui.state.inv_state);
	update_btns(ui);
}

function handle_btn_toggle_alt(ui: CalcUi) {
	ui.state.alt_state = !ui.state.alt_state;
	debug_log("ui", "Toggling alt state to: " + ui.state.alt_state);
	update_btns(ui);
}

function update_btns(ui: CalcUi) {
	update_trig_btns(ui);
	update_arrow_btns(ui);
	update_other_btns(ui);
	update_var_btns(ui);
}

function update_arrow_btns(ui: CalcUi) {
	if (!ui.state.alt_state) {
		ui.btn_left.innerHTML = "&larr;";
		ui.btn_right.innerHTML = "&rarr;";
		ui.btn_clear.innerHTML = "clear";
	} else {
		ui.btn_left.innerHTML = "|&larr;";
		ui.btn_right.innerHTML = "&rarr;|";
		ui.btn_clear.innerHTML = "clear scrn";
	}
}

function update_trig_btns(ui: CalcUi) {
	let is_hyp_suffix;
	if (ENABLE_HYPERBOLIC_TRIG && ui.state.alt_state) {
		is_hyp_suffix = "h";
	} else {
		is_hyp_suffix = "";
	}
	if (!ui.state.inv_state) {
		ui.btn_sin.innerHTML = "sin" + is_hyp_suffix;
		ui.btn_cos.innerHTML = "cos" + is_hyp_suffix;
		ui.btn_tan.innerHTML = "tan" + is_hyp_suffix;
	} else {
		ui.btn_sin.innerHTML = "sin" + is_hyp_suffix + "<sup>-1</sup>";
		ui.btn_cos.innerHTML = "cos" + is_hyp_suffix + "<sup>-1</sup>";
		ui.btn_tan.innerHTML = "tan" + is_hyp_suffix + "<sup>-1</sup>";
	}
	
}

function update_other_btns(ui: CalcUi) {
	if (!ui.state.alt_state) {
		ui.btn_img_unit.innerHTML = "i";
		ui.btn_units.innerHTML = "units";
	} else {
		ui.btn_img_unit.innerHTML = "angle";
		ui.btn_units.innerHTML = "to units";
	}

	if (!ui.state.inv_state) {
		ui.btn_log.innerHTML  = "log<sub>10</sub>";
		ui.btn_ln.innerHTML   = "ln";
		ui.btn_sqrt.innerHTML = "sqrt(";
	} else {
		ui.btn_log.innerHTML  = "10<sup>x</sup>";
		ui.btn_ln.innerHTML   = "e<sup>x</sup>";
		ui.btn_sqrt.innerHTML = "x<sup>2</sup>";
	}

	if (!ui.state.polar_state) {
		ui.btn_polar_toggle.innerHTML = "polar";
	} else {
		ui.btn_polar_toggle.innerHTML = "rect";
	}

	if (!ui.state.degree_state) {
		ui.btn_degree_toggle.innerHTML = "degree";
	} else {
		ui.btn_degree_toggle.innerHTML = "radian";
	}
}

function update_var_btns(ui: CalcUi) {
	let btns_to_clear_children = [
		ui.btn_var1,
		ui.btn_pi,
		ui.btn_e,
		//ui.btn_var2,
	];
	for (let btn of btns_to_clear_children) {
		while (btn.firstChild) { btn.removeChild(btn.firstChild); }
	}

	ui.clear_latex_btn_stylesheets();

	if (!ui.state.alt_state) {
		// TODO handle these correctly now that generate_latex_nodes returns two things
		ui.btn_var1.appendChild(ui.generate_latex_node_btn("x",    "button", ui.btn_var1));
		ui.btn_pi.appendChild(  ui.generate_latex_node_btn("\\pi", "button", ui.btn_pi));
		ui.btn_e.appendChild(   ui.generate_latex_node_btn("e",    "button", ui.btn_e));
		//ui.btn_var2.appendChild(ui.generate_latex_node_btn("y", "button", ui.btn_var2));
	} else {
		ui.btn_var1.appendChild(ui.generate_latex_node_btn("y",       "button", ui.btn_var1));
		ui.btn_pi.appendChild(  ui.generate_latex_node_btn("z",       "button", ui.btn_pi));
		ui.btn_e.appendChild(   ui.generate_latex_node_btn("\\theta", "button", ui.btn_e));
		//ui.btn_var2.appendChild(ui.generate_latex_node_btn("\\theta", "button", ui.btn_var2));
	}
}

function update_input_textarea(ui: CalcUi) {
	let text = tokens_to_text(ui.state.input_tokens);
	ui.input_text.value = text;
	// TODO add an option to configure this
	if (!ui.is_mobile()) { // TODO maybe only if not iOS
		let pos = get_cursor_pos(ui);
		ui.input_text.selectionStart = pos;
		ui.input_text.selectionEnd   = pos;
	}
	update_latex_display(ui);
}


function on_tex_ready_callback(raw_input: string, tex: string) {
	ui.update_input_wip_display(tex);
	ui.output_disp_scroll_to_bottom();
}

function get_cursor_pos(ui: CalcUi) {
	let pos = 0;
	for (let i=0; i<ui.state.input_tokens.length; i++) {
		if (i >= ui.state.cursor_idx) {
			break;
		}
		// TODO this is failing. Maybe revert to old code first (but using typescript), make sure it all builds, then add the "ans" stuff?
		pos += ui.state.input_tokens[i].str.length;
	}
	return pos;
}

function cursor_pos_to_idx(ui: CalcUi, cursor_pos: number|null) {
	if (cursor_pos == null) {
		console.warn("cursor_pos is null");
		cursor_pos = 0;
	}
	let i;
	for (i=0; i<ui.state.input_tokens.length; i++) {
		if (cursor_pos <= 0) { return i; }
		cursor_pos -= ui.state.input_tokens[i].str.length;
	}
	return i;
}

let _is_set_on_tex_ready = false;
function update_latex_display(ui: CalcUi) {
	let raw_input = ui.input_text.value;
	console.debug("Updating latex display with raw_input \"" + raw_input + "\"");

	if (!_is_set_on_tex_ready) {
		set_on_tex_ready(on_tex_ready_callback);
		_is_set_on_tex_ready = true;
	}

	let tex;
	// TODO not sure if this case should go inside the calc
	if (raw_input == "") {
		on_tex_ready_callback("", "");
	} else {
		ui.to_latex_async(raw_input, get_cursor_pos(ui));
	}

}

function handle_normal_btn(ui: CalcUi, e: MouseEvent) {
	const e_target = e.target as HTMLElement;
	// NOTE: buttons with child nodes (e.g. superscripts) _would_ have e.target
	// point to those instead of the button itself, if the user happens
	// to click right on the child node...
	// But there's a CSS way to stop that:
	// ```css
	//     button > * {
	//         pointer-events: none;
	//     }
	// ```
	const id = e_target.id;
	console.debug("handle_normal_btn", e);
	debug_log("ui", "btn pressed " + id);
	if (!BTN_ID_TO_GET_TOKEN_FUNC.has(id)) {
		console.error("btn id \"" + id + "\" not handled");
		return;
	}
	let token_func = BTN_ID_TO_GET_TOKEN_FUNC.get(id);
	if (token_func === undefined) {
		throw new Error(`token func undefined for ${id}`);
	}
	let token = token_func(ui);
	btn_token_pressed(ui, token);
}

function btn_token_pressed(ui: CalcUi, token: Token, needs_mult?: boolean) {
	insert_new_input_token(ui, token, needs_mult);
	ui.show_input_wip_display = true;
	update_input_textarea(ui);
	ui.state.alt_state = false;
	ui.state.inv_state = false;
	update_btns(ui);
}

function tokens_to_text(tokens: InputTokenT[]): string {
	let outputText = "";
	let first = true;
	for (let token of tokens) {
		if (!first) {
			outputText += " ";
			first = false;
		}
		outputText += token.str;
	}
	return outputText;
}

function clear_input(ui: CalcUi) {
	ui.state.input_tokens = [];
	ui.state.cursor_idx = 0
	update_input_textarea(ui);
}

function handle_btn_clear(ui: CalcUi) {
	if (!ui.state.alt_state) {
		clear_input(ui);
	} else {
		ui.state.output_display_srcs = [];
		ui.clear_output_display();
		ui.state.alt_state = false;
		ui.state.inv_state = false;
		update_btns(ui);
	}
}

function handle_btn_bksp(ui: CalcUi) {
	let idx = ui.state.cursor_idx - 1;
	if (idx < 0) { return; }
	console.log("Removing element at position ", idx);
	ui.state.input_tokens.splice(idx, 1);
	ui.state.cursor_idx--;
	cap_cursor_idx(ui);
	update_input_textarea(ui);
}


// TODO also call this when the user presses enter while typing in the text box
function handle_user_enter(ui: CalcUi) {
	history_entry_add(ui, ui.state.input_tokens.slice());
	let raw_input = ui.input_text.value;
	//set_on_calc_output_ready(function (raw_input, latex, calc_output) {
	let callback = function (raw_input: string, latex: string, calc_output: CalcOutput, calcdata: CalcData) {
		calc_output_ready(ui, raw_input, latex, calc_output, calcdata);
	};
	ui.calc_async(raw_input, callback);
}

function calc_output_ready(ui: CalcUi, raw_input: string, latex: string, calc_output: CalcOutput, calcdata: CalcData) {
	console.debug("output is: ", calc_output);
	let show_raw = ui.state.show_raw_calc_io;
	console.debug(raw_input, calc_output);
	ui.generate_output_display(raw_input, latex, calc_output, show_raw);
	ui.state.output_display_srcs.push({ raw_input: raw_input, latex: latex, calc_output: calc_output });
	ui.output_disp_scroll_to_bottom();
	ui.update_vars(calcdata.vars);
	// TODO if there's a syntax error, don't clear input
	ui.show_input_wip_display = false;
	ui.remove_input_wip_display_stylesheets();
	clear_input(ui);
}

function toggle_show_raw(ui: CalcUi) {
	ui.state.show_raw_calc_io = ui.checkbox_show_raw.checked;
	//ui.state.show_raw_calc_io; = !ui.state.show_raw_calc_io;
	refresh_output_display(ui);
	ui.output_disp_scroll_to_bottom();
}

function write_all_output_display(ui: CalcUi) {
	for (let srcs of ui.state.output_display_srcs) {
		ui.generate_output_display(srcs.raw_input, srcs.latex, srcs.calc_output, ui.state.show_raw_calc_io);
	}
}

function refresh_output_display(ui: CalcUi) {
	console.debug("Refreshing output display");
	ui.clear_output_display();
	write_all_output_display(ui);
}

function handle_btn_pos_factory(ui: CalcUi, dir_str: "left"|"right") {
	return function (e: Event) {
		let len = ui.state.input_tokens.length;
		if (dir_str == "left") {
			if (!ui.state.alt_state) {
				ui.state.cursor_idx--;
			} else {
				ui.state.cursor_idx = 0
			}
		} else if (dir_str == "right") {
			if (!ui.state.alt_state) {
				ui.state.cursor_idx++;
			} else {
				ui.state.cursor_idx = len;
			}
		}
		cap_cursor_idx(ui);
		update_input_textarea(ui);
		ui.state.alt_state = false;
		ui.state.inv_state = false;
		update_btns(ui);
	}
}


function cap_cursor_idx(ui: CalcUi) {
	let len = ui.state.input_tokens.length;
	if (ui.state.cursor_idx <  0  ) { ui.state.cursor_idx = 0; }
	if (ui.state.cursor_idx >= len) { ui.state.cursor_idx = len; }
}

/*
function string_to_char_array(str: string) {
	let ary = new Array(str.length);
	for (let i=0; i<str.length; i++) {
		ary[i] = str[i];
	}
	return ary;
}
*/

function string_to_input_tokens(str: string): InputTokenT[] {
	let input_tokens = [];
	for (let c of str) {
		input_tokens.push(InputToken(c));
	}
	return input_tokens;
}


function input_text_keypress(ui: CalcUi, e: Event) {
	//console.log("keypress", e);
	ui.show_input_wip_display = true;
	ui.state.input_tokens = string_to_input_tokens(ui.input_text.value);
	ui.state.cursor_idx = cursor_pos_to_idx(ui, ui.input_text.selectionStart);
	update_latex_display(ui);
}

function input_text_keydown(ui: CalcUi, e: KeyboardEvent) {
	//console.log("keydown", e);
	if (e.key == "ArrowUp") {
		e.preventDefault();
		handle_btn_history(ui, -1);
	} else if(e.key == "ArrowDown") {
		e.preventDefault();
		handle_btn_history(ui, 1);
	} else if(e.key == "Enter") {
		e.preventDefault();
		handle_user_enter(ui);
	}
}

function cap(min_val: number, max_val: number, val: number): number {
	if (val <= min_val) {
		return min_val;
	} else if (val >= max_val) {
		return max_val;
	} else {
		return val;
	}
}

function input_tokens_eq(a: InputTokenT, b: InputTokenT): boolean {
	return a.str == b.str && a.is_unit == b.is_unit;
}

function input_token_arrays_eq(a: InputTokenT[], b: InputTokenT[]): boolean {
	if (!a || !b) { return false; }
	if (a.length != b.length) { return false; }
	for (let i=0; i<a.length; i++) {
		if (!input_tokens_eq(a[i], b[i])) { return false; }
	}
	return true;
}

function last_elem<T>(ary: T[]): T {
	return ary[ary.length-1];
}

function history_entry_add(ui: CalcUi, input_tokens: InputTokenT[]) {
	// don't add this input to the history if it's the same as the last element
	if (!input_token_arrays_eq(last_elem(ui.state.input_history), input_tokens)) {
		ui.state.input_history.push(input_tokens);
	}
	ui.state.input_history_pos = ui.state.input_history.length;
	console.debug("history update {pos:", ui.state.input_history_pos,
	            ", history: ", ui.state.input_history,
	            "}");
}

// TODO if the user presses up or down to go to a previous entry in the history,
// makes some changes, then presses up or down again, they shouldn't lose their input.
// It seems that the way bash handles it is by actually letting you edit the history
// For now I'll just throw it out, but maybe have both in the history
function handle_btn_history(ui: CalcUi, change: number) {
	ui.state.input_history_pos += change;
	ui.state.input_history_pos = cap(0, ui.state.input_history.length, ui.state.input_history_pos);
	const idx = ui.state.input_history_pos;
	let new_input: InputTokenT[];
	if (idx == ui.state.input_history.length) {
		new_input = [];
	}  else {
		new_input = ui.state.input_history[idx].slice();
	}
	console.debug("handle_btn_history: pos is now ", ui.state.input_history_pos,
	            ", input is now: ", new_input);
	ui.state.input_tokens = new_input;
	ui.show_input_wip_display = true;
	ui.state.cursor_idx = ui.state.input_tokens.length;
	update_input_textarea(ui);
}

// TODO call this with false to hide display if the user
// clicks a "cancel" button on the display itself, an "insert" button indicating "put the variable in the input"
// or if they click outside the pop up
function set_var_display_visible(ui: CalcUi, is_visible: boolean) {
	ui.state.show_var_display = is_visible;
	if (ui.state.show_var_display) {
		ui.var_selector_display.style.display="";
	} else {
		ui.var_selector_display.style.display="none";
	}
}

function set_about_popup_visible(ui: CalcUi, is_visible: boolean) {
	ui.state.show_about_popup = is_visible;
	if (ui.state.show_about_popup) {
		ui.about_popup.style.display = "";
	} else {
		ui.about_popup.style.display = "none";
	}
}

function toggle_var_display(ui: CalcUi, e: Event) {
	set_var_display_visible(ui, !ui.state.show_var_display);
}

function calc_ui_set_unit_display_visible(ui: CalcUi, is_visible: boolean) {
	ui.state.show_unit_display = is_visible;
	calc_ui_unit_sel_set_visible(ui.unit_sel, is_visible);
}

function toggle_unit_display(ui: CalcUi, e: Event) {
	calc_ui_set_unit_display_visible(ui, !ui.state.show_unit_display);
}


function handle_btn_pressed(ui: CalcUi) {
	// prevent text area from losing focus.
	// this doesn't do anything if you click on something besides a button
	// should maybe just call this every second or so? Or whenever the user clicks anywhere in the window?
	// the one exception is if I add another text input in the variable picker 

	// on mobile, don't focus on text input, it brings up the on screen keyboard and is really annoying.
	// mobile should use mostly buttons anyway, though if they want to use the text input then they can
	// click on the text input themselves when ready
	// TODO make an option to configure this
	if (!ui.is_mobile()) {
		ui.set_focus_to_textarea();
	}
}

function handle_new_var_btn_pressed(ui: CalcUi, btn: HTMLElement) {
	console.debug("New var ", btn, " button pressed");
	ui.var_btn_selected(btn);
	ui.custom_var_name_input.value = "";
	update_insert_var_btn_enabled_state(ui);
}

function insert_new_input_token(ui: CalcUi, token: Token, needs_mult?: boolean, token_is_unit?: boolean) {
	let token_str = token.str;
	console.debug("insert_new_input_token ", token, "needs_mult = ", needs_mult, "token_is_unit =", token_is_unit);
	let prev_token = null;
	if (ui.state.input_tokens.length > 0) { 
		let i = ui.state.cursor_idx - 1;
		if (i >= 0) {
			prev_token = ui.state.input_tokens[i];
		};
	}
	if (prev_token != null && needs_mult == undefined) {
		// TODO this is bad if you move the cursor and delete the thing before one of these.
		// should probably make them separate tokens. At least add a setting for this
		needs_mult = check_if_token_needs_mult_between(prev_token.str, token_str);
	}
	if (needs_mult) {
		token_str = "*" + token_str;
	}

	if (token_is_unit == undefined) {
		token_is_unit = false;
	}

	// If the user enters a binary operator as the first token,
	// then insert an "ans" before it, since they likely want to operate on the result
	// of the previous expression.
	// https://github.com/alexbarry/AlexCalc/issues/24
	if (ui.state.input_tokens.length == 0 && token.type == TokenType.OP) {
		ui.state.input_tokens.push(ans_token());
		ui.state.cursor_idx++;
	}

	let token_obj = InputToken(token_str, token_is_unit)

	ui.state.input_tokens.splice(ui.state.cursor_idx, 0, token_obj);
	ui.state.cursor_idx++;
}

function handle_insert_var_btn_pressed(ui: CalcUi) {
	let var_name = null;
	if (ui.state.var_btn_selected != null) {
		var_name = ui.state.new_var_btns_map.get(ui.state.var_btn_selected);
	} else if (ui.custom_var_name_input.value.length > 0) {
		var_name = ui.custom_var_name_input.value;
	}

	if (var_name != null) {
		let token = { str: var_name, type: TokenType.VAR };
		insert_new_input_token(ui, token);
		ui.show_input_wip_display = true;
		update_input_textarea(ui);
	}
	// not sure if this is actually necessary... if the user opens up this popup again,
	// what's the harm in keeping the same thing selected?
	ui.var_btn_selected(null);
	update_insert_var_btn_enabled_state(ui);
	set_var_display_visible(ui, false);
}

function handle_custom_var_name_input(ui: CalcUi) {
	ui.var_btn_selected(null);
	update_insert_var_btn_enabled_state(ui);
}

function update_insert_var_btn_enabled_state(ui: CalcUi) {
	let btn_enabled = (ui.state.var_btn_selected != null ||
	                   ui.custom_var_name_input.value.length > 0);

	ui.btn_var_display_insert.disabled = !btn_enabled;
}

// TODO not sure what to do about "^2" as a token
function token_is_bin_op(token: string) {
	const bin_ops = [
		"+",
		"-",
		"*",
		"/",
		"^",
		"->",
		ANGLE_OP_SYMBOL,
	];
	for (let bin_op of bin_ops) {
		if (bin_op == token) { return true; }
	}
	return false;
}

function ans_token() {
	return InputToken("ans", false);
}

function token_is_num(token: null|string) {
	if (token == null || token.length == 0) { return false; }
	return /^(\*)?[0-9E]$/.test(token);
}

function token_is_var(token: null|string) {
	if (token == null || token.length == 0) { return false; }
	// TODO can't distinguish between EXP (as in "1E3") and a variable named
	// "E" with current system.
	// Might need to have some tokens stored as special strings or objects,
	// replacing them with their string representation at the end
	if (token == "E") { return false; }
	return /^(\*)?[a-zA-Z][0-9a-zA-Z_]*$/.test(token);
}

function token_is_func_call(token: string) {
	return /^[a-zA-Z][0-9a-zA-Z_]*\($/.test(token);
}


// if the user presses buttons (e.g. "2" and "pi"), sometimes they want
// to multiply them.
//
// It seems that you want to multiply if:
// * variable or brackets next to variable, brackets, or numeric.
// * numeric next to numeric isn't a case, because pressing "2" "2" means 22, not 2*2
// * can make an exception for "i" and "j", next to numeric literals, at least as long
//   as they are both considered
//   imaginary units.
//
// Maybe it's easier to decide when you don't want to multiply:
// * if pressing a button following a binary operator (is negative is going to cause headaches?)
// * if pressing a binary operator
// * if pressing a closing bracket
// * if pressing a numeric literal after another numeric literal, including imaginary units
// * if pressing anything after a bracket, open function call
function check_if_token_needs_mult_between(prev_token: string, new_token: string) {
	console.debug("calling check_if_token_needs_mult_between with ", prev_token, new_token);
	if (token_is_bin_op(new_token) || token_is_bin_op(prev_token) ||
	    new_token == "^2") {
		console.debug("new or prev tokens are bin op, no mult needed");
		return false;
	}

	if (prev_token == "(" ||
	    token_is_func_call(prev_token) ||
	    new_token == ")") {
		console.debug("prev token is open brack or func call, or new token is close paren. No mult symbol");
		return false;
	}

	if (token_is_num(prev_token) && token_is_num(new_token)) {
		console.debug("both tokens are digits, adding mult symbol");
		return false;
	}

	// if prev token is num/var/close brack, and new token is num/func/var/open brack,
	// but note that prev=num and new=num is false, handled earlier.
	if ( (token_is_num(prev_token) ||
	      token_is_var(prev_token) ||
	      prev_token == ")") &&
	     (token_is_num(new_token) ||
	      token_is_func_call(new_token) ||
	      token_is_var(new_token) ||
	      new_token == "(")) {
		console.debug("prev token is digit/var/close brack and new token is digit/func/var/open brack, adding mult symbol");
		return true;
	}

	console.debug("no match, not adding mult symbol");
	return false;
}

function handle_polar_toggle(ui: CalcUi) {
	ui.state.polar_state = !ui.state.polar_state;
	debug_log("ui", "Toggling polar state to: " + ui.state.polar_state);
	ui.update_calcstate(get_calcstate(ui.state));
	update_btns(ui);
	update_latex_display(ui);
	// TODO maybe update the past couple of outputs too?
}

function handle_degree_toggle(ui: CalcUi) {
	ui.state.degree_state = !ui.state.degree_state;
	debug_log("ui", "Toggling degree state to: " + ui.state.degree_state);
	ui.update_calcstate(get_calcstate(ui.state));
	update_btns(ui);
	update_latex_display(ui);
}

const SUPPORTED_THEMES = [
	"light",
	"dark",
	"very_dark",
];

function ary_includes(ary: string[], val: string): boolean {
	for (const ary_val of ary) {
		if (ary_val == val) {
			return true;
		}
	}
	return false;
}

function set_theme(ui: CalcUi, theme: string) {
	console.debug("Setting theme to ", theme);
	if (!ary_includes(SUPPORTED_THEMES,theme)) {
		console.error("unsupported theme", theme);
		return;
	}
	document.body.classList.add(theme);
	for (let old_theme of SUPPORTED_THEMES) {
		if (old_theme != theme) {
			document.body.classList.remove(old_theme);
		}
	}

	let data_color_scheme_val = "light";
	if (theme == "light") {
		data_color_scheme_val = "light";
	} else if (theme == "dark" || theme == "very_dark") {
		data_color_scheme_val = "dark";
	} else {
		console.warn("Unhandled color scheme", theme);
	}

	const meta_color_scheme_elem = document.querySelector("meta[name=color-scheme]");

	if (meta_color_scheme_elem) {
		// Without this, it seems like the scrollbars, checkbox, and dropdown (select)
		// can retain the user's global colour scheme preference,
		// even if they specify that they want this page to be different.
		meta_color_scheme_elem.setAttribute('content', data_color_scheme_val);
	} else {
		console.warn("html meta color-scheme not present");
	}
}

function set_dark_mode_select(ui: CalcUi, theme: string) {
	for (let i=0; i<ui.dark_mode_select.options.length; i++) {
		let option = ui.dark_mode_select.options[i];
		if (option.value == theme) {
			ui.dark_mode_select.selectedIndex = i;
			return;
		}
	}
	console.error("theme ", theme, "not found in select");
	
}

// See https://developer.chrome.com/blog/auto-dark-theme/#detecting-auto-dark-theme
function check_forced_dark_mode() {
	let elem = document.querySelector('#auto_dark_mode_detection');
	if (elem === null)  {
		console.error("can not found #auto_dark_mode_detection");
		return;
	}
	return getComputedStyle(elem).backgroundColor != 'rgb(255, 255, 255)';
}

function init_ui_throws(ui: CalcUi) {
	console.debug("Initializing Calc UI");
	ui.state = init_ui_state();
	
	const ui_btn_handlers = [
		{ btn: ui.btn_inv,     handler: function (e: Event) { handle_btn_toggle_inv(ui); } },
		{ btn: ui.btn_alt,     handler: function (e: Event) { handle_btn_toggle_alt(ui); } },
		{ btn: ui.btn_clear,   handler: function (e: Event) { handle_btn_clear(ui); } },
		{ btn: ui.btn_bksp,    handler: function (e: Event) { handle_btn_bksp(ui); } },
		{ btn: ui.btn_left,    handler: handle_btn_pos_factory(ui, 'left')  },
		{ btn: ui.btn_right,   handler: handle_btn_pos_factory(ui, 'right') },
		{ btn: ui.btn_up,      handler: function (e: Event) { handle_btn_history(ui, -1); } },
		{ btn: ui.btn_down,    handler: function (e: Event) { handle_btn_history(ui, 1); } },
		{ btn: ui.btn_enter,   handler: function (e: Event) { handle_user_enter(ui); } },
		{ btn: ui.btn_polar_toggle,  handler: function (e: Event) { handle_polar_toggle(ui); } },
		{ btn: ui.btn_degree_toggle, handler: function (e: Event) { handle_degree_toggle(ui); } },
	];

	ui.checkbox_show_raw.checked = ui.state.show_raw_calc_io
	ui.checkbox_show_raw.addEventListener('click', function (e: Event) { toggle_show_raw(ui) });

	let darkMatch;
	if (window.matchMedia) {
		darkMatch = window.matchMedia("(prefers-color-scheme: dark)");
	}

	if (darkMatch && darkMatch.matches) {
		ui.selected_theme = "dark";
	} else if (check_forced_dark_mode()) {
		console.log("User has #force-dark-mode enabled, but not ('prefers-color-scheme: dark)!!!");
		ui.selected_theme = "dark";
	}

	
	console.debug("OS default for dark mode is: ", ui.selected_theme);
	set_dark_mode_select(ui, ui.selected_theme);
	set_theme(ui, ui.selected_theme);
	ui.dark_mode_select.addEventListener('change', function (e: Event) {
		const e_target = e.target as HTMLInputElement;
		set_theme(ui, e_target.value);
	});

	for (let info of ui_btn_handlers) {
		info.btn.addEventListener('click', info.handler);
		info.btn.addEventListener('click', function (e: Event) { handle_btn_pressed(ui); } );
	}

	for (let btn of ui.normal_btns) {
		btn.addEventListener('click', function (e: MouseEvent) { handle_normal_btn(ui, e); });
		btn.addEventListener('click', function (e: Event) { handle_btn_pressed(ui); } );
	}

	ui.btn_vars.addEventListener('click', function (e: Event) {
		toggle_var_display(ui, e);
		handle_btn_pressed(ui);
	});
	ui.btn_var_display_cancel.addEventListener('click', function(e: Event) {
		set_var_display_visible(ui, false);
		handle_btn_pressed(ui);
	});

	ui.btn_units.addEventListener('click', function (e: Event) {
		if (!ui.state.alt_state) {
			toggle_unit_display(ui, e);
		} else {
			const TO_UNITS_TOKEN = { str: " to ", type: TokenType.OTHER };
			btn_token_pressed(ui, TO_UNITS_TOKEN, /* needs mult */ false);
		}
		handle_btn_pressed(ui);
	});
	ui.btn_unit_display_cancel.addEventListener('click', function(e: Event) {
		calc_ui_set_unit_display_visible(ui, false);
		handle_btn_pressed(ui);
	});

	ui.input_text.addEventListener('input',   function (e: Event) { input_text_keypress(ui, e); } );
	ui.input_text.addEventListener('click',   function (e: Event) { input_text_keypress(ui, e); } );
	ui.input_text.addEventListener('keydown', function (e: KeyboardEvent) { input_text_keydown(ui, e); } );

	for (let info of ui.new_var_btns) {
		let btn = document.getElementById(info.id);
		if (btn == null) {
			console.error(`Could not find btn id ${info.id}`);
			continue;
		}
		ui.state.new_var_btns_map.set(btn, info.var_name);
		btn.addEventListener('click', function (e: Event) { handle_new_var_btn_pressed(ui, btn); });
	}
	ui.btn_var_display_insert.addEventListener('click', function (e: Event) { handle_insert_var_btn_pressed(ui); });
	ui.custom_var_name_input.addEventListener('input', function (e: Event) { handle_custom_var_name_input(ui); });

	init_unit_sel(ui.unit_sel);

	for (let btn of ui.show_about_popup_btns) {
		btn.addEventListener('click', function (e: Event) { set_about_popup_visible(ui, true); });
	}
	for (let btn of ui.close_about_popup_btns) {
		btn.addEventListener('click', function (e: Event) { set_about_popup_visible(ui, false); });
	}
}

/**
 * If the last input token is a unit, get all consecutive units.
 */
function get_previous_input_token_units(ui: CalcUi) {
	let units = [];
	for (let i=ui.state.input_tokens.length-1; i>=0; i--) {
		if (ui.state.input_tokens[i].is_unit) {
			units.push(ui.state.input_tokens[i].str);
		} else {
			break;
		}
	}
	return units;
}

function combine_unit_pieces(unit_strs: string[]): string {
	let str = "";
	let first = true;
	for (let unit_str of unit_strs) {
		if (!first) {
			str += " ";
		}
		first = false;
		str += unit_str;
	}
	return str;
}

// public API
// should only be called by unit display
export function public_add_input_token(ui: CalcUi, token: Token, prefix_mult: boolean, is_unit: boolean) {
	insert_new_input_token(ui, token, prefix_mult, is_unit);
	ui.show_input_wip_display = true;
	update_input_textarea(ui);
}

// TODO just add typescript to catch all of these type issues
// TODO confirm behaviour when pressing "insert unit"... should the popup go away? Did it before?

// public API
export function public_add_unit_input_token(ui: CalcUi, unit_token_str: string) {
	let prev_units = get_previous_input_token_units(ui);

	const token = {
		str: unit_token_str,
		type: TokenType.OTHER,
	};
	public_add_input_token(ui, token, false, true);

	// if [s^-2] is pressed after [m], then add only [m s^-2] instead
	// of both [s^-2] and [m].
	let referenced_unit = unit_token_str;
	if (prev_units.length > 0) {
		prev_units.push(token.str);
		referenced_unit = combine_unit_pieces(prev_units);
		//update_recently_used_units(ui.unit_sel, [combined_unit_str]);
	}
	referenced_unit = referenced_unit.trim();
	alexcalc_unit_referenced(referenced_unit);
}

// public API
export function add_variable_row_click_listener(ui: CalcUi, row: HTMLElement, var_name: string) {
	row.addEventListener('click', function (e: Event) { handle_new_var_btn_pressed(ui, row) });
	ui.state.new_var_btns_map.set(row, var_name);
}

// public API
export function remove_variable_row_click_listener(ui: CalcUi, row: HTMLElement) {
	ui.state.new_var_btns_map.delete(row);
}

// public API
export function init_ui(ui: CalcUi) {
	try {
		init_ui_throws(ui);
	} catch (e) {
		ui.generate_output_msg("Error initializing UI: " + e, true);
		throw e;
	}
}
