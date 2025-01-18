#include<assert.h>

#include<memory>
#include<cstdlib>
#include<sstream>
#include<vector>
#include<list>
#include<iostream>
#include<regex>
#include<string>
#include<cmath> // TODO remove this

#include "calc_core.h"
#include "misc.h"

#include "calc_parse.h"

// these aren't needed with the angle operator, should always evaluate to the same thing,
// more or less
#define LITERAL_ANGLES_ENABLED 0

// static const char ANGLE_OP_SYMBOL[] = "angle";
#define ANGLE_OP_SYMBOL "angle"

bool str_starts_with(const std::string &str, const std::string &prefix) {
	return str.size() >= prefix.size() &&
	       str.substr(0,prefix.size()).compare(prefix) == 0;
}

int get_closing_brack_pos( const std::string str_input );
Node * calc_parse_internal( std::string str_input, int start_pos, InputInfo *info_out, const parse_params_s *params);

class ValInfo {
	public:
	static ValInfo* FromRect(bool is_imag, std::string mag_str) {
		ValInfo *toReturn = new ValInfo();
		toReturn->is_rect = true;
		toReturn->is_rect_imag = is_imag;
		toReturn->mag_str = mag_str;
		return toReturn;
	}

	static ValInfo* FromPolar(std::string mag_str, std::string angle_str) {
		ValInfo *toReturn = new ValInfo();
		toReturn->is_rect   = false;
		toReturn->mag_str   = mag_str;
		toReturn->angle_str = angle_str;
		return toReturn;
	}
	bool is_rect;
	bool is_rect_imag;
	std::string mag_str;
	std::string angle_str;

	private:
	ValInfo(void){};
};


bool parse_value( std::string *str_input, int *input_pos, InputInfo *info_out, std::unique_ptr<ValInfo> *val_info_out, const parse_params_s *params)
{
	static const std::regex raw_val_regex( "^"
	                                       "\\s*"
                                          "([ij])?" // optional imaginary unit at the beginning
	                                       "("
	                                           // "-?" // there's unary negative, separate from this
	                                          "[0-9]*"
	                                          "(\\.[0-9]+)?"   // optional decimal
	                                          "([eE]-?[0-9]+)?"  // optional exponent
	                                       ")"
	                                       "([ij])?" // optional imaginary unit at the end
#if LITERAL_ANGLES_ENABLED
	                                       "(?:" // optional angle at end if followed by "angle<angle>"
	                                          ANGLE_OP_SYMBOL // literal "angle" symbol for angle
	                                          "(" // actual numeric value that we want to capture for angle
	                                             "(?:-)?" // optional negative sign
	                                             "[0-9]+" // 
	                                             "(?:\\.[0-9]+)?" // optional decimal after angle
	                                          ")"
	                                        ")?" 
#endif
	                                       );
	std::smatch result;

	bool found = std::regex_search( *str_input,
	                                result,
	                                raw_val_regex ); //,
	                                //std::regex_constants::basic );

	if( !found ) {
		//std::cout << "did not find raw val in s: \"" << str_input << "\"" << std::endl;
		return false;
	}

	int         match_len   = result.str(0).size();

	std::string img_unit_prefix = result.str(1);
	std::string raw_val_str     = result.str(2);
	if (raw_val_str.size() == 0) {
		return false;
	}
	if (str_starts_with(raw_val_str, "e") || str_starts_with(raw_val_str, "E")) {
		return false;
	}
	// decimal
	// exponent
	std::string img_unit_suffix = result.str(5);
#if LITERAL_ANGLES_ENABLED
	std::string angle_suffix    = result.str(6);
#endif

	// if this is true, then the value does not have an angle
	bool is_rect;

	// if this is true, then the value is rect and is imaginary only
	// (i.e. starts or ends with "i" or "j")
	bool is_rect_imag;

	// can only specify one of:
	//  * img unit at beginning or end, or
	//  * use polar notation
	int img_counts = ( (img_unit_prefix.size() > 0) ? 1 : 0 ) +
	                 ( (img_unit_suffix.size() > 0) ? 1 : 0 ) +
#if LITERAL_ANGLES_ENABLED
	                 ( (angle_suffix.size()    > 0) ? 1 : 0 );
#else
	                 0;
#endif
	if (img_counts > 1) {
		return false;
	} else if (img_unit_prefix.size() > 0 ||
	           img_unit_suffix.size() > 0) {
		is_rect      = true;
		is_rect_imag = true;
#if LITERAL_ANGLES_ENABLED
	} else if (angle_suffix.size() > 0) {
		is_rect = false;
		// is_rect_imag = false; // should be unused if is_rect is false
#endif
	} else {
		is_rect      = true;
		is_rect_imag = false;
	}

	//std::string base            = result.str(2);
	//std::string decimal         = result.str(3);
	//std::string exponent        = result.str(4);

	/* remove the match from str_input */
	str_input->erase( 0, match_len );
	*input_pos += match_len;

	//calc_float_t mag = std::atof(raw_val_str.c_str());
	if (is_rect) {
		val_info_out->reset(ValInfo::FromRect(is_rect_imag, raw_val_str));
	} else {
#if LITERAL_ANGLES_ENABLED
		val_info_out->reset(ValInfo::FromPolar(raw_val_str, angle_suffix));
#else
		assert(false);
#endif
	}
	//std::cout << "Converted val \"" << raw_val_str << "\" to re: " << val->re << ", im: " << val->im << std::endl;
	return true;

}

bool parse_unit(std::string *str_input, int *input_pos, std::vector<UnitInfoInput> *unit_out) {
	static const std::regex unit_regex( "^"
	                                    "\\s*"
	                                    "(?:"
	                                      "([a-zA-Z][a-zA-Z0-9]*)"
	                                      "(?:\\^(-?[0-9]+))?"
	                                    ")");
	bool found_any = false;
	while (true) {
		std::smatch result;

		bool found = std::regex_search( *str_input,
		                                result,
		                                unit_regex );
		if (!found) {
			break;
		}

		int match_len = result.str(0).size();
		std::string unit_base    = result.str(1);
		std::string unit_pow_str = result.str(2);


		if (str_starts_with(unit_base, ANGLE_OP_SYMBOL)) {
			break;
		}
		found_any = true;

		int unit_pow;
		if (unit_pow_str.size() > 0) {
			unit_pow = std::stoi(unit_pow_str);
		} else {
			unit_pow = 1;
		}

		str_input->erase(0, match_len);
		*input_pos += match_len;

		unit_out->push_back(UnitInfoInput(unit_base, unit_pow));

	}

	return found_any;
}


bool parse_var_name(std::string *str_input, int *input_pos, std::string *var_name_out) {
	static const std::regex var_name_regex( "^"
	                                       "\\s*"
	                                       "("
	                                           // "-?" // there's unary negative, separate from this
	                                          "[a-zA-Z]+"
	                                          "[a-zA-Z_0-9]*"
	                                       ")" );
	std::smatch result;

	bool found = std::regex_search( *str_input,
	                                result,
	                                var_name_regex ); //,
	                                //std::regex_constants::basic );

	if( !found ) {
		return false;
	}

	int match_len = result.str(0).size();
	*var_name_out = result.str(1);

	/* remove the match from str_input */
	str_input->erase( 0, match_len );
	*input_pos += match_len;

	return true;

}

bool parse_func_call(std::string *str_input,
                     int         *input_pos,
                     std::string *func_name_out,
                     int         *args_start_pos_out,
                     std::string *func_args_str_out,
                     bool        *found_close_brack_out) {
	static const std::regex func_call_regex( "^"
	                                       "\\s*"
	                                       "("
	                                          "[a-zA-Z_]+"
	                                          "[a-zA-Z_0-9]*"
	                                          //"\\s*" // could allow whitespace but meh
	                                          "\\("    // a literal open bracket
	                                       ")"
	                                        );
	std::smatch result;

	bool found = std::regex_search( *str_input,
	                                result,
	                                func_call_regex ); //,
	                                //std::regex_constants::basic );

	if( !found ) {
		return false;
	}


	int match_len = result.str(0).size();
	// includes the open bracket
	std::string func_name = result.str(1);
	// remove open bracket
	func_name.erase( func_name.size()-1, func_name.size() );
	*func_name_out = func_name;

	/* remove the match from str_input, but leave the open bracket for now */
	str_input->erase(0, match_len-1);
	*args_start_pos_out = *input_pos + match_len;
	*input_pos += match_len-1;

	bool found_close_brack;
	int close_bracket_pos;
	try {
		close_bracket_pos = get_closing_brack_pos( *str_input );
		found_close_brack = true;
	} catch(const NoClosingBracketsException &ex ) {
		found_close_brack = false;
		close_bracket_pos = str_input->size();
	}
	*found_close_brack_out = found_close_brack;

	// remove the open and close brackets from the function body
	*func_args_str_out = str_input->substr( 1, close_bracket_pos-1 );

	str_input->erase(0, close_bracket_pos);
	*input_pos += close_bracket_pos;
	// also erase the lingering close bracket, if it exists
	if (found_close_brack) {
		str_input->erase(0, 1);
		*input_pos += 1;
	}

	return true;

}


std::vector<Node*> parse_func_args(std::string func_args_str, int start_pos, InputInfo *info, const parse_params_s *params) {
	std::vector<Node*> args;
	// TODO handle more than one function argument, delimited by comma
	Node *n = calc_parse_internal(func_args_str, start_pos, info, params);
	args.push_back(n);
	return args;
}

bool parse_wip_valvar_token(std::string *str_input,
                            int *input_pos,
                            std::string *wip_token_out,
                            std::string *wip_angle_out,
                            std::vector<UnitInfoInput> *wip_units_out,
                            WipValVarTokenCursorPos *cursor_pos_info_out) {


// TODO share this with parse_unit
// Note that the first group must be checked to see if it starts with ANGLE_OP_SYMBOL,
// otherwise "angle" or "angle90" will be recognized as a unit
#define UNIT_WIP_REGEX 	"^([a-zA-Z][a-zA-Z0-9]*)"     \
	                     "(\\^(-?[0-9]*))?" \
	                     "\\s*"         \
                        


	// a more permissive version of the val regex, that allows
	// for partially written values
	static const std::regex wip_token_regex( "^"
	                                       "\\s*"
	                                       "("
	                                          "[ij]?"
	                                          "[0-9]*"
	                                          "\\.?[0-9]*"
	                                          "(?:[eE]-?[0-9]*)?"
	                                          "[ij]?"
	                                       ")" 
#if LITERAL_ANGLES_ENABLED
	                                       "(" // optional angle at end if followed by "angle<angle>"
	                                          ANGLE_OP_SYMBOL // literal "angle" symbol for angle
	                                          "(" // actual numeric value that we want to capture for angle
	                                             "(?:-)?" // optional negative sign
	                                             "[0-9]*" // 
	                                             "(?:\\.[0-9]*)?" // optional decimal after angle
	                                          ")"
	                                        ")?" 
#endif

	                                       "(\\s*)" // whitespace separating units
#if 0
	                                       "(" 
												"(?:" UNIT_WIP_REGEX ")*"
	                                       ")" 
#endif
	                                       );

	static const std::regex wip_unit_regex(UNIT_WIP_REGEX);

	std::smatch result;

	bool found = std::regex_search( *str_input,
	                                result,
	                                wip_token_regex ); //,
	                                //std::regex_constants::basic );

	if( !found ) {
		return false;
	}

	int         match_len     = result.str(0).size();
	std::string wip_token_str = result.str(1);
	if (wip_token_str.size() == 0 || wip_token_str == "i" || wip_token_str == "j") {
		return false;
	}
#if LITERAL_ANGLES_ENABLED
	std::string angle_suffix_whole = result.str(2);
	std::string wip_angle_str = result.str(3);
	if (angle_suffix_whole.size() > 0) {
		*angle_marker_offset_out = wip_token_str.size();
	}
	#error "fix index of below match"
	#error "add angle cursor pos info"
#endif
	std::string whitespace_between_val_and_unit = result.str(2);
	//std::string wip_units_str = result.str(3);
	cursor_pos_info_out->base_val_start_pos = *input_pos;
	cursor_pos_info_out->base_val_end_pos   = *input_pos + wip_token_str.size() - 1;
	int unit_input_pos = *input_pos + wip_token_str.size() + whitespace_between_val_and_unit.size();

	/* remove the match from str_input */
	str_input->erase( 0, match_len );
	*input_pos += match_len;

	while (true) {
		std::smatch result;
		bool found = std::regex_search(*str_input, result, wip_unit_regex);
		if (!found) { break; }

		std::string base = result.str(1);
		//if (base == "to") { break; }
		//if (base == ANGLE_OP_SYMBOL) { break; }
		if (str_starts_with(base, ANGLE_OP_SYMBOL)) {
			if (debug_collapse) std::cerr << "found angle symbol in WIP token" << std::endl;
			break;
		}
		std::string pow_str_incl_symb  = result.str(2);
		std::string pow_int_str        = result.str(3);
		bool        pow_symb_present = pow_str_incl_symb.size() > 0;
		assert(pow_str_incl_symb.size() == 0 || pow_int_str.size() < pow_str_incl_symb.size());

		UnitCursorPosInfo unit_cursor_info;
		unit_cursor_info.base_unit_start_pos = unit_input_pos;
		unit_cursor_info.base_unit_end_pos   = unit_input_pos + base.size() - 1;

		UnitInfoInput unit_info(base, 0);
		if (!pow_symb_present) {
			unit_info.wip_pow = false;
			unit_info.pow = 1;
		} else {
			unit_info.wip_pow = true;
			unit_info.pow_wip_str = pow_int_str;

			unit_cursor_info.pow_symbol_present = true;
			unit_cursor_info.pow_val_present = true;
			unit_cursor_info.pow_symbol_pos = unit_input_pos + base.size();
			unit_cursor_info.pow_val_start_pos = unit_cursor_info.pow_symbol_pos + 1;
			unit_cursor_info.pow_val_end_pos   = unit_cursor_info.pow_val_start_pos + pow_int_str.size();
		}
		wip_units_out->push_back(unit_info);
		str_input->erase(0, result.str(0).size());
		*input_pos += result.str(0).size();

		cursor_pos_info_out->unit_info.push_back(unit_cursor_info);
		unit_input_pos += result.str(0).size();
	}

	*wip_token_out = wip_token_str;
#if LITERAL_ANGLES_ENABLED
	*wip_angle_out = wip_angle_str;
#endif

	return true;
}


void handle_cursor_node(int prev_pos, int current_pos, Node *node, InputInfo *info_out, const parse_params_s *params) {
	if (params->parse_wip) {
		if (prev_pos <= params->cursor_pos &&
		    //params->cursor_pos < current_pos) {
		    params->cursor_pos <= current_pos) {
			info_out->cursor_node = node;
			info_out->cursor_offset = params->cursor_pos - prev_pos;
			if (debug_collapse) {
				std::cout << "found cursor pos " << params->cursor_pos << " in range "
			              << "[" << prev_pos << ", " << current_pos << "], offset=" << info_out->cursor_offset
			              << ", node=" << node->to_string() << std::endl;
			}
		}
	}
	
}

bool parse_non_op( std::string *str_input, int *input_pos, Node** n_out, InputInfo *info_out, const parse_params_s *params) {
	if (params->parse_wip) {
		std::string wip_token, wip_angle;
		//int prev_pos = *input_pos;
		//bool found_cursor = false;
		auto cursor_pos_info = std::unique_ptr<WipValVarTokenCursorPos>(new WipValVarTokenCursorPos());
		std::vector<UnitInfoInput> wip_units;
		bool found_val = parse_wip_valvar_token(str_input, input_pos, &wip_token, &wip_angle, &wip_units, cursor_pos_info.get());
		if (found_val) {
			*n_out = new NodeWipToken(wip_token, wip_angle, wip_units);
			bool cursor_in_node = cursor_pos_info->contains_pos(params->cursor_pos);
			if (cursor_in_node) {
				info_out->cursor_node = *n_out;
				info_out->cursor_pos_info.reset(cursor_pos_info.release());
			}
			return true;
		}
	}

	{
		std::unique_ptr<ValInfo> val_info;
		int prev_pos = *input_pos;
		bool found_val = parse_value(str_input, input_pos, info_out, &val_info, params);
		if (found_val) {
			NodeValue *n_value;
			if ((*val_info).is_rect) {
				n_value = new NodeValueRect((*val_info).mag_str, (*val_info).is_rect_imag);
			} else {
				n_value = new NodeValuePolar((*val_info).mag_str, (*val_info).angle_str);
			}
			handle_cursor_node(prev_pos, *input_pos, *n_out, info_out, params);

			std::vector<UnitInfoInput> unit_info;
			bool found_unit = parse_unit(str_input, input_pos, &unit_info);
			if (found_unit) {
				n_value->input_units = unit_info;
			}
			*n_out = n_value;
			return true;
		}
	}

	{
		int func_start_pos = *input_pos;
		int args_start_pos = -1;
		std::string func_name;
		std::string func_args_str;
		bool found_close_brack;
		Node *old_cursor_node = info_out->cursor_node;
		bool found_func_call = parse_func_call(str_input, input_pos, &func_name, &args_start_pos, &func_args_str, &found_close_brack);
		if (found_func_call) {
			std::vector<Node*> func_args = parse_func_args(func_args_str, args_start_pos, info_out, params);
			NodeFunc *func_node = new NodeFunc(func_name, func_args);
			if (params->parse_wip) {
				*n_out = new NodeWipFuncCall(func_node, found_close_brack);
			} else {
				*n_out = func_node;
			}
			if (old_cursor_node == info_out->cursor_node) {
				handle_cursor_node(func_start_pos, *input_pos, *n_out, info_out, params);
				if (*input_pos == params->cursor_pos) {
					info_out->cursor_offset = -1;
				}
			}
			// this is only for the "my_func(" part of the call
			return true;
		}
	}

	{
		std::string var_name;
		int prev_pos = *input_pos;
		bool found_var_name = parse_var_name(str_input, input_pos, &var_name);
		if (found_var_name) {
			*n_out = new NodeVar(var_name);
			handle_cursor_node(prev_pos, *input_pos, *n_out, info_out, params);
			if (*input_pos == params->cursor_pos) {
				info_out->cursor_offset = -1;
			}
			return true;
		}
	}
	return false;
}


bool parse_op_str( std::string *str_input, int *input_pos, std::string * op_str )
{
	static const std::regex op_str_regex( "^"
	                                      "\\s*"
	                                      "("
	                                         "[+-/*^]"
	                                         "|" ANGLE_OP_SYMBOL // "\\b"
	                                      ")" );
	std::smatch result;

	bool found = std::regex_search( *str_input,
	                                result,
	                                op_str_regex ); //,
	                                //std::regex_constants::basic );

	if( !found ) {
		//std::cout << "did not find raw val in s: \"" << str_input << "\"" << std::endl;
		return false;
	}

	int         match_len   = result.str(0).size();
	*op_str                 = result.str(1);

	/* remove the match from str_input */
	str_input->erase( 0, match_len );
	*input_pos += match_len;

	//std::cout << "found " << match_len << " chars." << std::endl;
	//std::cout << "found op_char: \"" << *op_str << "\"" << std::endl;

	//std::cout << "rest:      \"" << *str_input  << "\"" << std::endl;

	return true;

}


NodeOp * op_str_to_node( std::string   op_str,
                         bool          prev_node_needs_no_args )
{
	if      ( op_str == "+" ) { return new NodeOp( OP_ADD  ); }
	else if ( op_str == "*" ) { return new NodeOp( OP_MULT ); }
	else if ( op_str == "/" ) { return new NodeOp( OP_DIV  ); }
	else if ( op_str == "^" ) { return new NodeOp( OP_POW  ); }
	else if ( op_str == "-" ) {
		//if( prev_node_is_val ) { std::cout << "found sub" << std::endl; }
		//else                   { std::cout << "found neg" << std::endl; }
		if( prev_node_needs_no_args ) { return new NodeOp( OP_SUB ); }
		else                          { return new NodeOp( OP_NEG ); }
	} 
	else if (/*op_str == "L" || */op_str == ANGLE_OP_SYMBOL) { return new NodeOp(OP_ANGLE); }
	else {
		std::string msg = "unexpected op_str ";
		msg = msg + "'" + op_str + "'";
		throw BaseCalcException( msg ); /* TODO replace with internal calc exception */
	}
}


int get_closing_brack_pos( const std::string str_input ) {

	static const char bracket_types[][2] = {
		{ '(', ')' },
		{ '[', ']' },
		{ '{', '}' },
	};

	//#define BRACKET_TYPES ( sizeof(bracket_types[0]) / sizeof(bracket_types) )
	#define BRACKET_TYPES (3)

	std::vector<uint32_t> brack_stack;

	//int bracket_depths[BRACKET_TYPES];

	//for( int i=0; i<BRACKET_TYPES; i++ ) { bracket_depths[i] = 0; }

	for( uint32_t i=0; i<str_input.size(); i++ ) {
		//std::cout << "checking pos " << i << "char " << str_input[i] << std::endl;
		for( uint32_t bracket_index=0; bracket_index<BRACKET_TYPES; bracket_index++ ) {
			//std::cout << bracket_types[bracket_index][0] << ", " << (bracket_types[bracket_index][0] == str_input[i] ) << std::endl;
			//std::cout << bracket_types[bracket_index][1] << ", " << (bracket_types[bracket_index][1] == str_input[i] ) << std::endl;
			if( str_input[i] == bracket_types[bracket_index][0] ) {
				//std::cout << "found open brack " << bracket_index << " at pos " << i << std::endl;
				//bracket_depths[bracket_index] += 1;
				brack_stack.push_back( bracket_index );
				break;
			}
			else if( str_input[i] == bracket_types[bracket_index][1] ) {
				//std::cout << "found closed brack " << bracket_index << " at pos " << i << std::endl;
				if( brack_stack.back() != bracket_index ) {
					std::string ex_msg = "misformed brackets, expected '";
					ex_msg.append( to_string(bracket_types[bracket_index][1]) );
					ex_msg.append( "', received '" );
					ex_msg.append( to_string(str_input[i]) );
					ex_msg.append( "'" );

					throw InvalidInputException( ex_msg, i );
				}
				//bracket_depths[bracket_index] -= 1;

				brack_stack.pop_back();

				if( brack_stack.size() == 0 ) {
					return i;
				}
				break;
			}
		}
	}

	throw NoClosingBracketsException();
}



void remove_whitespace( std::string *str_input, int *input_pos ) {
	static const std::regex whitespace_regex( "^"
	                                          "(\\s*)"
	                                       );
	std::smatch result;

	bool found = std::regex_search( *str_input,
	                                result,
	                                whitespace_regex );

	if( found == false ) {
		return;
	}

	uint32_t whitespace_count = result.str(1).size();

	int substr_start = whitespace_count;
	int substr_end   = str_input->size();

	//int substr_len   = substr_end - substr_start;

	*str_input = str_input->substr( substr_start, substr_end );
	*input_pos += whitespace_count;
}

bool remove_brackets( std::string *str_input,
                      int         *input_pos,
                      std::string *bracket_contents,
                      int         *bracket_content_start_pos_out,
                      bool        *right_brack_present_out ) {
	
	static const std::regex bracket_regex( "^"
	                                       "(\\s*)"
	                                       "\\("
	                                       );
	std::smatch result;

	bool found = std::regex_search( *str_input,
	                                result,
	                                bracket_regex ); //,
	                                //std::regex_constants::basic );

	if( !found ) {
		//std::cout << "did not find raw val in s: \"" << str_input << "\"" << std::endl;
		return false;
	}

	int      match_len         = result.str(0).size();
	uint32_t open_bracket_pos  = result.str(1).size();
	uint32_t close_bracket_pos;
	*bracket_content_start_pos_out = *input_pos + match_len;

	try {
		close_bracket_pos = get_closing_brack_pos( *str_input );
		*right_brack_present_out = true;
	} catch(const NoClosingBracketsException &ex) {
		//std::cout << "no closing brackets found" << std::endl;
		// TODO append a warning or something here
		close_bracket_pos = str_input->size();
		*right_brack_present_out = false;
	}
	
	#if 0
	std::cout << std::endl;
	std::cout << "input str: " << *str_input << std::endl;
	std::cout << "           ";
	for( uint32_t i2=0; i2<30; i2++ ) { std::cout << (i2%10); }
	std::cout << std::endl;

	std::cout << "Found open bracket at" << open_bracket_pos << std::endl;
	std::cout << "Found close bracket at" << close_bracket_pos << std::endl;
	#endif

	int substr_start = open_bracket_pos + 1;
	int substr_end   = close_bracket_pos - 1;

	int substr_len   = substr_end - substr_start + 1;

	*bracket_contents = str_input->substr( substr_start,
	                                       substr_len ) ;

	//std::cout << "bracket_contents = \"" << *bracket_contents << "\"" << std::endl;


	//std::cout << "substr: \"" << *bracket_contents << "\"" << std::endl;
	

	if( close_bracket_pos < str_input->size() ) {
		int substr_start = close_bracket_pos+1;
		int substr_end   = str_input->size();
		int substr_len   = substr_end - substr_start;
		*str_input = str_input->substr( substr_start, substr_len );
		*input_pos += substr_start;
	} else {
		*input_pos += str_input->size();
		*str_input = "";
	}
	//std::cout << "str_input remaining: \"" << *str_input << "\"" << std::endl;
	return true;
}

std::string node_list_to_str( const std::list<Node*> &node_list ) {
	std::string output = "[ ";
	bool first = true;
	for( Node *n : (node_list) ) {
		if( first == false ) {
			output.append( ", " );
		}
		output.append( n->to_string() );
		first = false;
	}

	output.append( " ]" );

	return output;
}

bool debug_collapse = false;
//bool debug_collapse = true;



bool collapse( std::list<Node*> *node_list );

bool collapse_throws( std::list<Node*> *node_list,
                      std::list<Node*> &l_arg_list,
                      std::list<Node*> &r_arg_list) {
	if( node_list->size() <= 1 ) {
		return false;
	}
	//bool completed = false;


	std::unique_ptr<NodeOp> op1;
	
	bool found_first_op = false;

	if( debug_collapse ) {
		std::cout << "Called collapse with node_list: " << node_list_to_str(*node_list) << std::endl;
	}

	/* TODO I shouldn't be taking them out until I know that I'm going to actually use them-- I shouldn't
	 * simply put them back in. */

	/* First, find the left and right args of the first op.
	 * the right arg will extend until we find another op of equal or lesser precedence */
	while( node_list->empty() == false ) {
		Node *n = node_list->front();
		node_list->pop_front();
		if( found_first_op == false ) {
			if( n->get_node_type() != NODE_OP ||
				n->promote_to_op()->needs_args() == false ) {
				l_arg_list.push_back( n );
			} else {
				found_first_op = true;
				op1.reset(n->promote_to_op());
			}
		} else {
			if( n->get_node_type() == NODE_OP ) {
				NodeOp *op2 = n->promote_to_op();
				/* Will continue to push args to r_arg_list until we find an op that we know
				 * needs a left arg but is lower or equal precedence to us */
				if( op2->get_precedence() <= (*op1).get_precedence() &&
					( op2->needs_args() && get_l_arg_count(op2->get_op()) > 0 ) &&
					op2->is_left_associative() ) {
					node_list->push_front( n );
					break;
				}
			}
			r_arg_list.push_back( n );
		}
	}

	if( found_first_op == false ) {
		std::list<Node*>::reverse_iterator iter;
		for( iter=l_arg_list.rbegin(); iter!=l_arg_list.rend(); iter++ ) {
			node_list->push_front( *iter );
		}
		return false;
	}

	if( debug_collapse ) {
		std::cout << "first op is: " << (*op1).to_string() << std::endl;
		std::cout << "l_arg_list is: " << node_list_to_str(l_arg_list) << std::endl;
		std::cout << "r_arg_list is: " << node_list_to_str(r_arg_list) << std::endl;
	}

	arg_count_t op_l_arg_c = get_l_arg_count( (*op1).get_op() );

	if( op_l_arg_c == 0 && l_arg_list.size() != 0 ) {
		std::string msg = "op_l_arg_c is 0 and l_arg_list is ";
		msg.append( node_list_to_str( l_arg_list ) );
		throw BaseCalcException( msg );
		
	}

	while (collapse( &l_arg_list ) );

	if( op_l_arg_c != 0 && l_arg_list.size() != 1 ) {
		std::string msg = "could not collapse l_arg_list to one node: ";
		msg.append( node_list_to_str(l_arg_list) );
		throw BaseCalcException( msg );
	}

	while (collapse( &r_arg_list ));

	if( r_arg_list.size() != 1 ) {
		std::string msg = "could not collapse r_arg_list to one node: ";
		msg.append( node_list_to_str(r_arg_list) );
		throw BaseCalcException( msg );
	}

	if( op_l_arg_c != 0 ) {
		(*op1).add_child( l_arg_list.front() );
	}

	(*op1).add_child( r_arg_list.front() );

	node_list->push_front( op1.get() );
	op1.release();

	return true;
}

bool collapse( std::list<Node*> *node_list ) {
	std::list<Node*> l_arg_list;
	std::list<Node*> r_arg_list;
	try {
		return collapse_throws(node_list, l_arg_list, r_arg_list);
	} catch (...) {
		for ( Node* n : l_arg_list) {
			delete n;
		}
		for ( Node* n : r_arg_list) {
			delete n;
		}
		throw;
	} 
}

void calc_parse_throws(std::list<Node*> *node_list,
                       std::string str_input,
                       int *input_pos,
                       InputInfo *info_out,
                       const parse_params_s *params) {
	//std::list  <Node*>   node_list;

	if( debug_collapse ) {
		std::cout << "calling calc_parse_internal with str_input=\"" << str_input << "\"" << std::endl;
	}

	bool any_found = true;
	while( any_found ) {

		remove_whitespace( &str_input, input_pos );
		//std::cout << "searching \"" << str_input << "\" for vals/ops..." << std::endl;


		any_found = false;

		std::string bracket_contents;
		int bracket_start_pos = *input_pos;
		int bracket_content_start_pos = -1;
		bool right_brack_present;
		bool found_brack = remove_brackets( &str_input, input_pos, &bracket_contents, &bracket_content_start_pos, &right_brack_present );
		if( found_brack ) {
			// set cursor to brackets, but calc_parse_internal will overwrite it if
			// a better cursor node is found inside the brackets
			Node *old_cursor_node = info_out->cursor_node;
			Node * n = calc_parse_internal( bracket_contents, bracket_content_start_pos, info_out, params );
			if (params->parse_wip) {
				Node *arg = n;
				n = new NodeWipBrackets(arg, right_brack_present);
				// TODO if cursor is over *input_pos, set info_out->cursor_offset to -1?
				if (old_cursor_node == info_out->cursor_node) {
					handle_cursor_node(bracket_start_pos, *input_pos, n, info_out, params);
					if (params->cursor_pos == *input_pos) {
						info_out->cursor_offset = -1;
					}
				}
			}

			UnitInfoInputAry unit_info;
			bool found_unit_after_brack = parse_unit(&str_input, input_pos, &unit_info.units);
			if (found_unit_after_brack) {
				Node *apply_units = new NodeApplyUnits(n, unit_info);
				n = apply_units;
			}

			node_list->push_back( n );
			any_found = true;
		}

		Node *non_op_node;
		bool found_non_op = parse_non_op( &str_input, input_pos, &non_op_node, info_out, params );

		if( found_non_op ) {
			//std::cout << "found val \"" << val << "\", ";
			//std::cout << "rest of str: \"" << str_input << "\"" << std::endl;

			node_list->push_back(non_op_node);
			any_found = true;
		}

		std::string op_str;
		int prev_pos = *input_pos;
		bool found_op = parse_op_str( &str_input, input_pos, &op_str );

		if( found_op ) {
			//std::cout << "found op_char \"" << op_str << "\", ";
			//std::cout << "rest of str: \"" << str_input << "\"" << std::endl;
			any_found = true;

			bool prev_node_needs_no_args = node_list->size() > 0 &&
			                               ( node_list->back()->get_node_type() != NODE_OP
			                               //|| false ) ; //node_list->back()->promote_to_op()->needs_args() == false );
			                               || node_list->back()->promote_to_op()->needs_args() == false );


			//std::cout << "prev_node_needs_no_args = " << prev_node_needs_no_args << std::endl;
			//std::cout << "node_list->size() = " << node_list->size() << std::endl;

			//for( Node * n : node_list ) {
			//	std::cout << "n: " << n->to_string() << std::endl;
			//}
			//std::cout << "node_list.back()->get_node_type()  " << node_list.back()->get_node_type() << std::endl;
			NodeOp * n = op_str_to_node( op_str, prev_node_needs_no_args );
			//std::cout << "op n = " << n->to_string() << std::endl;


			/*
			std::cout << "debug: size: " << node_stack_op_only.size() << std::endl;
			if( node_stack_op_only.size() > 0 ) {
				std::int32_t prec1 = node_stack_op_only.back()->get_precedence();
				std::int32_t prec2 = n->get_precedence();
				std::cout << "debug:" << prec1 << std::endl;
				std::cout << "debug:" << prec2 << std::endl;
			}
			*/

			node_list->push_back( n );
			handle_cursor_node(prev_pos, *input_pos, n, info_out, params);
			if (params->cursor_pos == *input_pos) {
				info_out->cursor_offset = -1;
			}
		}

	}

	if( str_input.size() != 0 ) {
		std::string msg = "str_input not fully parsed: '";
		msg.append( str_input );
		msg.append( "'" );
		throw BaseCalcException( msg );
	}

	bool completed;
	do { 
		completed = collapse( node_list );
	} while( completed );

		//std::cout << "Node stack size: " << node_list.size() << std::endl;
		//int node_index = 0;
		//for( Node * n : node_list )
		//{
		//	std::cout << "\"" << n->to_string() << "\"" << std::endl;
		//}

	if( node_list->size() != 1 ) {
		std::string ex_msg = "node_list size is " + to_string( node_list->size() );
		throw BaseCalcException( ex_msg );
	}

}

/**
 * Checks if every node in the tree has enough args to evaluate
 */
bool check_nodes_have_args(Node *n) {
	if (dynamic_cast<NodeFunc*>(n) != nullptr) {
		NodeFunc* n_func = dynamic_cast<NodeFunc*>(n);
		for (Node *arg : n_func->args) {
			bool has_args = check_nodes_have_args(arg);
			if (!has_args) { return has_args; }
		}
		return true;
	} else if (dynamic_cast<NodeWipFuncCall*>(n) != nullptr) {
		NodeWipFuncCall *n_wip_func = dynamic_cast<NodeWipFuncCall*>(n);
		return check_nodes_have_args(n_wip_func->arg);
	} else if (dynamic_cast<NodeWipBrackets*>(n) != nullptr) {
		NodeWipBrackets *n_wip_brackets = dynamic_cast<NodeWipBrackets*>(n);
		return check_nodes_have_args(n_wip_brackets->arg);
	} else if (dynamic_cast<NodeValue*>(n)    != nullptr ||
	           dynamic_cast<NodeVar*>(n)      != nullptr ||
	           dynamic_cast<NodeWipToken*>(n) != nullptr) {
		return true;
	} else if (dynamic_cast<NodeOp*>(n) != nullptr) {
		NodeOp *n_op = dynamic_cast<NodeOp*>(n);
		if (n_op->needs_args()) {
			return false;
		}
		for (Node *child : n_op->children) {
			bool has_args = check_nodes_have_args(child);
			if (!has_args) { return has_args; }
		}
		return true;
	} else if (dynamic_cast<NodeApplyUnits*>(n) != nullptr) {
		NodeApplyUnits *n_apply_units = dynamic_cast<NodeApplyUnits*>(n);
		return n_apply_units->n != nullptr;
	} else {
		std::cerr << "Unhandled node type in " << __func__ << ": " << n << std::endl;
		throw BaseCalcException("Unhandled node type in check_nodes_have_args()");
		return true;
	}
}

Node * calc_parse_internal(std::string str_input, int start_pos, InputInfo *info_out, const parse_params_s *params) {
	std::list<Node*> node_list;
	try {
		calc_parse_throws(&node_list, str_input, &start_pos, info_out, params);
	} catch (...) {
		for (Node * n: node_list) {
			delete n;
		}
		throw;
	}
	return node_list.front();
}


// TODO rather than having two public APIs here, should just have
// one that returns a structure with information like "variable assignment" or something.

// public API
void calc_parse_no_sto(std::string str_input, int *input_pos, const parse_params_s *params, InputInfo *info_out) {
	std::list<Node*> node_list;
	try {
		calc_parse_throws(&node_list, str_input, input_pos, info_out, params);
		if (node_list.size() != 1) { throw InvalidInputException("node_list_size is " + to_string(node_list.size()), 0); }

		// This should never happen. Currently it's happening on "e^-", I'm not sure why.
		// it should fail in the parsing stage.
		// but better to error here than when it gets evaluated
		if (!check_nodes_have_args(node_list.front())) { throw InvalidInputException("not all nodes have enough arguments", 0); }
	} catch (...) {
		for (Node * n: node_list) {
			delete n;
		}
		throw;
	}
	info_out->n = node_list.front();
	return;
}

// public API
bool calc_check_for_sto(const std::string str_input, std::string *expression_out, std::string *name_out) {
	static const std::regex sto_regex( //"^"
	                                   "(.*)"
	                                   "->"
	                                   "\\s*"
	                                   "("
	                                       "[a-zA-Z][0-9a-zA-Z]*"
	                                       "(_[0-9a-zA-Z]+)?"
	                                   ")"
	                                   "\\s*"
	                                   );
	std::smatch result;

	bool found = std::regex_match( str_input,
	                               result,
	                               sto_regex );

	if( !found ) {
		return false;
	}

	*expression_out = result.str(1);
	*name_out = result.str(2);

	return true;
}

bool is_all_whitespace(const std::string &str_input) {
	for (char c : str_input) {
		if (c != ' ') {
			return false;
		}
	}
	return true;
}


bool calc_check_for_to_unit(const std::string &str_input,
                            std::string *expression_out,
                            std::vector<UnitInfoInput> *to_units) {
	static const std::regex to_unit_regex( //"^"
	                                      "(.*)"
	                                      "\\bto\\b"
	                                      "\\s*"
	                                      "("
	                                        "(?:"
	                                          "(?:"
	                                             // for non WIP, it should look like this
	                                             //"[a-zA-Z_]*[0-9a-zA-Z_]+"
	                                             // but for WIP, it looks like this:
	                                             "[a-zA-Z_]*[0-9a-zA-Z_]*"
	                                          ")"
	                                          "(?:"
	                                             "\\^(?:-?[0-9]+)"
	                                          ")?"
	                                          "\\s*"
	                                        ")+"
	                                      ")"
	                                      "\\s*$"
	                                      );
	std::smatch result;

	bool found = std::regex_match( str_input,
	                               result,
	                               to_unit_regex );

	if( !found ) {
		return false;
	}

	*expression_out = result.str(1);
	std::string to_units_str = result.str(2);


	if (!is_all_whitespace(to_units_str)) {
		int dummy = 0;
		bool found = parse_unit(&to_units_str, &dummy, to_units);
		if (!found) {
			throw InvalidInputException("could not parse unit desired for output", 0);
		}
	}

	return true;

}


// public API
InputInfo calc_parse( std::string str_input,
                      const parse_params_s *params,
                      const CalcData *calcData) {
	int input_pos = 0;
	InputInfo info;

	info.cursor_pos = params->cursor_pos;

	// TODO this is bad, just pass the units map? Or a const pointer to the units map?
#warning "replace this with a pointer to the units map or something"
	info.calcData = calcData;

	{
		std::string expression;
		std::string sto_var_name;
		info.has_sto = calc_check_for_sto(str_input, &expression, &sto_var_name);
		if (info.has_sto) {
			str_input = expression;
			info.sto_var_name = sto_var_name;
		}
	}

	{
		std::string expression;
		std::string result_unit_selection;
		std::vector<UnitInfoInput> to_units;
		info.has_to_unit = calc_check_for_to_unit(str_input, &expression, &to_units);
		if (info.has_to_unit) {
			str_input = expression;
			info.to_unit = to_units;
		}
	}

	calc_parse_no_sto(str_input, &input_pos, params, &info);

	// TODO only throw in debug 
	if (input_pos != str_input.size()) {
		char msg[256];
		snprintf(msg, sizeof(msg), "input_pos=%d, str_input.size=%lu", input_pos, str_input.size());
		delete info.n;
		throw BaseCalcException(msg);
	}
	return info;
}

bool cmd_parse(std::string str_input, std::string *cmd_out, std::string *args) {
		static const std::regex sto_regex( //"^"
	                                   ":"
		                               "([0-9A-Za-z_]+)"
		                               "\\s*"
		                               "(.*)"
		                               "\\s*"
	                                   );
	std::smatch result;

	bool found = std::regex_match( str_input,
	                               result,
	                               sto_regex );

	if (!found) {
		return false;
	}

	*cmd_out = result.str(1);
	*args    = result.str(2);
	return true;
}
