/*
 * This is the file with public APIs that take in strings and output strings of
 * json formatted output.
 *
 * Used by both:
 *   * Emscripten for WASM/JS/HTML, and
 *   * Android
 *
 */
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include "calc_parse.h"
#include "node_to_latex.h"
#include "calc_units_organizer.h"
#include "calc_core_ui.h"

#include "calc_json.h"


class CalcState {
	public:
	~CalcState(void) {
		delete calcData;
		delete calcUiState;
	};
	CalcData *calcData;
	CalcUiState *calcUiState;
};

static void log(const std::string &str) {
	// TODO maybe write to a file or something for the command line version,
	// dump to stdout for wasm
	(void)str;
}

std::string json_escape_str(std::string str) {
	// TODO figure out how comparisons with npos work
	// 	    for signed integers. In general I would prefer to use
	// 	    signed integers.
	int i;

	i = 0;
	while(true) {
		i = str.find("\\", i);
		if (i == std::string::npos) { break; }
		str.replace(i, 1, "\\\\");
		i += 2;
	}

	i = 0;
	while(true) {
		i = str.find("\"", i);
		if (i == std::string::npos) { break; }
		str.replace(i, 1, "\\\"");
		i += 2;
	}
	return str;
}
std::string json_escape_str(const char *str) {
	return json_escape_str(std::string(str));
}

const char *get_special_val_str(calc_float_t f) {
	if (std::isnan(f)) {
		return "NaN";
	} else if (std::isinf(f)) {
		if (f < 0) { return "-inf"; }
		else { return "inf"; }
	} else {
		return nullptr;
	}
}

std::string calc_float_to_json_str(calc_float_t val, int decimal_places) {
	const char *str_override = get_special_val_str(val);
	if (str_override != nullptr) {
		return std::string("\"") + std::string(str_override) + std::string("\"");
	} else {
		char val_str[64];
		snprintf(val_str, sizeof(val_str), "%.*" CALC_FLOAT_FMT "e", decimal_places, val);
		return std::string(val_str);
	}
}

std::string format_output_latex(const val_t val, const struct calc_fmt_params &params) {
	// TODO what was this for?
	return "";
}

void write_val_to_json_str(char *str_output, int str_output_len,
                           bool show_rc, int rc, int decimal_places, const val_t val,
                           const OutputInfo *output_info,
                           const CalcData *calcData) {
	struct calc_fmt_params params = get_default_params();
	const std::vector<UnitInfoInput> *desired_units = nullptr;
	if (output_info != nullptr) {
		desired_units = &output_info->output_unit_str;
	}
	std::string nice_str = val_to_latex(&val, params, calcData, desired_units, &output_info->unit);

	std::string re_str = calc_float_to_json_str(val.re, decimal_places);
	std::string im_str = calc_float_to_json_str(val.im, decimal_places);

	nice_str = json_escape_str(nice_str);

	std::string output_str_obj = "{";
	{
		char output_buff[512];
		if (show_rc) {
			snprintf(output_buff, sizeof(output_buff), "\"rc\": %d, \"re\": %s, \"im\": %s, \"val_str\": \"%s\"",
			         rc, re_str.c_str(), im_str.c_str(), nice_str.c_str());
		} else {
			snprintf(output_buff, sizeof(output_buff), "\"re\": %s, \"im\": %s, \"val_str\": \"%s\"",
			         re_str.c_str(), im_str.c_str(), nice_str.c_str());
		}
		output_str_obj += output_buff;
	}

	output_str_obj += ",";
	output_str_obj += "\"units_in_input\": [";
	bool first = true;
	for (const auto &unit_info : output_info->units_in_input) {
		if (!first) {
			output_str_obj += ", ";
		}
		first = false;
		output_str_obj += std::string("\"") + unit_info.to_string() + "\"";
	}
	output_str_obj += "]";

	output_str_obj += "}";

	snprintf(str_output, str_output_len, "%s", output_str_obj.c_str());

}

std::string calc_val_to_json_str(bool show_rc, int rc, int decimal_places, const val_t val, const OutputInfo *output, const CalcData *calcData) {
	char str_output[256];
	write_val_to_json_str(str_output, sizeof(str_output), show_rc, rc, decimal_places, val, output, calcData);
	return std::string(str_output);
}


void write_json_msg(char *str_output, int str_output_len,
                    int rc, const char *msg) {
	snprintf(str_output, str_output_len, "{ \"rc\": %d, \"msg\": \"%s\"}", rc, json_escape_str(msg).c_str());
}


int alexcalc_init(void) {
	return 0;
}

void *alexcalc_new_calcdata(void) {
	CalcState *calcState = new CalcState();
	calcState->calcData = new CalcData();
	calcState->calcData->units = get_units();
	calcState->calcUiState = new CalcUiState();
	return calcState;
}

void alexcalc_free_calcdata(void *arg) {
	CalcState *calcData = (CalcState*)arg;
	delete calcData;
}

/**
 * Computes output from input and updates recently used units.
 */
int alexcalc_json_str_output(const char *str_input, void *calc_ptr, char *str_output, int str_output_len) noexcept {
//	static const bool parse_only = false;

	log(std::string("calc: ") + str_input);
	
	CalcState *calcState = (CalcState*)calc_ptr;
	CalcData *calcData = calcState->calcData;

	InputInfo input_info;
	static const parse_params_s parse_params = {
		.cursor_pos = 0,
		.parse_wip = false,
	};

	if (Node::nodes_allocated > 0) {
		Node::print_allocated_nodes(std::cerr);
	}


	try { 
		input_info = calc_parse( str_input, &parse_params, calcData );
	} catch(const InvalidInputException &e) {
		std::cerr << "InvalidInputException when parsing: " << e.msg << std::endl;
		std::string to_return = "InvalidInputException when parsing: " + e.msg;
		int rc = -1;
		write_json_msg(str_output, str_output_len, rc, e.msg.c_str());
		log(std::string("calc_err1: ") + to_return);
		return 0; 
	} catch(const BaseCalcException &e ) {
		std::cerr << "BaseCalcException when parsing: " << e.msg << std::endl;
		int rc = -2;
		write_json_msg(str_output, str_output_len, rc, e.msg.c_str());
		log(std::string("calc_err2: ") + e.msg);
		return 0;
	}


#if 0
	if( parse_only ) {
		std::cerr << n->to_string() << std::endl;
		return n->to_string().c_str();
	}
#endif

	OutputInfo output;
	try {
		output = input_info.eval(calcData);

		std::cout << "Parsed \"" << str_input << "\" to: " << input_info.n->to_string()
		          << ", evaluated to: " << output.to_string() << std::endl;
	} catch(const BaseCalcException &e ) {
		std::cerr << "BaseCalcException when evaluating: " << e.msg << std::endl;
		int rc = -3;
		write_json_msg(str_output, str_output_len, rc, e.msg.c_str());
		log(std::string("calc_err3: ") + e.msg.c_str());
		delete input_info.n;
		return 0;
	} catch (std::exception& ex) {
		std::cerr << "std::exception when evaluating" << std::endl;
		int rc = -4;
		write_json_msg(str_output, str_output_len, rc, "std::exception");
		log(std::string("calc_err4"));
		delete input_info.n;
		return 0;
	} catch (...) {
		std::cerr << "other exception type" << std::endl;
		int rc = -5;
		write_json_msg(str_output, str_output_len, rc, "unknown err");
		log(std::string("calc_err5"));
		delete input_info.n;
		return 0;
	}
	// note that none of these will be hit in the wasm path if
	// `DISABLE_EXCEPTION_CATCHING=0` is not set (I think)

	delete input_info.n;
	calcState->calcUiState->update_recently_used_units(output.units_in_input);
	int rc = 0; // TODO
	write_val_to_json_str(str_output, str_output_len, true, rc, MAX_DECIMAL_PLACES, output.val, &output, calcData);
	log(std::string("calc_output: ") + std::string(str_output));
	return 0;
}

int alexcalc_to_latex_once(const char *str_input,
                           const void *calc_ptr,
                           char *str_output,
                           int str_output_len,
                           int cursor_pos,
                           bool parse_wip) noexcept {

	const parse_params_s parse_params = {
		.cursor_pos = cursor_pos,
		.parse_wip = parse_wip,
	};
	const CalcState *calcState = (CalcState*)calc_ptr;
	const CalcData *calcData = calcState->calcData;
	InputInfo input_info;

	// str_output[0] = 0;
	snprintf(str_output, str_output_len, "error");

	try { 
		input_info = calc_parse( str_input, &parse_params, calcData);
	} catch(const BaseCalcException &e ) {
		std::cerr << "BaseCalcException when parsing: " << e.msg << std::endl;
		return -1;
	}

	std::cout << "Parsed \"" << str_input << "\" to: " << input_info.n->to_string() << ", ";

	std::string latex = node_to_latex(&input_info);
	delete input_info.n;
	std::cout << "converted node into latex: " << latex;
	std::cout << std::endl;
	strncpy(str_output, latex.c_str(), str_output_len);
	return 0;
}

int alexcalc_to_latex(const char *str_input,
                      const void *calc_ptr,
                      char *str_output,
                      int str_output_len,
                      int cursor_pos,
                      bool parse_wip) noexcept {

	log(std::string("to_latex: ") + str_input);
	int rc = alexcalc_to_latex_once(str_input, calc_ptr, str_output, str_output_len, cursor_pos, parse_wip);
	//if (rc != 0 && parse_wip) {
	if (rc != 0) {
		std::cout << "Could not convert \"" << str_input << "\" to string, trying with \\square added to the end" << std::endl;
		std::string str_input2 = std::string(str_input) + " square";
		rc = alexcalc_to_latex_once(str_input2.c_str(), calc_ptr, str_output, str_output_len, cursor_pos, parse_wip);
	}
	return rc;
}

int alexcalc_calcdata_to_json(void *calc_ptr, char *str_output_arg, int str_output_len) noexcept {
	struct calc_fmt_params params = get_default_params();
	//const int decimal_places = 6;
	CalcState *calcState = (CalcState*)calc_ptr;
	CalcData *calcData = calcState->calcData;
	std::string output = "{";
	output += "\"vars\": [";
	for (auto it = calcData->vars.begin(); it != calcData->vars.end(); it++) {
		const std::string name = it->first;
		const val_t       val  = it->second;
		if (it != calcData->vars.begin()) {
			output += ", ";
		}
		//output += "[ \"" + name + "\", " + calc_val_to_json_str(false, 0, decimal_places, val, nullptr, calcData) + "]";
		//std::string val_str = val_to_latex(&val, params, calcData, nullptr, nullptr);
		std::string val_str = val_to_string(&val, params, nullptr);
		val_str = json_escape_str(val_str);
		output += "[ \"" + name + "\", \"" + val_str + "\"]";
	}
	output += "],";

	output += std::string("\"polar\": ")  + (calcData->polar  ? "true" : "false") + ",";
	output += std::string("\"degree\": ") + (calcData->degree ? "true" : "false");

	output += "}";
	snprintf(str_output_arg, str_output_len, "%s", output.c_str());
	return 0;
}

int alexcalc_data_state_set(void *calc_ptr, bool polar, bool degree) noexcept {
	CalcState *calcState = (CalcState*)calc_ptr;
	CalcData *calcData = calcState->calcData;
	if (calcData == nullptr) { return -1; }
	calcData->polar  = polar;
	calcData->degree = degree;
	std::cout << "Setting calcData polar:" << calcData->polar << ", degree: " << calcData->degree << std::endl;
	return 0;
}

int alexcalc_get_unit_info_json(char *unit_info_str_out, int unit_info_str_len) {
	UnitGroup root = get_unit_info();
	snprintf(unit_info_str_out, unit_info_str_len, "%s", root.to_json().c_str());
	return 0;
}

int alexcalc_get_unit_info_group_count(void) {
	return get_unit_info().group_count();
}

int alexcalc_get_unit_info_group_item_count(int group_idx) {
	return get_unit_info().get_item_count(group_idx);
}

int alexcalc_get_unit_info_json_group_name(char *unit_info_str_out, int unit_info_str_len,
		int group_idx) {
	std::string group_name = get_unit_info().get_group_name(group_idx);
	snprintf(unit_info_str_out, unit_info_str_len, "%s", group_name.c_str());
	return 0;
}

int alexcalc_get_unit_info_json_item(char *unit_info_str_out, int unit_info_str_len,
										   int group_idx, int item_idx) {
	std::string json_str = get_unit_info().get_item_json(group_idx, item_idx);
	snprintf(unit_info_str_out, unit_info_str_len, "%s", json_str.c_str());
	return 0;
}

int alexcalc_add_recently_used_unit(void *calc_ptr,
                                    const char *unit_str,
                                    int unit_str_len) {
	CalcState *calcState = (CalcState*)calc_ptr;

	// TODO
	#warning TODO

	std::string str_input(unit_str);
	if (str_input.size() != unit_str_len) {
		// TODO
		return -3;
	}

	int input_pos = 0;
	std::vector<UnitInfoInput> unit_vec;
	bool found = parse_unit(&str_input, &input_pos, &unit_vec);
	if (!found) {
		return -1;
	} else if (input_pos != unit_str_len) {
		return -2;
	}

	// TODO clean this up
	UnitInfoInputAry unit;
	unit.units = unit_vec;

	std::vector<UnitInfoInputAry> units;
	units.push_back(unit);
	calcState->calcUiState->update_recently_used_units(units);
	return 0;
}

int alexcalc_get_recently_used_units_json(void *calc_ptr,
                                          char *units_str_out,
                                          int units_str_out_len) {
	CalcState *calcState = (CalcState*)calc_ptr;

	std::string output = "{ \"units\": [";
	std::list<UnitInfoInputAry> units;
	calcState->calcUiState->get_recently_used_units(&units);
	bool first = true;
	for (UnitInfoInputAry unit : units) {
		if (!first) { output += ","; }
		first = false;
		output += "\"" + json_escape_str(unit.to_string()) + "\"";
	}
	output += "]}";
	
	snprintf(units_str_out, units_str_out_len, "%s", output.c_str());
	return 0;
}

int alexcalc_delete_recently_used_units(void *calc_ptr) {
	if (calc_ptr == nullptr) {
		std::cerr << __func__ << ": null ptr" << std::endl;
		return -1;
	}
	CalcState *calcState = (CalcState*)calc_ptr;
	calcState->calcUiState->delete_recently_used_units();
	return 0;
}

int alexcalc_delete_vars(void *calc_ptr) {
	if (calc_ptr == nullptr) {
		std::cerr << __func__ << ": null ptr" << std::endl;
		return -1;
	}
	CalcState *calcState = (CalcState*)calc_ptr;
	calcState->calcData->delete_vars();
	return 0;
}
