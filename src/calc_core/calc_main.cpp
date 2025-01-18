
#include<string.h>
#include<iostream>
#include<memory>

#include "calc_parse.h"
#include "node_to_latex.h"

void print_help(void) {
	std::cout << "Commands:" << std::endl;
	std::cout << "   :exit         exits the program" << std::endl;
	std::cout << "   :polar        shows complex number output in polar form" << std::endl;
	std::cout << "   :rect         shows complex number output in rectangular form" << std::endl;
	std::cout << "   :parse        parses but does not evaluate input" << std::endl;
	std::cout << "   :test_parse   runs some test cases" << std::endl;
	std::cout << "   :alloc        prints allocated nodes (to test for memory leaks)" << std::endl;
	std::cout << "   :to_latex     converts input to LaTeX instead of evaluating" << std::endl;
	std::cout << "   :vars         prints all current variables and their values" << std::endl;
	std::cout << "   :cursor [off|<pos>] turns off cursor or sets cursor posistion (For LaTeX only)" << std::endl;
	std::cout << "   :echo [on|off]      on prints every received line" << std::endl;
	std::cout << "   :print_eof [on|off] on prints when EOF is received" << std::endl;
	std::cout << "   :del_vars     deletes all vars" << std::endl;
}

int main(void) {

	bool parse_only = false;
	bool to_latex = false;
	
	bool echo      = false;
	bool print_eof = false;

	bool show_cursor = false;
	int  cursor_pos = -1;
	struct calc_fmt_params params = get_default_params();

	bool user_exit = false;

	CalcData calcData;
	
	std::string str_input;

	try {
		calcData.units = get_units();
	} catch (const std::exception& ex) {
		std::cerr << "when generating units, recvd ex: " << ex.what() << std::endl;
		return -1;
	}

	while(std::getline(std::cin, str_input)) {

		if (echo) {
			std::cerr << "Received line \"" << str_input << "\"" << std::endl;
		}

		if (str_input == "help") {
			print_help();
			continue;
		}

		std::string cmd, args;
		if (cmd_parse(str_input, &cmd, &args)) {
			if (cmd == "help") {
				print_help();
			} else if (cmd == "exit") {
				user_exit = true;
				break;
			} else if (cmd == "parse") {
				parse_only = !parse_only;
				std::cout << "Parse only is now: " << parse_only << std::endl;
			} else if (cmd == "debug_collapse") {
				debug_collapse = !debug_collapse;
				std::cout << "Debug collapse is now: " << debug_collapse << std::endl;
			} else if (cmd == "alloc") {
				Node::print_allocated_nodes(std::cout);
			} else if (cmd == "to_latex") {
				to_latex = !to_latex;
				std::cout << "Output to latex is now: " << to_latex << std::endl;
			} else if (cmd == "vars") {
				calcData.print_vars();
			} else if (cmd == "cursor") {
				if (args == "off") {
					show_cursor = false;
				} else {
					try {
						cursor_pos  = std::stoi(args);
						show_cursor = true;
					} catch (std::invalid_argument &ex) {
						std::cerr << ":cursor requires integer argument "
						          << "or \"off\", received \"" << args << "\"" << std::endl; 
						continue;
					}
				}
			
				if (show_cursor) {
					std::cout << "Cursor is now shown at pos " << cursor_pos << std::endl;
				} else {
					std::cout << "Cursor is now disabled" << std::endl;
				}
			} else if (cmd == "degree" || cmd == "deg") {
				calcData.degree = true;
				std::cout << "Trig input and calc output will now be in degrees" << std::endl;
			} else if (cmd == "rad" || cmd == "radian") {
				calcData.degree = false;
				std::cout << "Trig input and calc output will now be in radians" << std::endl;
			} else if (cmd == "polar") {
				calcData.polar = true;
				std::cout << "Calc output will now use polar notation for complex numbers" << std::endl;
			} else if (cmd == "rect") {
				calcData.polar = false;
				std::cout << "Calc output will now use rect notation for complex numbers" << std::endl;
			} else if (cmd == "unit") {
				std::string unit_name = args;
				if (calcData.units.find(unit_name) == calcData.units.end()) {
					std::cout << "Unit \"" << unit_name << "\" not found" << std::endl;
				} else {
					UnitInfoParsed unit = calcData.units.at(unit_name);
					std::cout << "Unit \"" << unit_name << "\" = " << unit.to_string() << std::endl;
				}
			} else if (cmd == "del_vars") {
				calcData.delete_vars();
			} else if (cmd == "echo") {
				if (args == "off") {
					echo = false;
				} else if (args == "on") {
					echo = true;
				} else {
					std::cerr << "Unexpected arg to echo: \"" << args << "\"" << std::endl;
				}
			} else if (cmd == "print_eof") {
				if (args == "off") {
					print_eof = false;
				} else if (args == "on") {
					print_eof = true;
				} else {
					std::cerr << "Unexpected arg to print_eof: \"" << args << "\"" << std::endl;
				}
			} else {
					std::cout << "unexpected command \"" << cmd << "\"" << std::endl;
			}
			continue;
		}

		std::unique_ptr<Node> n;

		parse_params_s parse_params;
		memset(&parse_params, 0, sizeof(parse_params));

		if (show_cursor) {
			parse_params.cursor_pos = cursor_pos;
			parse_params.parse_wip  = true;
		} else {
			parse_params.cursor_pos = 0;
			parse_params.parse_wip  = false;
		}

		InputInfo input_info;
		try { 
			input_info = calc_parse( str_input, &parse_params, &calcData );
			n.reset(input_info.n);
		} catch( const BaseCalcException &e ) {
			std::cout << "BaseCalcException when parsing: " << e.msg << std::endl;
			continue;
		}

		if( parse_only ) {
			std::cout << *n << std::endl;
			continue;
		} else if(to_latex) {
			try {
				std::string latex = node_to_latex(&input_info);
				std::cout << latex << std::endl;
			} catch (const BaseCalcException &e) {
				std::cout << "BaseCalcException when evaluating: " << e.msg << std::endl;
			}
			continue;
		}

		OutputInfo output;
		try {
			//val = (*n).eval(&calcData);
			output = input_info.eval(&calcData);
			const unit_t *desired_unit = nullptr;
			if (output.output_unit_str.size() > 0) {
				desired_unit = &output.unit;
			}
			std::cout << val_to_string(&output.val, params, desired_unit);
 			if (output.output_unit_str.size() > 0) {
				std::cout << " " << unit_info_input_vec_to_string(&output.output_unit_str);
			}
			std::cout << std::endl;
		} catch( const BaseCalcException &e ) {
			std::cout << "BaseCalcException when evaluating: " << e.msg << std::endl;
			continue;
		}
	}

	if (user_exit) {
		std::cout << "Exiting" << std::endl;
		return 0;
	} else if (std::cin.bad()) {
		std::cout << "I/O error" << std::endl;
		return -1;
	} else if(std::cin.eof()) {
		if (print_eof) {
			std::cout << "reached EOF" << std::endl;
		}
		return 0;
	} else {
		std::cout << "unexpected format error" << std::endl;
		return -2;
	}
}
