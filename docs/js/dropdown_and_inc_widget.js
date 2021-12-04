/**
 * Generic code for a set of 4 html elements:
 *   - up/down buttons
 *   - span to display current value (usually large)
 *   - dropdown (i.e. select element)
 * User can either press the buttons to increment/decrement the current choice,
 * or use the dropdown.
 *
 */

function update_display(ui_dropdown_and_inc, select_idx) {
	let html_val = ui_dropdown_and_inc.get_display_val(select_idx);
	console.log("setting display to val", html_val);
	ui_dropdown_and_inc.display.innerHTML = html_val;
}

function disable_display(ui_dropdown_and_inc) {
	ui_dropdown_and_inc.display.innerHTML = "&nbsp;";
	//ui_dropdown_and_inc.display.innerHTML = "";
}

function select_selection_made(ui_dropdown_and_inc, select_idx) {
	console.assert(typeof(select_idx) == "number");
	ui_dropdown_and_inc.select_idx = select_idx;
	update_display(ui_dropdown_and_inc, select_idx);
}

function btn_adjust(ui_dropdown_and_inc, inc) {
	let select_idx = ui_dropdown_and_inc.select_idx;
	select_idx += inc;
	if (select_idx < 0) { select_idx = 0; }
	if (select_idx >= ui_dropdown_and_inc.choice_count) { select_idx = ui_dropdown_and_inc.choice_count-1; }
	ui_dropdown_and_inc.select_idx = select_idx;

	let prefix_info = ui_dropdown_and_inc.get_display_val(select_idx);
	console.log(ui_dropdown_and_inc.name, " changed to idx=", select_idx, "info:", prefix_info);
	ui_dropdown_and_inc.select.selectedIndex = select_idx;
	update_display(ui_dropdown_and_inc, select_idx);
}

// public API
function dropdown_and_inc_set_enabled(ui_dropdown_and_inc, is_enabled) {
	// disable buttons/dropdown if this widget is disabled,
	// or if there is only one choice
	if (!is_enabled || ui_dropdown_and_inc.choice_count <= 1) {
		ui_dropdown_and_inc.btn_inc.style.display = "none";
		ui_dropdown_and_inc.btn_dec.style.display = "none";
		ui_dropdown_and_inc.select.style.display  = "none";
	} else {
		ui_dropdown_and_inc.btn_inc.style.display = "";
		ui_dropdown_and_inc.btn_dec.style.display = "";
		ui_dropdown_and_inc.select.style.display  = "";
	}

	if (!is_enabled) {
		disable_display(ui_dropdown_and_inc);
	} else {
		update_display(ui_dropdown_and_inc, ui_dropdown_and_inc.select_idx);
	}
}

// public API, builds HTML elements for the dropdown, removing any existing
function dropdown_set_select_options(ui_dropdown_and_inc, get_option_text) {
	let select = ui_dropdown_and_inc.select;
	while (select.firstChild != null) {
		select.removeChild(select.firstChild);
	}

	for (let i=0; i<ui_dropdown_and_inc.choice_count; i++) {
		let txt = get_option_text(i);
		let option = document.createElement("option");
		let txtNode = document.createTextNode(txt);
		option.value = i;
		option.appendChild(txtNode);
		select.appendChild(option);
	}
}

// public API
function dropdown_and_inc_reset_sel(ui_dropdown_and_inc) {
	ui_dropdown_and_inc.select_idx = ui_dropdown_and_inc.init_select_idx;
	ui_dropdown_and_inc.select.selectedIndex = ui_dropdown_and_inc.init_select_idx;
	update_display(ui_dropdown_and_inc, ui_dropdown_and_inc.select_idx);
}

// public API
function dropdown_get_selected_val(ui_dropdown_and_inc) {
	return ui_dropdown_and_inc.get_output_val();
}


// argument:
// ui_dropdown_and_inc = {
//    choice_count: number of options in select
//    select:  HTML select element
//    btn_inc: HTML button element to increment
//    btn_dec: HTML button element to decrement
//    display: HTML span (or similar) element to set innerHTML with value returned by get_display_val
//    get_display_val: callback(select_idx), returning value to set as innerHTML of `select` element
//    init_select_idx: initial index of selection
// }
function init_dropdown_and_inc_widget(ui_dropdown_and_inc) {
	console.debug("init_dropdown_and_inc_widget", ui_dropdown_and_inc.name, "called with", ui_dropdown_and_inc);
	let select_change_callback = function (e) {
		let select_idx = Number(e.target.value); // note e.target.value is a string, need to convert to an int. (ffs fucking god damn javascript)
		select_selection_made(ui_dropdown_and_inc, select_idx);
	};

	ui_dropdown_and_inc.listeners = [
		// note that the dropdown options are from largest to greatest, because this order is
		// more natural for humans to read.
		// But that means pressing "up" will decrease our position in the list, so "inc" is -1
		{ elem: ui_dropdown_and_inc.btn_inc, evt: 'click',  callback: function (e) { btn_adjust(ui_dropdown_and_inc, -1) } },
		{ elem: ui_dropdown_and_inc.btn_dec, evt: 'click',  callback: function (e) { btn_adjust(ui_dropdown_and_inc,  1) } },
		{ elem: ui_dropdown_and_inc.select,  evt: 'change', callback: select_change_callback },
	];

	for (let listener_info of ui_dropdown_and_inc.listeners) {
		listener_info.elem.addEventListener(listener_info.evt, listener_info.callback);
	}

	ui_dropdown_and_inc.select_idx = ui_dropdown_and_inc.init_select_idx;
	ui_dropdown_and_inc.select.selectedIndex = ui_dropdown_and_inc.init_select_idx;

	// maybe just update display instead of setting enabled?
	dropdown_and_inc_set_enabled(ui_dropdown_and_inc, true);

}

function destruct_dropdown_and_inc_widget(ui_dropdown_and_inc) {
	for (let listener_info of ui_dropdown_and_inc.listeners) {
		listener_info.elem.removeEventListener(listener_info.evt, listener_info.callback);
	}
}
