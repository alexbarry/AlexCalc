
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <cmath>

#include "node_to_latex.h"

// I find the regular math "-" to be way too wide for negativity,
// both as like "4 + -3" or "1 m s^-1"
#define NEG_SYMB_LATEX ("\\text{-}")


// Dummy variable to help me notice two hacky parts of the 
// code that are related.
int sci_not_pow_brackets_hack = 1;

bool debug_latex = false;

//static const std::string cursor_latex = "\\vert";
static const std::string cursor_latex = "\\text{[]}";
static const std::string latex_degree = "^\\circ";

std::string raw_node_to_latex(Node *n, const InputInfo *info);
std::string insert_cursor(std::string str, int cursor_pos);
std::string input_units_to_latex(const CalcData *calcData,
                                 const std::vector<UnitInfoInput> *input_units);

std::string input_units_to_latex(const CalcData *calcData,
                                 const std::vector<UnitInfoInput> *input_units,
                                 int cursor_pos,
                                 const WipValVarTokenCursorPos *cursor_info);

std::unordered_map<std::string, bool> get_supported_latex_symbols(void) {
	static std::unordered_map<std::string, bool> m = {
		{"alpha",   true},
		{"Nu",      true},
		{"nu",      true},
		{"Beta",    true},
		{"beta",    true},
		{"Xi",      true},
		{"xi",      true},
		{"Gamma",   true},
		{"gamma",   true},
		{"Omicron", true},
		{"omicron", true},
		{"Delta",   true},
		{"delta",   true},
		{"Pi",      true},
		{"pi",      true},
		{"varpi",   true},
		{"epsilon", true},
		{"varepsilon", true},
		{"Rho",     true},
		{"rho",     true},
		{"varrho",  true},
		{"Zeta",    true},
		{"zeta",    true},
		{"Sigma",   true},
		{"sigma",   true},
		{"varsigma",true},
		{"Eta",     true},
		{"eta",     true},
		{"Tau",     true},
		{"tau",     true},
		{"theta",   true},
		{"vartheta",true},
		{"Upsilon", true},
		{"upsilon", true},
		{"Iota",    true},
		{"iota",    true},
		{"Phi",     true},
		{"phi",     true},
		{"varphi",  true},
		{"kappa",   true},
		{"varkappa",true},
		{"Chi",     true},
		{"chi",     true},
		{"Lambda",  true},
		{"lambda",  true},
		{"Psi",     true},
		{"psi",     true},
		{"Mu",      true},
		{"mu",      true},
		{"Omega",   true},
		{"omega",   true},

		{"square",   true},
	};
	return m;
}

// Keys in this map should be converted to the value in the map,
// with the argument surrounded by curly braces, e.g.:
// calc_input:
//     sqrt(x)
// to latex:
//     \sqrt{x}
static const std::unordered_map<std::string, std::string> latex_funcs = {
	{"sqrt",   "\\sqrt"},
};

// Keys in this map should be converted to the value in the map,
// with the argument surrounded by normal brackets (optionally with \left( \right) )
// calc_input:
//     sin(x)
// to latex:
//     \sin(x)
static const std::unordered_map<std::string, std::string> latex_func_symbols = {
	{"log",    "\\log_{10}"},
	{"sin",    "\\sin"},
	{"cos",    "\\cos"},
	{"tan",    "\\tan"},
	{"asin",   "\\sin^{-1}"},
	{"acos",   "\\cos^{-1}"},
	{"atan",   "\\tan^{-1}"},
	{"sinh",   "\\sinh"},
	{"cosh",   "\\cosh"},
	{"tanh",   "\\tanh"},
	{"asinh",  "\\sinh^{-1}"},
	{"acosh",  "\\cosh^{-1}"},
	{"atanh",  "\\tanh^{-1}"},
};

bool is_supported_latex_symbol(std::string name) {
	std::unordered_map<std::string, bool> m = get_supported_latex_symbols();
	if (m.find(name) != m.end()) {
		return true;
	} else {
		return false;
	}
}

enum bracks_check_case {
	BRACKS_CHECK_UNSET,
	POW_BASE,
};

std::string surround_in_bracks_if_needed(NodeOp *parent, Node *n,
                                         const struct calc_fmt_params &params,
                                         const InputInfo *info,
                                         enum bracks_check_case bracks_case = BRACKS_CHECK_UNSET) {
	bool bracks_needed = false;
	if (n->get_node_type() == NODE_OP) {
		NodeOp *node_op = n->promote_to_op();
		if (node_op->get_precedence() < parent->get_precedence()) {
			bracks_needed = true;
		} else if (bracks_needed_if_eq_precedence(parent->get_op()) &&
		           parent->get_precedence() == node_op->get_precedence()) {
			bracks_needed = true;
		} else if (bracks_case == POW_BASE && node_op->get_op() == OP_POW) {
			bracks_needed = true;
		} else if (node_op->get_op() == OP_ANGLE && parent->get_op() == OP_POW) {
			bracks_needed = true;
		}
	}


	// TODO need to also add brackets even
	// if the node is a raw value but represented as "1.23 \cdot 10^12"
	// maybe the best way of doing this is to call node_to_latex first, and 
	// check if it contains certain characters, or whitespace?
	// effectively change this from blacklist of operator nodes with lower precedence, to
	// a whitelist of only skipping brackets on:
	//     * raw values with no scientific notation, and
	//     * shit... what do we do for 4*5 + 3?
	// maybe the scientific notation value raised to an exponent is a special case?
	NodeValue* n_value = dynamic_cast<NodeValue*>(n);
	NodeValueRect* n_value_rect = dynamic_cast<NodeValueRect*>(n);
	if (parent->get_node_type() == NODE_OP &&
	    parent->promote_to_op()->get_op() == OP_POW &&
	    n_value != nullptr) {
		val_t val = n_value->get_val(info->calcData, info->degree());
		// TODO if displayed as rect, also need for multiplication operators to
		// force brackets
		if (val.re != 0 && val.im != 0) {
			bracks_needed = true;
		} else if (units_non_zero(val.unit_dim)) {
			bracks_needed = true;
		} else if (n_value_rect != nullptr &&
		           get_pow_exp_str(n_value_rect->val_str) != 0) {
			// If this is a value represented in scientific notation, currently
			// those always have brackets. So omit them in this case.
			(void)sci_not_pow_brackets_hack;
			bracks_needed = false;
		} else {
			// I think this is pretty outdated now, since I've decided
			// to preserve the formatting of all inputted raw values.
			// And it isn't possible to get any other type of value
			// in an input expression.
			// I suppose this would be needed if I ever generated
			// equations
			calc_float_t mag = val.re != 0 ? val.re : val.im;
			calc_flt_fmt_t val_fmt = format_calc_float_params(mag, params);
			if (val_fmt.exponent != 0) { 
				bracks_needed = true;
			}
		}
	}

	if (bracks_needed) {
		return "\\left(" + raw_node_to_latex(n, info) + "\\right)";
	} else {
		return raw_node_to_latex(n, info);
	}

}

std::string surround_str_in_braces_if_len_gt1(std::string str) {
	if (str.size() <= 1) {
		return str;
	} else {
		return std::string("{") + str + "}";
	}
}

std::string surround_in_braces_if_len_gt1(Node *n, const InputInfo *info) {
	std::string latex = raw_node_to_latex(n, info);
	if (latex.size() > 1) {
		return "{" + latex + "}";
	} else {
		return latex;
	}
}


std::string op_node_to_latex(NodeOp *n, const struct calc_fmt_params &params, const InputInfo *info) {
	std::string op_prefix = "";
	std::string op_suffix = "";
	if (n == info->cursor_node) {

		// TODO not sure when I should use prefix or suffix
		if (info->cursor_offset >= 0) {
			op_prefix = cursor_latex;
		//} else if (info->cursor_offset == -1) {
		//	op_suffix = cursor_latex;
		} else {
			op_suffix = cursor_latex;
		}
	}
	switch (n->get_op()) {
		case OP_ADD:  return raw_node_to_latex(n->children.at(0), info) + op_prefix + std::string(" + ") + op_suffix + surround_in_bracks_if_needed(n, n->children.at(1), params, info);
		case OP_SUB:  return raw_node_to_latex(n->children.at(0), info) + op_prefix + std::string(" - ") +  op_suffix + surround_in_bracks_if_needed(n, n->children.at(1), params, info);
		case OP_MULT: return surround_in_bracks_if_needed(n, n->children.at(0), params, info) + op_prefix + std::string(" \\cdot ") +  op_suffix + surround_in_bracks_if_needed(n, n->children.at(1), params, info);
		case OP_DIV:  return std::string("\\frac{") + raw_node_to_latex(n->children.at(0), info) + op_prefix + std::string("}{") + op_suffix + raw_node_to_latex(n->children.at(1), info) + std::string("}");
		case OP_POW:  return surround_in_bracks_if_needed(n, n->children.at(0), params, info, POW_BASE) + op_prefix + std::string("^") + op_suffix + surround_in_braces_if_len_gt1(n->children.at(1), info);
		// TODO add config option to show either math "-" or text "-" for negative
		case OP_NEG:  return op_prefix + std::string("\\text{-}") + op_suffix + surround_in_bracks_if_needed(n, n->children.at(0), params, info);
		case OP_ANGLE: {
			std::string output;
			output = raw_node_to_latex(n->children.at(0), info);
			output += op_prefix + std::string(" \\measuredangle ") + op_suffix;
			output += surround_in_bracks_if_needed(n, n->children.at(1), params, info);
			if (info->degree()) {
				output += latex_degree;
			}
			return output;
			break;
		}
#if 0
		case OP_STO:  return prefix + raw_node_to_latex(n->children.at(0)) + suffix + "\\rightarrow" +
		                     "{" + raw_node_to_latex(n->children.at(1)) + "}";
#endif
		default:
			std::cerr << "unexpected operator " << n->get_op() << std::endl;
			return std::string("unexpected operator");
	}
}

std::string escape_tex_text(std::string arg) {
	std::string output = arg;

	int i = 0;
	while(true) {
		i = output.find("_", i);
		if (i == std::string::npos) { break; }
		output.replace(i, 1, "\\_");
		i += 2;
	}
	return output;
}

size_t find_single_underscore(std::string arg) {
	int pos = 0;
	while (true) {
		pos = arg.find("_", pos);
		if (pos == std::string::npos) {
			break;
		}
		if (pos+1 >= arg.size()) {
			return pos;
		} else if (arg.at(pos+1) == '_') {
			pos += 2;
			continue;
		} else {
			return pos;
		}
	}
	return std::string::npos;
}

std::string double_underscores_to_single(std::string arg) {
	std::string output = arg;
	int i = 0;
	while (true) {
		i = output.find("__", i);
		if (i == std::string::npos) { break; }
		output.replace(i,2, "_");
	}
	return output;
}

// used for variable and function names
// e.g. "e" should stay as "e"
//      "pi" should become "\\pi"
//      "myfunc" should become "\\texttt{myfunc}"
//      "sqrt" should become "\\sqrt"
//      alpha should become \alpha_1
//      "R_derp" should become "R_\text{derp}"
std::string name_to_latex(std::string name, bool allowed_underscore = true) {
	if (name.size() == 1) {
		return name;
	}

	if (is_supported_latex_symbol(name)) {
		return "\\" + name;
	}

	if (allowed_underscore && find_single_underscore(name) != std::string::npos) {
		int pos = find_single_underscore(name);
		std::string base_str  = name.substr(0,pos);
		std::string subscript = name.substr(pos+1,name.size());
		if (subscript.size() == 0) {
			subscript = "square";
		}
		return name_to_latex(base_str, false) + "_" + name_to_latex(subscript, false);
	}

	name = double_underscores_to_single(name);

	return "\\text{" + escape_tex_text(name) + "}";
	//    TODO: maybe if it has two sequential underscores, then convert it to a literal underscore,
	//          to allow for names like `my_var`, which aren't subscripts?
}

std::string flt_to_latex(calc_float_t flt, const struct calc_fmt_params &params, bool neg_symb=false) {
	bool is_neg = (flt < 0);
	std::string neg_prefix = "";
	if (neg_symb && flt < 0) {
		flt = -flt;
		neg_prefix = "\\text{-}";
	}
	calc_flt_fmt_t val_fmt = format_calc_float_params(flt, params);
	const char *fmt_str;
	// TODO figure out how to define array of args to avoid duplicating to both snprintf(null) 
	// and snprintf(&str);

	char str_buff[256];
	if (std::isnan(flt)) {
		std::snprintf(str_buff, sizeof(str_buff), "\\text{NaN}");
	} else if (std::isinf(flt)) {
		std::snprintf(str_buff, sizeof(str_buff), "%s\\infty", is_neg ? "-" : "");
	} else if (val_fmt.exponent == 0) {
		fmt_str = "%s%.*" CALC_FLOAT_FMT "f";
		//size_t size = std::snprintf(nullptr, 0, fmt_str, val_fmt.decimal_places, val_fmt.base);
		//str.resize(size+1);
		//std::sprintf(&str[0], fmt_str, val_fmt.decimal_places, val_fmt.base, val_fmt.exponent);
		std::snprintf(str_buff, sizeof(str_buff), fmt_str, neg_prefix.c_str(), val_fmt.decimal_places, val_fmt.base, val_fmt.exponent);
	} else {
		// negative numbers take up two places because of the negative sign,
		// exponents >= 10 also take up more than one place
		// so curly braces are needed on the exponent
		if (val_fmt.exponent < 0 || val_fmt.exponent >= 10) {
			fmt_str = "%s%.*" CALC_FLOAT_FMT "f \\cdot 10^{%d}";
		} else {
			fmt_str = "%s%.*" CALC_FLOAT_FMT "f \\cdot 10^%d";
		}
		//size_t size = std::snprintf(nullptr, 0, fmt_str, val_fmt.decimal_places, val_fmt.base, val_fmt.exponent);
		//str.resize(size+1);
		//std::sprintf(&str[0], fmt_str, val_fmt.decimal_places, val_fmt.base, val_fmt.exponent);
		std::snprintf(str_buff, sizeof(str_buff), fmt_str, neg_prefix.c_str(), val_fmt.decimal_places, val_fmt.base, val_fmt.exponent);
	}
	std::string str(str_buff);
	return str;
}

/**
 * has_rect: if true, puts brackets if necessary assuming a unit gets added to the end
 */
std::string val_to_latex_rect(const val_t *val_arg, const struct calc_fmt_params &params, bool has_unit) {
	std::string re_str, im_str;
	val_t val = *val_arg;
	val.im = round_to_zero_if_negligible(val.re, val.im);
	val.re = round_to_zero_if_negligible(val.im, val.re);
	bool im_neg = false;
	if (val.im < 0) {
		val.im = -val.im;
		im_neg = true;
	}
	if (val.re != 0.0 || (val.re == 0 && val.im == 0)) {
		re_str = flt_to_latex(val.re, params);
	}
	if (val.im != 0.0) {
		im_str = "j" + flt_to_latex(val.im, params);
	}
	if (re_str.size() > 0 && im_str.size() > 0) {
		std::string output = re_str + (im_neg ? " - " : " + ") + im_str;
		if (!has_unit) {
			return output;
		} else {
			return std::string("\\left(") + output + "\\right)";
		}
	} else if (re_str.size() > 0) {
		return re_str;
	} else {
		return (im_neg ? "-" : "") + im_str;
	}
}

// TODO show units?
std::string val_to_latex_polar(const val_t *val_arg,
                               const struct calc_fmt_params &params,
                               bool degrees) {
	calc_float_t mag, angle;
	rect_to_polar(*val_arg, &mag, &angle);
	if (degrees) {
		angle *= 180/M_PI;
	}
	std::string mag_str   = flt_to_latex(mag,   params);
	std::string angle_str = flt_to_latex(angle, params, true);

	if (degrees) {
		angle_str += latex_degree;
	}

	if (angle == 0) {
		return mag_str;
	} else {
		return mag_str + " \\measuredangle " + angle_str;
	}
}

std::string unit_info_to_latex(const UnitInfoParsed &unit_info,
                               int cursor_pos = -1,
                               const UnitCursorPosInfo *unit_pos_info = nullptr) {
	bool prefix_is_text;
	std::string prefix;
	if (unit_info.prefix == "u") {
		prefix_is_text = false;
		prefix = "\\mu";
	} else {
		prefix_is_text = true;
		prefix = unit_info.prefix;
	}


	bool base_is_text;
	std::string base;

	if (unit_info.base == "Ohm" ||
	    unit_info.base == "ohm") {
		base_is_text = false;
		base = "\\Omega";
	} else {
		// TODO are there any others?
		base_is_text = true;
		base = unit_info.base;
	}

	std::string output;

	// If both prefix and base are text, then combine them into one big text
	if (prefix_is_text && base_is_text) {
		if (cursor_pos != -1 &&
		    unit_pos_info != nullptr &&
		    unit_pos_info->base_unit_start_pos <= cursor_pos &&
		    cursor_pos <= unit_pos_info->base_unit_end_pos + 1) {
			output = insert_cursor(prefix + base, cursor_pos - unit_pos_info->base_unit_start_pos);
		} else {
			output = "\\text{" + prefix + base + "}";
		}
	} else {
#warning "TODO handle cursor in units with special prefixes/base units"
		if (prefix_is_text) {
			output += "\\text{" + prefix + "}";
		} else {
			output += prefix;
		}

		if (base_is_text) {
			output += "\\text{" + base + "}";
		} else {
			output += base;
		}
	}

	if (unit_info.pow != 1) {
		output += "^" + surround_str_in_braces_if_len_gt1(std::to_string(unit_info.pow));
	}
	
	return output;
}

std::string raw_unit_piece_to_latex(const std::string base, int pow) {
	if (pow == 0) {
		return "";
	} else if (pow == 1) {
		return "\\text{" + base + "}";
	} else {
		std::string pow_str;
		if (pow < 0) {
			pow_str = "\\text{-}" + std::to_string(-pow);
		} else {
			pow_str = std::to_string(pow);
		}
		return "\\text{" + base + "}" + "^" + surround_str_in_braces_if_len_gt1(pow_str);
	}
}

std::string raw_units_to_latex(const unit_dim_t &unit_dim) {
	std::string output = "";

	output += raw_unit_piece_to_latex("kg" , unit_dim.kg );
	if (unit_dim.kg  != 0) { output += "\\,"; }
	output += raw_unit_piece_to_latex("m"  , unit_dim.m  );
	if (unit_dim.m   != 0) { output += "\\,"; }
	output += raw_unit_piece_to_latex("s"  , unit_dim.s  );
	if (unit_dim.s   != 0) { output += "\\,"; }
	output += raw_unit_piece_to_latex("A"  , unit_dim.A  );
	if (unit_dim.A   != 0) { output += "\\,"; }
	output += raw_unit_piece_to_latex("K"  , unit_dim.K  );
	if (unit_dim.K   != 0) { output += "\\,"; }
	output += raw_unit_piece_to_latex("mol", unit_dim.mol);
	if (unit_dim.mol != 0) { output += "\\,"; }
	output += raw_unit_piece_to_latex("cd" , unit_dim.cd);
	return output;
}

std::string val_to_latex(const val_t *val_arg,
                         const struct calc_fmt_params &params,
                         const CalcData *calcData,
                         const std::vector<UnitInfoInput> *desired_units,
                         const unit_t                     *desired_units_val) {


	calc_float_t unit_scale_fac = 1.0;
	std::string unit_str_latex;
	if (!units_non_zero(val_arg->unit_dim)) {
		unit_str_latex = "";
	} else {
		if (desired_units == nullptr || desired_units->size() == 0) {
			std::unique_ptr<UnitInfoParsed> unit_info = unit_dim_to_string_nice_lookup(val_arg->unit_dim);
			if (unit_info != nullptr) {
				unit_str_latex = unit_info_to_latex(*unit_info);
			} else {
				unit_str_latex = raw_units_to_latex(val_arg->unit_dim);
			}
		} else {
			unit_str_latex = input_units_to_latex(calcData, desired_units);
			unit_scale_fac = desired_units_val->mag;
		}
	}

	std::string output;
	val_t val = *val_arg;
	val.re /= unit_scale_fac;
	val.im /= unit_scale_fac;
	
	if (!calcData->polar) {
		output = val_to_latex_rect(&val, params, unit_str_latex.size() > 0);
	} else {
		output = val_to_latex_polar(&val, params, calcData->degree);
	}
	if (unit_str_latex.size() > 0) {
		output += "\\," + unit_str_latex;
	}
	return output;
}



std::string input_unit_base_to_latex(const CalcData *calcData,
                                     const std::string &unit_str,
                                     int cursor_pos,
                                     const UnitCursorPosInfo *unit_pos_info) {
	if (calcData->units.find(unit_str) != calcData->units.end()) {
		UnitInfoParsed unit_info = calcData->units.at(unit_str);
		std::string output = unit_info_to_latex(unit_info, cursor_pos, unit_pos_info);
		return output;
	} else {
		//std::string output = "\\text{" + unit_str + "}";
		//insert_cursor
		if (unit_pos_info != nullptr &&
		    unit_pos_info->base_unit_start_pos <= cursor_pos &&
		    cursor_pos <= unit_pos_info->base_unit_end_pos+1) {
			return insert_cursor(unit_str, cursor_pos - unit_pos_info->base_unit_start_pos);
		} else {
			return "\\text{" + unit_str + "}";
		}
	}
}

std::string input_units_to_latex(const CalcData *calcData,
                                 const std::vector<UnitInfoInput> *input_units) {
	return input_units_to_latex(calcData, input_units, -1, nullptr);
}

std::string input_units_to_latex(const CalcData *calcData,
                                 const std::vector<UnitInfoInput> *input_units,
                                 int cursor_pos,
                                 const WipValVarTokenCursorPos *cursor_info) {
	std::string output = "";
	//for (UnitInfoInput input_unit : input_units) {
	if (cursor_info != nullptr && input_units->size() != cursor_info->unit_info.size()) {
		std::string msg = "input_units.size = " + std::to_string(input_units->size()) + ", "
		                  "cursor_info.units.size = " + std::to_string(cursor_info->unit_info.size());
		throw BaseCalcException(msg);
	}
	for (int i=0; i<input_units->size(); i++) {
		const UnitInfoInput &input_unit = input_units->at(i);
		const UnitCursorPosInfo *unit_cursor_info = nullptr;
		if (cursor_info != nullptr) {
			unit_cursor_info = &cursor_info->unit_info.at(i);
		}

		std::string unit_str = input_unit_base_to_latex(calcData, input_unit.base, cursor_pos, unit_cursor_info);
		if (unit_str.size() > 0) {
			if (i != 0) { output += "\\,"; }
			output += unit_str;
		}
		if (input_unit.wip_pow) {
			std::string pow_str;
			if (input_unit.pow_wip_str.size() > 0) {
				if (unit_cursor_info != nullptr &&
				    unit_cursor_info->pow_val_present &&
				    unit_cursor_info->pow_val_start_pos <= cursor_pos &&
				    cursor_pos <= unit_cursor_info->pow_val_end_pos) {
					int cursor_offset = cursor_pos - unit_cursor_info->pow_val_start_pos;
					pow_str = "{" + insert_cursor(input_unit.pow_wip_str, cursor_offset) + "}";
				} else {
					pow_str = "\\text{" + input_unit.pow_wip_str + "}";
				}
			} else {
				pow_str = "\\square";
			}
			output += "^" + pow_str;
		} else if (input_unit.pow != 1) {
			bool pow_is_neg = false;
			int pow_val_abs = input_unit.pow;
			if (pow_val_abs < 0) {
				pow_val_abs = -pow_val_abs;
				pow_is_neg = true;
			}
			std::string pow_val_latex = std::to_string(pow_val_abs);
			if (pow_is_neg) {
				pow_val_latex = NEG_SYMB_LATEX + pow_val_latex;
			}
			std::string pow_str = surround_str_in_braces_if_len_gt1(pow_val_latex);
			output += "^" + pow_str;
		}
	}
	return output;
}

// Simply converts an integer to a string,
// but puts "\text{-}" before it if negative instead of "-"
static std::string int_to_latex_str(int val) {
	bool is_neg = false;
	if (val < 0) {
		is_neg = true;
		val = -val;
	}

	std::string output = std::to_string(val);
	if (is_neg) {
		output = NEG_SYMB_LATEX + output;
	}

	return output;
}

// TODO I don't like how messy this "needs_bracks_out" is, just to put the
// units inside the brackets?
// TODO for now, I'm surrounding all scientific notation inputted values with brackets.
// I think this is generally okay, for multiplication it isn't necessary but is easier to follow:
// (1.23 * 10^2) * (4.56 * 10^-2) is easier to read than without the brackets.
// For raising one of these values to a power, it is messy.
// Ugh. Should I add yet another hack related to brackets and exponents? If the value
// is in scientific notation, then don't put the brackets? I think I already did
// the reverse
static std::string format_inputted_val_latex(std::string val_str, bool is_imag, bool *needs_bracks_out) {
	*needs_bracks_out = false;

	std::string mag_str = get_mag_no_exp_str(val_str);
	int pow             = get_pow_exp_str(val_str);

	std::string imag_prefix = "";
	if (is_imag) {
		imag_prefix = "j";
	}

	if (pow == 0) {
		return imag_prefix + mag_str;
	} else {
		(void)sci_not_pow_brackets_hack;
		*needs_bracks_out = true;
		return imag_prefix + mag_str + " \\cdot 10^" + surround_str_in_braces_if_len_gt1(int_to_latex_str(pow));
	}
}

std::string val_node_rect_to_latex(const CalcData *calcData,
                                   const NodeValueRect *node_val_rect,
                                   const struct calc_fmt_params &params) {
	//val_t val = node_val_rect->get_val(calcData, false); // rect shouldn't matter if it's radians or degrees
	std::string unit_str = input_units_to_latex(calcData, &node_val_rect->input_units);
	//std::string output   = val_to_latex_rect(&val, params, unit_str.size() > 0);
	bool needs_bracks = false;
	std::string output = format_inputted_val_latex(node_val_rect->val_str, node_val_rect->is_imag, &needs_bracks);
	if (unit_str.size() > 0) {
		output += "\\," + unit_str;
	}
	if (needs_bracks) {
		output = "\\left(" + output + "\\right)";
	}
	return output;
}

// TODO rename this from `node_polar_rect` to `node_val_polar`
std::string val_node_polar_to_latex(const CalcData *calcData,
                                    const NodeValuePolar *node_polar_rect,
                                    const struct calc_fmt_params &params) {
	bool is_degrees = calcData->degree;
	// TODO format strings with params
	std::string mag_str   = "\\text{" + node_polar_rect->mag_str + "}";
	std::string angle_str = node_polar_rect->angle_str;
	if (is_degrees) {
		angle_str += "^\\circ";
	}
	std::string output = mag_str + " \\measuredangle " + angle_str;
	std::string unit_str = input_units_to_latex(calcData, &node_polar_rect->input_units);
	if (unit_str.size() == 0) {
		return output;
	} else {
		return output + " " + unit_str;
	}
}

/**
 * force_show_brackets: if false, omit brackets for functions like \sqrt{x}
 * show_right_brack:    if `force_show_brackets` is true, decides whether to show
 *                      the right bracket or not
 */
std::string func_call_to_latex(NodeFunc *func_node, const InputInfo *info, bool show_cursor, bool force_show_brackets=false, bool show_right_brack=true) {
	std::string prefix = "";
	std::string suffix = "";
	if (show_cursor) {
		if (info->cursor_offset >= 0) {
			prefix = cursor_latex;
		} else {
			suffix = cursor_latex;
		}
	}
	if (latex_funcs.find(func_node->func_name) != latex_funcs.end()) {
		std::string latex_func_name = latex_funcs.at(func_node->func_name);
		std::string left_brack;
		std::string right_brack;
		if (!force_show_brackets) {
			left_brack = "";
			right_brack = "";
		} else {
			left_brack = "\\left(";
			if (!show_right_brack) {
				right_brack = "\\right.";
			} else {
				right_brack = "\\right)";
			}
		}
		return prefix + latex_func_name +
		       "{" +
		       left_brack +
		       raw_node_to_latex(func_node->args.at(0), info) +
		       right_brack +
		       "}" +
		       suffix;
	} else {
		std::string func_name;
		if (latex_func_symbols.find(func_node->func_name) != latex_func_symbols.end()) {
			func_name = latex_func_symbols.at(func_node->func_name);
		} else {
			func_name = name_to_latex(func_node->func_name);
		}
		// normal function calls always show brackets
		return prefix + func_name +
		       "\\left(" +
		       raw_node_to_latex(func_node->args.at(0), info) +
		       (show_right_brack ? "\\right)" : "\\right.") +
		       suffix;
	}
}

std::string insert_cursor(std::string str, int cursor_pos) {
	if (cursor_pos < 0) {
		throw BaseCalcException(std::string("cursor pos < 0 in ") + __func__);
	}
	if (cursor_pos > str.size()) {
		throw BaseCalcException(std::string("cursor pos > len in ") + __func__);
	}

	if (cursor_pos == str.size()) {
		return "\\text{" + str + "}" + cursor_latex;
	}

	std::string l_str = str.substr(0, cursor_pos);
	std::string r_str = str.substr(cursor_pos, str.size()); // TODO should this be str.size - cursor_pos ?

	std::string output = "";

	if (l_str.size() > 0) {
		output += "\\text{" + l_str + "}";
	}

	output += cursor_latex;

	if (r_str.size() > 0) {
		output += "\\text{" + r_str + "}";
	}

	return output;
}

std::string wip_token_to_latex(const NodeWipToken *wip_token, const InputInfo *info) {

	std::string output = "";
	WipValVarTokenCursorPos * cursor_info = nullptr;
	if (info->cursor_node == wip_token) {
		cursor_info = dynamic_cast<WipValVarTokenCursorPos*>(info->cursor_pos_info.get());
		if (cursor_info == nullptr) {
			throw BaseCalcException("cursor_info is not correct instance when cursor_node is set to this token");
		}
	}
	bool inserted_cursor = false;
	bool inserted_cursor_at_end = false;
	bool no_whitespace_between_val_and_unit = false;
	if (info->cursor_node == wip_token && cursor_info->base_contains_pos(info->cursor_pos)) {
		int cursor_offset =  info->cursor_pos - cursor_info->base_val_start_pos;
		output = insert_cursor(wip_token->wip_token, cursor_offset);

		inserted_cursor = true;
		if (cursor_offset == cursor_info->base_val_end_pos+1) {
			inserted_cursor_at_end = true;
		}

		if (cursor_info->unit_info.size() > 0 &&
		    cursor_info->unit_info.at(0).base_unit_start_pos == cursor_info->base_val_end_pos+1) {
			no_whitespace_between_val_and_unit = true;
		}
	} else {
		output = "\\text{" + wip_token->wip_token + "}";
	}

#if 0
	if (wip_token->wip_angle.size() > 0) {

		output += " \\measuredangle ";
		if (info->cursor_node == wip_token && info->cursor_offset < 0) {
			int angle_len = wip_token->wip_angle.size();
			// TODO remove this offset stuff, this is hideous. Add a cusror_offset enum field
			// that has separate values for "angle" and "magnitude"
			output += insert_cursor(wip_token->wip_angle, -1-info->cursor_offset);
		} else {
			output += wip_token->wip_angle;
		}
		if (info->degree()) {
			output += "^\\circ";
		}
	}
#endif

	if (wip_token->wip_units.size() > 0) { 
		std::string units = input_units_to_latex(info->calcData,
		                               &wip_token->wip_units,
		                               info->cursor_pos,
		                               /* don't pass cursor info if cursor was already placed, */
		                               /* to avoid duplicate cursors */
		                               inserted_cursor ? nullptr : cursor_info);
		if (inserted_cursor_at_end && no_whitespace_between_val_and_unit) {
			// don't add whitespace here, the cursor is between them
			// and separation is ambiguous, if the user types a digit then it's attached to
			// the value,
			// if the user types a unit character then it's attached to the unit
			// e.g. "1km"
			//        ^
			// should render to "\text{1}\cursor\text{km}
			// 
			// but "1 km" and "1 km"
			//       ^           ^
			// would render to, respectively:
			// "\text{1}\cursor\,\text{km}" and "\text{1}\,\cursor\text{km}"
		} else {
			output += "\\,";
		}
		output += units;

	}
	return output;
}

std::string var_node_to_latex(const NodeVar *var_node, const InputInfo *info) {
	std::string prefix = "";
	std::string suffix = "";
	if (var_node == info->cursor_node) {
		if (info->cursor_offset >= 0) {
			prefix = cursor_latex;
		} else {
			suffix = cursor_latex;
		}
	}
	return prefix + name_to_latex(var_node->var_name) + suffix;
}
		

std::string raw_node_to_latex(Node *n, const InputInfo *info)  {
	std::string output;
	struct calc_fmt_params params = get_default_params();
	if (dynamic_cast<NodeOp*>(n) != nullptr) {
		output = op_node_to_latex(dynamic_cast<NodeOp*>(n), params, info);
	} else if (dynamic_cast<NodeVar*>(n) != nullptr) {
		NodeVar *var_node = dynamic_cast<NodeVar*>(n);
		output = var_node_to_latex(var_node, info);
	} else if (dynamic_cast<NodeFunc*>(n) != nullptr) {
		NodeFunc *func_node = dynamic_cast<NodeFunc*>(n);
		bool prefix_cursor = (info->cursor_node == func_node);
		output = func_call_to_latex(func_node, info, prefix_cursor);

	} else if (dynamic_cast<NodeValueRect*>(n) != nullptr) {
		NodeValueRect *val_rect_node = dynamic_cast<NodeValueRect*>(n);
		output = val_node_rect_to_latex(info->calcData, val_rect_node, params);
	} else if (dynamic_cast<NodeValuePolar*>(n) != nullptr) {
		NodeValuePolar *val_rect_node = dynamic_cast<NodeValuePolar*>(n);
		output = val_node_polar_to_latex(info->calcData, val_rect_node, params);
	} else if (dynamic_cast<NodeWipToken*>(n) != nullptr) {
		NodeWipToken *wip_token = dynamic_cast<NodeWipToken*>(n);
		output = wip_token_to_latex(wip_token, info);
	} else if (dynamic_cast<NodeWipBrackets*>(n) != nullptr) {
		NodeWipBrackets *wip_brackets = dynamic_cast<NodeWipBrackets*>(n);
		std::string prefix        = "";
		std::string suffix_before = "";
		std::string suffix_after  = "";
		if (info->cursor_node == wip_brackets) {
			if (info->cursor_offset == 0) {      prefix        = cursor_latex; }
			else if(info->cursor_offset == -1) { suffix_after  = cursor_latex; }
			else {                               suffix_before = cursor_latex; }
		}
		std::string to_return = prefix + "\\left(" + raw_node_to_latex(wip_brackets->arg, info);
		to_return += suffix_before;
		if (wip_brackets->right_brack_present) {
			to_return += "\\right)";
		} else {
			to_return += "\\right.";
		}
		to_return += suffix_after;
		output = to_return;
	} else if (dynamic_cast<NodeWipFuncCall*>(n) != nullptr) {
		NodeWipFuncCall* node_wip_func = dynamic_cast<NodeWipFuncCall*>(n);
		bool prefix_cursor = (info->cursor_node == node_wip_func);
		output = func_call_to_latex(node_wip_func->arg, info, prefix_cursor, true, node_wip_func->right_brack_present);
	} else if (dynamic_cast<NodeApplyUnits*>(n) != nullptr) {
		NodeApplyUnits* node_apply_units = dynamic_cast<NodeApplyUnits*>(n);
		std::string val_str = raw_node_to_latex(node_apply_units->n, info);
		if (dynamic_cast<NodeWipBrackets*>(node_apply_units->n) == nullptr) {
			val_str = "\\left(" + val_str + "\\right)";
		}
		output = val_str + "\\," + input_units_to_latex(info->calcData, &node_apply_units->unit_info.units);
	} else {
		output = "\\text{err: unexpected node type}";
	}
	if (debug_latex) {
		std::cout << "Returning latex \"" << output << "\" for node " << n->to_string() << std::endl;
	}
	return output;
}

std::string latex_sto_suffix(const InputInfo *info) {
	return " \\rightarrow " + name_to_latex(info->sto_var_name);
}

std::string node_to_latex(const InputInfo *parse_info) {
	std::string cursor_node_str = "null";
	if (parse_info->cursor_node) {
		cursor_node_str = parse_info->cursor_node->to_string();
	}
	if (debug_latex) {
		std::cout << "cursor_node: " << cursor_node_str << ", offset: " << parse_info->cursor_offset << std::endl;
	}

	std::string output = raw_node_to_latex(parse_info->n, parse_info);

	if (parse_info->has_to_unit) {
		output += "\\enspace (\\small{\\textit{to unit:}}\\normalsize\\;";
		if (parse_info->to_unit.size() == 0) {
			output += "\\square";
		} else {
			output += input_units_to_latex(parse_info->calcData, &parse_info->to_unit);
		}
		output += ")";
	}

	if (parse_info->has_sto) {
		output += latex_sto_suffix(parse_info);
	}

	return output;
}
