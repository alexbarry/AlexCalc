// TODO:
//  * initialize colours to whatever they are currently set to (based on prev colour scheme?)
//  * preserve custom colours in local storage
//  * print all hex codes somewhere, allowing for easy copy/pasting
//  * button to clear custom colours
//  * maybe separate colour scheme for "custom", affect that one instead
//  * button to hide the custom colour thing, only show it when user switches to custom colours
//  
//
import {CalcUi} from './calc_types.js';

declare const ui: CalcUi;
declare function set_theme(ui: CalcUi, theme: string): void;

let g_callback_switch_to_custom_colours: (() => void) | undefined = undefined;

const COLOUR_SEL_INFO = [
	{ label: 'main',      colour_sel_id: "color_sel_bg_main",       selector_text: "body.custom-colours div.main" },
	{ label: 'output',    colour_sel_id: "color_sel_output_txt",    selector_text: "body.custom-colours .output_txt_cls" },
	{ label: 'important', colour_sel_id: "color_sel_btn_important", selector_text: "body.custom-colours button.btn_important" },
	{ label: 'meta',      colour_sel_id: "color_sel_btn_meta",      selector_text: "body.custom-colours button.meta"          },
	{ label: 'var',       colour_sel_id: "color_sel_btn_var",       selector_text: "body.custom-colours button.var"           },
	{ label: 'func',      colour_sel_id: "color_sel_btn_func",      selector_text: "body.custom-colours button.func"          },
	{ label: 'op',        colour_sel_id: "color_sel_btn_op",        selector_text: "body.custom-colours button.op"            },
	{ label: 'specop',    colour_sel_id: "color_sel_btn_specop",    selector_text: "body.custom-colours button.specop"        },
	{ label: 'num',       colour_sel_id: "color_sel_btn_num",       selector_text: "body.custom-colours button.num"           },
];

const has_fg = new Set([
	'important',
	'meta',
	'var',
	'func',
	'op',
	'specop',
	'num',
]);

const example_elems: {[key: string]: string} = {
	'main':       'div.main',
	'output':     '.output_txt_cls',
	'important':  '#btn_enter',
	'meta':       '#btn_alt',
	'var':        '#btn_var1',
	'func':       '#btn_sin',
	'op':         '#btn_add',
	'specop':     '#btn_par_l',
	'num':        '#btn_1',
};

const output_elem = document.getElementById("custom_colour_output");

const output_colours: Map<string, string> = new Map();

function update_output_elem() {
	if (!output_elem) { throw new Error(); }
	let text = '';
	for (const {label} of COLOUR_SEL_INFO) {
		const val = output_colours.get(label);
		if (text) { text += '\n'; }
		const label_aligned = padStart(label, 9, ' ');
		text += `${label_aligned}: ${val}`;
		if (has_fg.has(label)) {
			const label2 = label + "_fg";
			const val2 = output_colours.get(label2);
			text += `, ${val2}`;
		}
	}
	output_elem.innerText = text;
}

function get_css_rule(selectorText: string): CSSStyleRule | undefined {
	for (const sheet of Array.from(document.styleSheets)) {
		for (let rule of sheet.cssRules || sheet.rules) {
			//console.log("checking rule", rule);
			if (!(rule instanceof CSSStyleRule)) {
				continue;
			}
			if (rule.selectorText == selectorText) {
				return rule;
			}
		}
	}	
	return undefined;
}

function padStart(val: string, n: number, pad: string): string {
	while (val.length < n) {
		val = pad + val;
	}
	return val;
}

function rgba_to_hex(rgb_str: string): string {
	const result = rgb_str.match(/^rgb\((\d+),\s*(\d+),\s*(\d+)\)$/);
	if (result) {
		const r = padStart(parseInt(result[1], 10).toString(16), 2, '0');
		const g = padStart(parseInt(result[2], 10).toString(16), 2, '0');
		const b = padStart(parseInt(result[3], 10).toString(16), 2, '0');
		return `#${r}${g}${b}`;
	}
	const result2 = rgb_str.match(/^rgba\((\d+),\s*(\d+),\s*(\d+),\s*(\d+\.?\d*)\)$/);
	if (result2) {
		const r = padStart(parseInt(result2[1], 10).toString(16), 2, '0');
		const g = padStart(parseInt(result2[2], 10).toString(16), 2, '0');
		const b = padStart(parseInt(result2[3], 10).toString(16), 2, '0');
		const a = padStart(parseInt(result2[4], 10).toString(16), 2, '0');
		return `#${r}${g}${b}`;
	}

	return rgb_str;
}

function load_existing_colours() {
	for (const {label, colour_sel_id} of COLOUR_SEL_INFO) {
		const colour_sel = document.getElementById(colour_sel_id) as HTMLInputElement;
		if (!colour_sel) {
			throw Error(`could not find element with id "${colour_sel_id}"`);
		}
	
		const example_elem_selector = example_elems[label];
		const example_elem = document.querySelector(example_elem_selector);
		if (!example_elem) {
			console.error("Could not find element matching selector", example_elem_selector);
			continue;
		}
		const computedStyle = window.getComputedStyle(example_elem);
		const existing_bg = computedStyle.backgroundColor;
		colour_sel.value = rgba_to_hex(existing_bg);
		console.log(`Loading colour ${existing_bg} for ${label}, using ${colour_sel.value}`);

		if (has_fg.has(label)) {
			const colour_fg = rgba_to_hex(computedStyle.color);
			const colour_sel_fg_id = colour_sel_id + "_fg";
			const colour_sel_fg = document.getElementById(colour_sel_fg_id) as HTMLInputElement;
			if (!colour_sel_fg) {
				throw Error(`could not find element with id "${colour_sel_fg_id}`);
			}
			colour_sel_fg.value = colour_fg;
		}
	}
}

function apply_colours() {
	for (const info of COLOUR_SEL_INFO) {
		const {label, colour_sel_id, selector_text} = info;
		const colour_sel = document.getElementById(colour_sel_id) as HTMLInputElement;
		if (!colour_sel) {
			throw Error(`could not find element with id "${colour_sel_id}"`);
		}
		const css_rule = get_css_rule(selector_text);
		if (!css_rule) {
			console.error("could not find CSS rule for selector_text", selector_text);
			continue;
		}
		css_rule.style.setProperty('background-color', colour_sel.value);
		output_colours.set(label, colour_sel.value);

		if (has_fg.has(label)) {
			const label_fg = label + "_fg";
			const colour_sel_fg = document.getElementById(colour_sel_id + "_fg") as HTMLInputElement;

			css_rule.style.setProperty('color', colour_sel_fg.value);
			output_colours.set(label_fg, colour_sel_fg.value);
		}
	}
	update_output_elem();
}

for (const info of COLOUR_SEL_INFO) {
	const {label, colour_sel_id, selector_text} = info;
	const colour_sel = document.getElementById(colour_sel_id) as HTMLInputElement;
	if (!colour_sel) {
		throw Error(`could not find element with id "${colour_sel_id}"`);
	}
	const css_rule = get_css_rule(selector_text);
	if (!css_rule) {
		console.error("could not find CSS rule for selector_text", selector_text);
		continue;
	}
	const update_handler = (e: Event) => {
		const target = e.target as HTMLInputElement;
		console.log(colour_sel_id, "colour is now", target.value);
		css_rule.style.setProperty('background-color', target.value);
		output_colours.set(label, target.value);
		update_output_elem();

		if (g_callback_switch_to_custom_colours) {
			g_callback_switch_to_custom_colours();
		}
		write_colours_to_local_storage();
	};
	colour_sel.addEventListener('input', update_handler);
	colour_sel.addEventListener('submit', update_handler);

	if (has_fg.has(label)) {
		const label_fg = label + "_fg";
		const colour_sel_fg = document.getElementById(colour_sel_id + "_fg")! as HTMLInputElement;
		const update_handler_fg = (e: Event) => {
			const target = e.target as HTMLInputElement;
			console.log(colour_sel_id + "_fg", "colour is now", target.value);
			css_rule.style.setProperty('color', target.value);
			output_colours.set(label_fg, target.value);
			update_output_elem();
	
			if (g_callback_switch_to_custom_colours) {
				g_callback_switch_to_custom_colours();
			}
			write_colours_to_local_storage();
		};
		colour_sel_fg.addEventListener('input', update_handler_fg);
		colour_sel_fg.addEventListener('submit', update_handler_fg);
	}
}

function load_colours_from_local_storage() {
	for (const {label, colour_sel_id} of COLOUR_SEL_INFO) {
		const colour_sel = document.getElementById(colour_sel_id)! as HTMLInputElement;
		const stored_val = window.localStorage[label];
		if (stored_val) {
			colour_sel.value = stored_val;
		}

		if (has_fg.has(label)) {
			const label_fg = label + "_fg";
			const colour_sel_fg = document.getElementById(colour_sel_id + "_fg")! as HTMLInputElement;
			const stored_val = window.localStorage[label_fg];
			if (stored_val) {
				colour_sel_fg.value = stored_val;
			}
		}
	}
}

function write_colours_to_local_storage() {
	for (const {label, colour_sel_id} of COLOUR_SEL_INFO) {
		const colour_sel = document.getElementById(colour_sel_id)! as HTMLInputElement;
		window.localStorage[label] = colour_sel.value;

		if (has_fg.has(label)) {
			const label_fg = label + "_fg";
			const colour_sel_fg = document.getElementById(colour_sel_id + "_fg")! as HTMLInputElement;
			window.localStorage[label_fg] = colour_sel_fg.value;
		}
	}
}


export function set_callback_switch_to_custom_colours(callback: () => void) {
	g_callback_switch_to_custom_colours = callback;
}

document.getElementById("btn_load_existing_colours")!.addEventListener('click', () => {
	load_existing_colours();
	apply_colours();
	if (g_callback_switch_to_custom_colours) {
		g_callback_switch_to_custom_colours();
	}
});


export function set_colour_adjuster_pane_visible(is_visible: boolean) {
	const pane_hidden_classname = "pane_hidden";
	if (is_visible) {
		colour_sel_pane.classList.remove(pane_hidden_classname);
		btn_show_colour_adjuster_pane.innerText = '-';
	} else {
		colour_sel_pane.classList.add(pane_hidden_classname);
		btn_show_colour_adjuster_pane.innerText = '+';
	}
}

function toggle_colour_adjuster_pane_visible() {
	const pane_hidden_classname = "pane_hidden";
	const is_visible = !colour_sel_pane.classList.contains(pane_hidden_classname);
	set_colour_adjuster_pane_visible(!is_visible);
}

const colour_sel_pane = document.getElementById("colour_adjuster_pane")!;
const btn_show_colour_adjuster_pane = document.getElementById("btn_show_colour_adjuster_pane")!;
btn_show_colour_adjuster_pane.addEventListener('click', () => {
	toggle_colour_adjuster_pane_visible();
});


load_existing_colours();
load_colours_from_local_storage();
apply_colours();
