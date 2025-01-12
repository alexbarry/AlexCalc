
// Passed to the WASM AlexCalc binary
export interface CalcParams {
	polar: boolean;
	degree: boolean;
}

export interface InputTokenT {
	str: string;
	is_unit: boolean;
}

export interface RawCalcInputOutput {
	raw_input: string;
	latex: string;
	calc_output: CalcOutput;
}

export interface CalcState {
	new_var_btns_map: Map<HTMLElement, string>;
	alt_state: boolean;
	inv_state: boolean;
	polar_state: boolean;
	degree_state: boolean;
	input_tokens: InputTokenT[];
	cursor_idx: number;

	input_history: InputTokenT[][];
	input_history_pos: number;

	show_raw_calc_io: boolean;

	show_var_display: boolean;
	show_unit_display: boolean;
	show_about_popup: boolean;

	output_display_srcs: RawCalcInputOutput[];

	var_btn_selected: HTMLElement|null;
}

export interface CalcUi {
	update_input_wip_display(tex: string): void;
	output_disp_scroll_to_bottom(): void;
	clear_latex_btn_stylesheets(): void;
	generate_latex_node_btn(tex: string, format: "button"|undefined, parent: HTMLElement): Node;
	clear_output_display: () => void;
	calc_async: (raw_input: string, callback: (raw_input: string, latex: string, calc_output: CalcOutput, calcdata: CalcData) => void) => void;
	set_focus_to_textarea: () => void;
	var_btn_selected: (var_btn_elem: HTMLElement|null) => void;

	generate_output_msg: (msg: string, is_err: boolean) => void;
	generate_output_display: (raw_input: string, latex: string, calc_output: CalcOutput, show_raw: boolean) => void;
	update_vars: (vars: [string, string][]) => void;

	to_latex_async: (raw_input: string, idx: number) => void;
	remove_input_wip_display_stylesheets: () => void;

	is_mobile: () => boolean;

	update_calcstate: (calcstate: CalcParams) => void;

	state: CalcState;
	unit_sel: CalcUnitSel;



	show_input_wip_display: boolean;
	selected_theme: string;

	btn_alt: HTMLElement;
	btn_inv: HTMLElement;
	btn_left: HTMLElement;
	btn_right: HTMLElement;
	btn_up: HTMLElement;
	btn_down: HTMLElement;
	btn_bksp: HTMLElement;
	btn_clear: HTMLElement;
	btn_enter: HTMLElement;

	btn_units: HTMLElement;
	btn_vars: HTMLElement;

	btn_var_display_cancel: HTMLElement;
	btn_unit_display_cancel: HTMLElement;
	show_about_popup_btns: HTMLElement[];
	close_about_popup_btns: HTMLElement[];

	new_var_btns: { id: string; var_name: string }[];

	btn_sin: HTMLElement;
	btn_cos: HTMLElement;
	btn_tan: HTMLElement;
	btn_img_unit: HTMLElement;
	btn_log: HTMLElement;
	btn_ln: HTMLElement;
	btn_sqrt: HTMLElement;
	btn_degree_toggle: HTMLElement;
	btn_polar_toggle: HTMLElement;
	btn_var1: HTMLElement;
	btn_pi: HTMLElement;
	btn_e: HTMLElement;
	normal_btns: HTMLElement[];


	input_text: HTMLInputElement;

	checkbox_show_raw: HTMLInputElement;
	var_selector_display: HTMLElement;
	custom_var_name_input: HTMLInputElement;
	btn_var_display_insert: HTMLInputElement;

	about_popup: HTMLElement;

	dark_mode_select: HTMLSelectElement;
};

export interface CalcUnitSel {};
export interface CalcOutput {};
export interface CalcData {
	vars: [string, string][];
};
