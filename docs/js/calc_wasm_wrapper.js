const MAX_VAL_OUTPUT_SIZE   = 4096;
const MAX_LATEX_OUTPUT_SIZE = 65536;
const MAX_CALCDATA_STR_SIZE = 65536;
const MAX_UNIT_INFO_STR_SIZE = 65536;
const MAX_RECENTLY_USED_UNITS_STR_SIZE = 65536;

function alexcalc_init() {
	let rc = Module.ccall('alexcalc_init',
	                      'number',
	                      [],
	                      []);
	if (rc != 0) {
		throw 'alexcalc_init returned' + rc;
	}
}

function alexcalc_info_func() {
	let rc = Module.ccall('alexcalc_info_func',
	                      'string',
	                      [],
	                      []);
	return rc;
}

function alexcalc_new_calcdata() {
	let ptr = Module.ccall('alexcalc_new_calcdata',
	                      'number',
	                      [],
	                      []);
	return ptr;
}

function alexcalc(input_text, calcdata) {
	//let json_str = Module.ccall('alexcalc_json_str_output', 'string', ['string'], [input_text]);
	//let output = JSON.parse(json_str);
	//return output;
	console.assert(typeof input_text == 'string');

	let output_buff = _malloc(MAX_VAL_OUTPUT_SIZE);
	let output = "error";
	try {
		let rc = Module.ccall('alexcalc_json_str_output',
		                      'number',
		                      ['string', 'number', 'number', 'number'],
		                      [input_text, calcdata, output_buff, MAX_VAL_OUTPUT_SIZE]);
		if (rc == 0) {
			output = UTF8ToString(output_buff, MAX_VAL_OUTPUT_SIZE);
		} else {
			throw "alexcalc_json_str_output returned " + rc;
		}
	} finally {
		_free(output_buff);
	}

	console.debug("received calc output: ", output);
	let output_json = JSON.parse(output);
	return output_json;
}

function alexcalc_to_latex(input_text, calcdata, cursor_pos, parse_wip) {
	let start_time = performance.now();
	console.assert(typeof input_text == 'string');
	console.assert(typeof parse_wip == 'boolean')
	let output_buff = _malloc(MAX_LATEX_OUTPUT_SIZE);
	let output = "error";
	try {
		let rc = Module.ccall('alexcalc_to_latex',
		                      'number',
		                      ['string', 'number', 'number', 'number', 'number', 'boolean'],
		                      [input_text, calcdata, output_buff, MAX_LATEX_OUTPUT_SIZE, cursor_pos, parse_wip]);
		if (rc == 0) {
			output = UTF8ToString(output_buff, MAX_LATEX_OUTPUT_SIZE);
		} else {
			throw "alexcalc_to_latex returned " + rc;
		}
	//} catch (ex) {
	//	console.error("error thrown", ex);
	//	output = "syntax error";
	} finally {
		_free(output_buff);
	}
	let run_time = performance.now() - start_time;
	console.debug("Converted input ", input_text, " to latex output ", output, " in ", run_time);
	return output;
}

function alexcalc_calcdata_to_json(calcdata_ptr) {
	let output_str = "error";
	let output_buff = _malloc(MAX_CALCDATA_STR_SIZE);
	try {
		let rc = Module.ccall('alexcalc_calcdata_to_json',
		                      'number',
		                      ['number', 'number', 'number'],
		                      [calcdata_ptr, output_buff, MAX_CALCDATA_STR_SIZE]);
		if (rc == 0) {
			output = UTF8ToString(output_buff, MAX_CALCDATA_STR_SIZE);
		} else {
			throw "alexcalc_calcdata_to_json returned " + rc;
		}
	} finally {
		_free(output_buff);
	}
	return JSON.parse(output);
}

function alexcalc_data_state_set(calcdata_ptr, state) {
	try {
		let rc = Module.ccall('alexcalc_data_state_set',
		                      'number',
		                      ['number', 'number', 'number'],
		                      [calcdata_ptr, state.polar, state.degree]);
		if (rc == 0) {
		} else {
			throw "alexcalc_data_state_set returned " + rc;
		}
	} finally {
	}
}

function alexcalc_get_unit_info_json() {
	let output_str = "error";
	const size = MAX_UNIT_INFO_STR_SIZE;
	let output_buff = _malloc(size);
	try {
		let rc = Module.ccall('alexcalc_get_unit_info_json',
		                      'number',
		                      ['number', 'number'],
		                      [output_buff, size]);
		if (rc == 0) {
			output = UTF8ToString(output_buff, size);
		} else {
			throw "alexcalc_get_unit_info_json returned " + rc;
		}
	} finally {
		_free(output_buff);
	}
	return JSON.parse(output);

}

function alexcalc_get_recently_used_units_json(calcdata_ptr) {
	let output_str = "error";
	const size = MAX_RECENTLY_USED_UNITS_STR_SIZE;
	let output_buff = _malloc(size);
	try {
		let rc = Module.ccall("alexcalc_get_recently_used_units_json",
		                      'number',
		                      ['number', 'number', 'number'],
		                      [calcdata_ptr, output_buff, size]);
		if (rc == 0) {
			output = UTF8ToString(output_buff, size);
		} else {
			throw "alexcalc_get_recently_used_units_json returned " + rc;
		}
	} finally {
		_free(output_buff);
	}
	return JSON.parse(output);
}

function alexcalc_add_recently_used_unit(calcdata_ptr, unit_str) {
	let rc = Module.ccall("alexcalc_add_recently_used_unit",
	                      'number',
	                      ['number', 'string', 'number'],
	                      [calcdata_ptr, unit_str, unit_str.length]);
	return rc;

}
