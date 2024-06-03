#ifndef  CALC_JSON_H_
#define CALC_JSON_H_

//
extern "C" {

int alexcalc_init(void);
int alexcalc_json_str_output(const char *str_input, void *calcdata_ptr, char *str_output, int str_output_len) noexcept;
int alexcalc_to_latex(const char *str_input, const void *calcdata_ptr, char *str_output, int str_output_len, int cursor_pos, bool parse_wip) noexcept;
void *alexcalc_new_calcdata(void);
void alexcalc_free_calcdata(void *arg);
int alexcalc_calcdata_to_json(void *calcdata_ptr, char *str_output_arg, int str_output_len) noexcept;
int alexcalc_data_state_set(void *calcdata_ptr, bool polar, bool degree) noexcept;

int alexcalc_get_unit_info_json(char *unit_info_str_out, int unit_info_str_len);

int alexcalc_get_unit_info_group_count(void);
int alexcalc_get_unit_info_group_item_count(int group_idx);
int alexcalc_get_unit_info_json_group_name(char *unit_info_str_out, int unit_info_str_len,
                                           int group_idx);
int alexcalc_get_unit_info_json_item(char *unit_info_str_out, int unit_info_str_len,
                                     int group_idx, int item_idx);

int alexcalc_add_recently_used_unit(void *calc_ptr, const char *unit_str, int unit_str_len);
int alexcalc_get_recently_used_units_json(void *calc_ptr, char *units_str_out, int units_str_out_len);
int alexcalc_delete_recently_used_units(void *calc_ptr);
int alexcalc_delete_vars(void *calc_ptr);
}

#endif
