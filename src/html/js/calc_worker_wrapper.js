const ui = window.alexcalc_ui;

if (!ui) {
	throw new Error("ui is undefined");
}

// I'm not sure how to design this yet.
// either:
//    * simply define a "on tex ready" callback, and maybe a "on tex + val ready" callback, or
//    * each call generates a promise (or null if busy?)? Can then use the same promise hashmap
//      to store both to_latex and to_latex_and_calc callbacks?

const update_display_worker = new Worker("js/calc_worker.js");
const _calc_worker_state = {
	latest_tex: null,
	busy:       false,
	//promises:   {},
	on_tex_ready: null,
	//on_calc_output_ready: null,
	callbacks: new Map(),
};

function set_on_tex_ready(callback) {
	_calc_worker_state.on_tex_ready = callback;
}

//function set_on_calc_output_ready(callback) {
//	_calc_worker_state.on_calc_output_ready = callback;
//}

function to_latex_async(raw_input, cursor_pos, parse_wip) {
	//tex = ui.to_latex(raw_input, true);
	// TODO would need to serialize everything through this
	// TODO need to queue up a message if it's busy,
	// and delete any if a new message is pushed to the queue
	// Only the most recent input should be worked on
	_calc_worker_state.latest_tex = raw_input;
	
	//let key = performance.now() + "," + Math.random();
	if (_calc_worker_state.busy) {
		return false;
	} else {
		_calc_worker_state.busy = true;
		update_display_worker.postMessage({msg_type:   "to_latex",
		                                   raw_input:  raw_input,
		                                   cursor_pos: cursor_pos,
		                                   parse_wip:  true});
		return true;
	}
}

function alexcalc_async(raw_input, callback) {
	let callback_key = Date.now() + ", " + Math.random();
	_calc_worker_state.callbacks[callback_key] = callback;
	update_display_worker.postMessage({msg_type: "calc", raw_input: raw_input, callback_key: callback_key, parse_wip: false });
}

function alexcalc_set_states(states) {
	update_display_worker.postMessage({msg_type:"calc_state_update", states: states});
}

function alexcalc_unit_referenced(unit_str) {
	update_display_worker.postMessage({msg_type:"unit_referenced", unit_str: unit_str});
}

function alexcalc_async_get_info(callback) {
	let callback_key = Date.now() + ", " + Math.random();
	_calc_worker_state.callbacks[callback_key] = callback;
	update_display_worker.postMessage({msg_type: "get_info", callback_key: callback_key });
}

function alexcalc_get_info_promise() {
	return new Promise((resolve) => {
		alexcalc_async_get_info(resolve);
	});
}

update_display_worker.onmessage = function(e) {
	console.debug("recvd ", e);
	let data = e.data;

	if (data == null) {
		return;
	}
	
	if (data.msg_type == "calc_loaded") {
		ui.on_wasm_load();
	} else if (data.msg_type == "to_latex") {
		console.debug("converted \"" + data.raw_input + "\" to tex: \"" + data.tex + "\""); 
		if (_calc_worker_state.on_tex_ready != null) {
			_calc_worker_state.on_tex_ready(data.raw_input, data.latex);
		}
		if (_calc_worker_state.latest_tex != data.raw_input) {
			update_display_worker.postMessage({msg_type: "to_latex", raw_input: _calc_worker_state.latest_tex, parse_wip: true});
		}
		_calc_worker_state.busy = false;
	} else if (data.msg_type == "calc") {
		let callback = _calc_worker_state.callbacks[data.callback_key];
		delete _calc_worker_state.callbacks[data.callback_key];
		if (callback != null) {
			callback(data.raw_input, data.latex, data.calc_output, data.calcdata);
		}
		//if (_calc_worker_state.on_calc_output_ready != null) {
		//	_calc_worker_state.on_calc_output_ready(data.raw_input, data.latex, data.calc_output);
		//}
	} else if (data.msg_type == "calc_state_update") {
		// do nothing
	} else if (data.msg_type == "recently_used_units_update") {
		console.debug("Updating recently used units: ", data.units);
		update_recently_used_units(ui.unit_sel, data.units);
	} else if (data.msg_type == "get_info") {
		let callback = _calc_worker_state.callbacks[data.callback_key];
		delete _calc_worker_state.callbacks[data.callback_key];
		if (callback != null) {
			callback(data.info);
		}
	} else {
		console.error("unexpected msg_type: ", data.msg_type, data );
	}
}
