

#ifndef __CALC_PARSE_H__
#define __CALC_PARSE_H__


#include<string>

#include "calc_core.h"

typedef struct {
    /**
     * position of cursor, where 0 is before first character, and length of string
     * is after the last character.
     * Will only be valid if parse_wip is true, I think
     */
    int cursor_pos;

	/** true if the user might be in the middle of typing this,
	 * (at any position in the string), so allow parsing of things
	 * like "1e" or "1e-" or "1."
	 *
	 * store them in a new node that preserves the original string,
	 * which gets converted back to the same string.
	 * The goal here is to provide nice latex formatting for things like fractions and square roots,
	 * but to otherwise be close to the input, so in general one button press by the user
	 * corresponds to one change to the LaTeX output
	 * 
	 * Should also show brackets in separate node, so that user inputted brackets always
	 * show up. Also allows for rightmost bracket to not be shown
	 */
	bool parse_wip;
} parse_params_s;

typedef struct {
	std::string deg;
	std::string min;
	std::string sec;
} deg_min_sec_s;


extern bool debug_collapse;

//Node * calc_parse( std::string str_input, const parse_params_s *params);
InputInfo calc_parse( std::string str_input, const parse_params_s *params, const CalcData *calcData);
bool calc_check_for_sto(const std::string str_input, std::string *expression_out, std::string *name_out);
bool cmd_parse(std::string str_input, std::string *cmd_out, std::string *args);

// Should only be used for adding units to the "recently used units" section.
bool parse_unit(std::string *str_input, int *input_pos, std::vector<UnitInfoInput> *unit_out);


calc_float_t parse_float_w_optional_deg_str(std::string str);
bool separate_deg_min_sec(std::string str, deg_min_sec_s *output);
bool is_numeric_only(std::string arg);

int parse_test_cases(void);


#endif
