//import { alexcalc_to_latex } from "js/calc_wasm_wrapper.js";
//importScripts('calc_wasm.js', 'calc_wasm_wrapper.js');


let calcdata_ptr = null;

// TODO only do this after init_calc is called?
var Module = {
	onRuntimeInitialized: function () {
		console.log("wasm initialized");
		// `ui` isn't defined at this point
		//ui.on_wasm_loaded();
		postMessage({msg_type: "calc_loaded"});
		alexcalc_init();
		calcdata_ptr = alexcalc_new_calcdata();
		console.log( "alexcalc_get_unit_info_json: ", alexcalc_get_unit_info_json());
		console.log( "alexcalc_info_func: ", alexcalc_info_func());
	},
};

importScripts('calc_wasm.js', 'calc_wasm_wrapper.js');
//importScripts('calc_wasm_wrapper.js');

/*
function init_wasm(module) {
	var config = {
		env: {
			memoryBase: 0,
			tableBase:  0,
			memory: new WebAssembly.Memory({initial: 256}),
			table: new WebAssembly.Table({initial: 0, element: 'anyfunc'})
		},
	};
	WebAssembly.instantiate(module, config);
	console.log("web assembly loaded?", module);
}

WebAssembly.compileStreaming(fetch('calc_wasm.wasm'))
	.then(module => {
		init_wasm(module);
	});
*/

/*
fetch('module.wasm').then(response =>
  response.arrayBuffer()
).then(bytes =>
  WebAssembly.instantiate(bytes, importObject)
).then(results => {
  // Do something with the results!
});
*/

/*
WebAssembly.instantiateStreaming(fetch('myModule.wasm'), importObject)
.then(obj => {
  // Call an exported function:
  obj.instance.exports.exported_func();

  // or access the buffer contents of an exported memory:
  var i32 = new Uint32Array(obj.instance.exports.memory.buffer);

  // or access the elements of an exported table:
  var table = obj.instance.exports.table;
  console.log(table.get(0)());
})
*/

function send_recent_units_update(units) {
	let msg = {
		msg_type: "recently_used_units_update",
		units: units,
	}
	postMessage(msg);
}

onmessage = function(e) {
	console.debug("recv msg ", e.data);
	let data = e.data;

	let output_data = null;
	if (data.msg_type == 'to_latex') {

		let latex;
		let error = null;
		try {
			latex = alexcalc_to_latex(data.raw_input, calcdata_ptr, data.cursor_pos, data.parse_wip);
		} catch(e) {
			latex="\\text{error}";
			//error = e;
			error = { message: e.message, fileName: e.fileName, lineNumber: e.lineNumber};
			console.error(e);
		}
		output_data = {
			msg_type:  data.msg_type,
			raw_input: data.raw_input,
			latex:     latex,
			error: error
		};
	} else if (data.msg_type == "calc") {
		const parse_wip = false;
		const cursor_pos = -1;

		let calc_output = null;
		let latex = "\\text{error}";
		let error = null;
		try {
			calc_output = alexcalc(data.raw_input, calcdata_ptr);
			// TODO should output latex from alexcalc too, since node is already parsed
			latex = alexcalc_to_latex(data.raw_input, calcdata_ptr, cursor_pos, parse_wip);
		} catch(e) {
			//error = e;
			error = { message: e.message, fileName: e.fileName, lineNumber: e.lineNumber};
			console.error(e);
		}
		let calcdata_obj = alexcalc_calcdata_to_json(calcdata_ptr);
		let units_info   = alexcalc_get_recently_used_units_json(calcdata_ptr);
		send_recent_units_update(units_info.units);

		output_data = {
			msg_type:     data.msg_type,
			raw_input:    data.raw_input,
			latex:        latex,
			calc_output:  calc_output,
			calcdata:     calcdata_obj,
			callback_key: data.callback_key,
			error: error
		};
	} else if (data.msg_type == "calc_state_update") {
		console.debug("updating calcdata with states: ", data.states);
		alexcalc_data_state_set(calcdata_ptr, data.states);
		output_data = {
			msg_type: data.msg_type,
			rc: 0,
		};
	} else if (data.msg_type == "unit_referenced") {
		let unit_str = data.unit_str;
		alexcalc_add_recently_used_unit(calcdata_ptr, unit_str);
		let units_info   = alexcalc_get_recently_used_units_json(calcdata_ptr);
		send_recent_units_update(units_info.units);
	} else if (data.msg_type == "get_info") {
		const info_str = alexcalc_info_func();
		output_data = {
			msg_type: data.msg_type,
			callback_key: data.callback_key,
			info:     info_str,
		};
	} else {
		console.error("unhandled msg_type ", data.msg_type);
	}
	console.debug("calc_worker sending ", output_data);

	try {
		postMessage(output_data);
	} catch (e) {
		console.error("Error when calling postMessage from worker", e);
		postMessage({msg_type: data.msg_type, error: e.message});
	}
}
