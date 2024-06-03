#ifndef CALC_UNITS_H_
#define CALC_UNITS_H_

#include<memory>
#include<unordered_map>

#include "calc_core_types.h"

class UnitInfoParsed {
	public:
	UnitInfoParsed() = default; // TODO should I have this? It seems like I need to to add these to a hashmap...
	                            // should I make it raise an exception?
	UnitInfoParsed(std::string prefix, std::string base, int pow, unit_t base_val);
	UnitInfoParsed(const UnitInfoParsed &arg);
	std::string to_string(void) const;
	unit_t get_val(void) const;
	
	std::string prefix;
	std::string base;
	int         pow;
	unit_t      unit;
};

extern const std::unordered_map<std::string, int> SI_PREFIXES;

std::unordered_map<std::string, UnitInfoParsed> get_units(void);

std::string unit_to_string(const unit_t &unit);
std::string unit_dim_to_string(const unit_dim_t &unit);

unit_dim_t sqrt_unit(const unit_dim_t &arg);
bool units_dim_eq(const unit_dim_t &left, const unit_dim_t &right);
unit_t mult_units(calc_float_t scalar, const unit_t right);
unit_t mult_units(const unit_t left, const unit_t right);
val_t mult_units(const val_t val_arg, const unit_t unit);
unit_dim_t mult_units_dim(const unit_dim_t left, const unit_dim_t right);
unit_t div_units(const unit_t left, const unit_t right);
unit_dim_t inv_unit_dim(const unit_dim_t arg);
unit_dim_t div_units_dim(const unit_dim_t left, const unit_dim_t right);
unit_dim_t pow_unit_dim(const unit_dim_t arg_base, int arg_pow);
unit_dim_t init_unit_dim(void);
bool units_non_zero(const unit_dim_t &arg);
unit_t init_unit(void);
unit_t pow_unit(const unit_t arg_base, int arg_pow);

std::unique_ptr<UnitInfoParsed> unit_dim_to_string_nice_lookup(const unit_dim_t &unit_dim);
std::string unit_info_to_plaintext(const UnitInfoParsed &unit_info);

#endif
