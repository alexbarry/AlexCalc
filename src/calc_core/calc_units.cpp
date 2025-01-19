#include<string>
#include<vector>
#include<unordered_map>
#include<list>
#include<cmath>

#include "calc_core_types.h"
#include "calc_core_exceptions.h"
#include "calc_units.h"
//#include "calc_core.h" // TODO remove?

std::unordered_map<std::string, UnitInfoParsed> get_base_si_units(void);

const std::unordered_map<std::string, int> SI_PREFIXES = std::unordered_map<std::string, int> {
	{ "Y",   24 },
	{ "Z",   21 },
	{ "E",   18 },
	{ "P",   15 },
	{ "T",   12 },
	{ "G",    9 },
	{ "M",    6 },
	{ "k",    3 },
	{ "h",    2 },
	{ "da",   1 },

	{ "d",   -1 },
	{ "c",   -2 },
	{ "m",   -3 },
	{ "u",   -6 },
	{ "n",   -9 },
	{ "p",  -12 },
	{ "f",  -15 },
	{ "a",  -18 },
	{ "z",  -21 },
	{ "y",  -24 },
};


UnitInfoParsed::UnitInfoParsed(std::string prefix, std::string base, int pow, unit_t base_val) {
	this->prefix = prefix;
	this->base   = base;
	this->pow    = pow;
	this->unit   = base_val;
}

UnitInfoParsed::UnitInfoParsed(const UnitInfoParsed &arg) {
	this->prefix = arg.prefix;
	this->base   = arg.base;
	this->pow    = arg.pow;
	this->unit   = arg.unit;
}

std::string UnitInfoParsed::to_string(void) const {
	std::string output = "{";
	output += "prefix: \"" + this->prefix + "\"";
	output += ", ";
	output += "base: \"" + this->base + "\"";
	output += ", ";
	output += "pow: \"" + std::to_string(this->pow) + "\"";
	output += ", ";
	output += "val: \"" + unit_to_string(this->unit) + "\"";
	output += "}";
	return output;
}

unit_t UnitInfoParsed::get_val(void) const {
	return pow_unit(this->unit, this->pow);
};



unit_dim_t sqrt_unit(const unit_dim_t &arg) {
	if ( arg.s   % 2 != 0 ||
	     arg.m   % 2 != 0 ||
	     arg.kg  % 2 != 0 ||
	     arg.A   % 2 != 0 ||
	     arg.K   % 2 != 0 ||
	     arg.mol % 2 != 0 ||
	     arg.cd  % 2 != 0) {
		throw UnitInvalidOperationException("can not take sqrt of unit " + unit_dim_to_string(arg));
	}
	unit_dim_t dim = {
		.s   = arg.s/2,
		.m   = arg.m/2,
		.kg  = arg.kg/2,
		.A   = arg.A/2,
		.K   = arg.K/2,
		.mol = arg.mol/2,
		.cd  = arg.cd/2,
	};
	return dim;
}


bool units_dim_eq(const unit_dim_t &left, const unit_dim_t &right) {
	return left.s   == right.s   &&
	       left.m   == right.m   &&
	       left.kg  == right.kg  &&
	       left.A   == right.A   &&
	       left.K   == right.K   &&
	       left.mol == right.mol &&
	       left.cd  == right.cd;
}

unit_t mult_units(calc_float_t scalar, const unit_t right) {
	unit_t output;
	output.mag     = scalar * right.mag;
	output.dim.s   = right.dim.s;
	output.dim.m   = right.dim.m;
	output.dim.kg  = right.dim.kg;
	output.dim.A   = right.dim.A;
	output.dim.K   = right.dim.K;
	output.dim.mol = right.dim.mol;
	output.dim.cd  = right.dim.cd;
	return output;
}

unit_t mult_units(const unit_t left, const unit_t right) {
	unit_t output;
	output.mag     = left.mag     * right.mag;
	output.dim = mult_units_dim(left.dim, right.dim);
	return output;
}

val_t mult_units(const val_t val_arg, const unit_t unit) {
	val_t new_val;
	new_val = val_arg;
	new_val.unit_dim = mult_units_dim(new_val.unit_dim, unit.dim);
	new_val.re *= unit.mag;
	new_val.im *= unit.mag;
	return new_val;
}

unit_dim_t mult_units_dim(const unit_dim_t left, const unit_dim_t right) {
	unit_dim_t output;
	output.s   = left.s   + right.s;
	output.m   = left.m   + right.m;
	output.kg  = left.kg  + right.kg;
	output.A   = left.A   + right.A;
	output.K   = left.K   + right.K;
	output.mol = left.mol + right.mol;
	output.cd  = left.cd  + right.cd;
	return output;
}

unit_t inv_unit(const unit_t arg) {
	unit_t output;
	output.mag = 1.0/arg.mag;
	output.dim = inv_unit_dim(arg.dim);
	return output;
}

unit_dim_t inv_unit_dim(const unit_dim_t arg) {
	unit_dim_t output;
	output.s   = -arg.s;
	output.m   = -arg.m;
	output.kg  = -arg.kg;
	output.A   = -arg.A;
	output.K   = -arg.K;
	output.mol = -arg.mol;
	output.cd  = -arg.cd;
	return output;
}

unit_t div_units(const unit_t left, const unit_t right) {
	return mult_units(left, inv_unit(right));
}

unit_dim_t div_units_dim(const unit_dim_t left, const unit_dim_t right) {
	return mult_units_dim(left, inv_unit_dim(right));
}

unit_t pow_unit(const unit_t arg_base, int arg_pow) {
	unit_t output;
	output.mag = std::pow(arg_base.mag, arg_pow);
	output.dim = pow_unit_dim(arg_base.dim, arg_pow);
	return output;
}
unit_dim_t pow_unit_dim(const unit_dim_t arg_base, int arg_pow) {
	unit_dim_t output;
	output.s   = arg_base.s   * arg_pow;
	output.m   = arg_base.m   * arg_pow;
	output.kg  = arg_base.kg  * arg_pow;
	output.A   = arg_base.A   * arg_pow;
	output.K   = arg_base.K   * arg_pow;
	output.mol = arg_base.mol * arg_pow;
	output.cd  = arg_base.cd  * arg_pow;
	return output;
}

void add_si_prefixes_of_unit(std::unordered_map<std::string, UnitInfoParsed> &units,
                             std::string unit_name, 
                             unit_t unit_val) {
	for (const auto& iter : SI_PREFIXES) {
		std::string prefix     = iter.first;
		int         base10_pow = iter.second;
		std::string new_unit_name = prefix + unit_name;
		unit_t      new_unit_val  = unit_val;
		new_unit_val.mag *= std::pow(10, base10_pow);
		units[new_unit_name] = UnitInfoParsed(prefix, unit_name, 1, new_unit_val);
	}
}

std::string print_unit_dim(const std::string &unit_dim, int pow) {
	if (pow == 0) {
		return "";
	} else if (pow == 1) {
		return " " + unit_dim;
	} else {
		return " " + unit_dim + "^" + std::to_string(pow);
	}
}

std::string unit_to_string(const unit_t &unit) {
	std::string unit_mag;
	if (unit.mag == 1.0) {
		unit_mag = "1.0";
	} else {
		char mag_buff_str[256];
		snprintf(mag_buff_str, sizeof(mag_buff_str), "%" CALC_FLOAT_FMT "e", unit.mag);
		unit_mag = std::string(mag_buff_str);
	}
	std::string output = "";
	output += unit_mag;
	output += unit_dim_to_string(unit.dim);
	return output;
}

std::string unit_dim_to_string(const unit_dim_t &unit_dim) {
	std::string output = "";
	output += print_unit_dim("kg",  unit_dim.kg);
	output += print_unit_dim("m",   unit_dim.m);
	output += print_unit_dim("s",   unit_dim.s);
	output += print_unit_dim("A",   unit_dim.A);
	output += print_unit_dim("K",   unit_dim.K);
	output += print_unit_dim("mol", unit_dim.mol);
	output += print_unit_dim("cd",  unit_dim.cd);
	return output;
}

std::string unit_info_to_plaintext(const UnitInfoParsed &unit_info) {
	std::string output = "";
	output = unit_info.prefix + unit_info.base;
	if (unit_info.pow != 1) {
		output += "^" + std::to_string(unit_info.pow);
	}
	return output;
}


std::unique_ptr<UnitInfoParsed> unit_dim_to_string_nice_lookup(const unit_dim_t &unit_dim) {
	std::unordered_map<std::string, UnitInfoParsed> units = get_base_si_units();

	for (const auto &iter : units) {
		std::string nice_unit_name = iter.first;
		UnitInfoParsed nice_unit_val  = iter.second;

		if (nice_unit_val.unit.mag != 1.0) {
			throw BaseCalcException("si unit lookup table has unit with mag " + std::to_string(nice_unit_val.unit.mag));
		}

		if (units_dim_eq(unit_dim, nice_unit_val.unit.dim)) {
			return std::unique_ptr<UnitInfoParsed>(new UnitInfoParsed(nice_unit_val));
		}
	}
	// TODO look up in some list of "nice" SI units
	return std::unique_ptr<UnitInfoParsed>(nullptr);
}

unit_dim_t init_unit_dim(void) {
	unit_dim_t dim = {
		.s   = 0,
		.m   = 0,
		.kg  = 0,
		.A   = 0,
		.K   = 0,
		.mol = 0,
		.cd  = 0,
	};
	return dim;
}

unit_t init_unit(void) {
	unit_t unit = {
		.mag = 1.0,
		.dim = init_unit_dim(),
	};
	return unit;
}

bool units_non_zero(const unit_dim_t &arg) {
	return arg.s   != 0 ||
	       arg.m   != 0 ||
	       arg.kg  != 0 ||
	       arg.K   != 0 ||
	       arg.A   != 0 ||
	       arg.mol != 0 ||
	       arg.cd  != 0;
}

#define EXISTING_UNIT(new_name, old_unit_name) \
	( UnitInfoParsed("", new_name, 1, units.at(old_unit_name).unit) )

// TODO should split this into core SI units that everyone should want, and
// imperial/cooking units
std::unordered_map<std::string, UnitInfoParsed> get_base_si_units(void) {
	// https://en.wikipedia.org/wiki/International_System_of_Units

	std::unordered_map<std::string, UnitInfoParsed> units;

	const unit_t unit_s   = { .mag = 1, .dim = { .s = 1, .m = 0, .kg = 0, .A = 0, .K = 0, .mol = 0, .cd = 0}};
	const unit_t unit_m   = { .mag = 1, .dim = { .s = 0, .m = 1, .kg = 0, .A = 0, .K = 0, .mol = 0, .cd = 0}};
	const unit_t unit_kg  = { .mag = 1, .dim = { .s = 0, .m = 0, .kg = 1, .A = 0, .K = 0, .mol = 0, .cd = 0}};
	const unit_t unit_A   = { .mag = 1, .dim = { .s = 0, .m = 0, .kg = 0, .A = 1, .K = 0, .mol = 0, .cd = 0}};
	const unit_t unit_K   = { .mag = 1, .dim = { .s = 0, .m = 0, .kg = 0, .A = 0, .K = 1, .mol = 0, .cd = 0}};
	const unit_t unit_mol = { .mag = 1, .dim = { .s = 0, .m = 0, .kg = 0, .A = 0, .K = 0, .mol = 1, .cd = 0}};
	const unit_t unit_cd  = { .mag = 1, .dim = { .s = 0, .m = 0, .kg = 0, .A = 0, .K = 0, .mol = 0, .cd = 1}};

	units["s"]   = UnitInfoParsed("", "s",   1, unit_s  );
	units["m"]   = UnitInfoParsed("", "m",   1, unit_m  );
	units["kg"]  = UnitInfoParsed("", "kg",  1, unit_kg );
	units["A"]   = UnitInfoParsed("", "A",   1, unit_A  );
	units["K"]   = UnitInfoParsed("", "K",   1, unit_K  );
	units["mol"] = UnitInfoParsed("", "mol", 1, unit_mol);
	units["cd"]  = UnitInfoParsed("", "cd",  1, unit_cd );

	units["Hz"]  = UnitInfoParsed("", "Hz", 1, inv_unit(  units.at("s").unit));
	units["N"]   = UnitInfoParsed("", "N",  1, { .mag = 1, .dim = { .s = -2, .m =  1, .kg =  1, .A =  0, .K =  0, .mol =  0, .cd =  0}});
	units["Pa"]  = UnitInfoParsed("", "Pa", 1, { .mag = 1, .dim = { .s = -2, .m = -1, .kg =  1, .A =  0, .K =  0, .mol =  0, .cd =  0}});
	units["J"]   = UnitInfoParsed("", "J",  1, mult_units(units.at("N").unit, units.at("m").unit));
	units["W"]   = UnitInfoParsed("", "W",  1, div_units( units.at("J").unit, units.at("s").unit));
	units["C"]   = UnitInfoParsed("", "C",  1, mult_units(units.at("A").unit, units.at("s").unit));
	units["V"]   = UnitInfoParsed("", "V",  1, div_units( units.at("W").unit, units.at("A").unit));
	units["F"]   = UnitInfoParsed("", "F",  1, div_units( units.at("C").unit, units.at("V").unit));
	units["Ohm"] = UnitInfoParsed("", "Ohm",1, div_units( units.at("V").unit, units.at("A").unit));
	units["S"]   = UnitInfoParsed("", "S",  1, inv_unit(  units.at("Ohm").unit));
	units["Wb"]  = UnitInfoParsed("", "Wb", 1, mult_units( units.at("V").unit, units.at("s").unit));
	units["T"]   = UnitInfoParsed("", "T",  1, div_units( units.at("Wb").unit, pow_unit(units.at("s").unit,2)));
	units["H"]   = UnitInfoParsed("", "H",  1, div_units( units.at("Wb").unit, units.at("A").unit));

	units["Gy"]  = UnitInfoParsed("", "Gy", 1, div_units( units.at("J").unit, units.at("kg").unit));
	units["Sv"]  = UnitInfoParsed("", "Sv", 1, div_units( units.at("J").unit, units.at("kg").unit));

	return units;
}


void add_non_standard_units(std::unordered_map<std::string, UnitInfoParsed> &units) {
	units["sec"]     = EXISTING_UNIT("sec",     "s");
	units["secs"]    = EXISTING_UNIT("secs",    "s");
	units["second"]  = EXISTING_UNIT("second",  "s");
	units["seconds"] = EXISTING_UNIT("seconds", "s");
	units["min"]    = UnitInfoParsed("", "min", 1, mult_units(60, units.at("s").unit));
	units["minute"]   = EXISTING_UNIT("minute", "min");
	units["minutes"]  = EXISTING_UNIT("minutes", "min");
	units["hr"]       = UnitInfoParsed("", "hr", 1, mult_units(60, units.at("min").unit));
	units["hour"]     = EXISTING_UNIT("hour", "hr");
	units["hours"]    = EXISTING_UNIT("hours", "hr");
	units["day"]      = UnitInfoParsed("", "day", 1, mult_units(24, units.at("hour").unit));
	units["days"]     = EXISTING_UNIT("days", "day");
	units["month"]  = UnitInfoParsed("", "month", 1, mult_units(365.24/12, units.at("day").unit));
	units["months"]  = EXISTING_UNIT("months", "month");
	units["yr"]      = UnitInfoParsed("", "yr", 1, mult_units(365.24, units.at("day").unit));
	units["year"]    = EXISTING_UNIT("year", "yr");
	units["years"]   = EXISTING_UNIT("years", "yr");

	units["au"]     = UnitInfoParsed("", "au", 1, mult_units(149597870700, units.at("m").unit));
	units["ly"]       = UnitInfoParsed("", "ly", 1, mult_units(9.4607e15, units.at("m").unit));
	units["lightyear"] = EXISTING_UNIT("lightyear", "ly");
	units["lightyears"] = EXISTING_UNIT("lightyears", "ly");
	units["pc"]     = UnitInfoParsed("", "pc", 1, mult_units(3.26156, units.at("ly").unit));
	units["parsec"]     = EXISTING_UNIT("parsec", "pc");
	units["parsecs"]     = EXISTING_UNIT("parsecs", "pc");

	units["ha"]     = UnitInfoParsed("", "ha", 1, mult_units(1e4,  pow_unit(units.at("m").unit,2))); // 1 hm^2
	units["hectare"]   = EXISTING_UNIT("hectare", "ha");
	units["hectares"]  = EXISTING_UNIT("hectares", "ha");
	units["L"]      = UnitInfoParsed("", "L", 1, mult_units(1e-3, pow_unit(units.at("m").unit,3))); // 1 dm^3 == 10^3 cm^3 == 1e-3 m^3
	//units["L"]      = UnitInfoParsed("", "L", 1, mult_units(1,    pow_unit(units.at("dm").unit,3)));
	units["l"]         = EXISTING_UNIT("l", "L");
	units["ml"]        = UnitInfoParsed("", "ml", 1, mult_units(1e-3, pow_unit(units.at("L").unit,1))); // 1 dm^3 == 10^3 cm^3 == 1e-3 m^3
	units["litre"]     = EXISTING_UNIT("litre", "L");
	units["litres"]    = EXISTING_UNIT("litres", "L");
	units["liter"]     = EXISTING_UNIT("liter", "L");
	units["liters"]     = EXISTING_UNIT("liters", "L");

	units["g"] = UnitInfoParsed("", "g", 1, mult_units(1e-3, units.at("kg").unit));
	add_si_prefixes_of_unit(units, "g", mult_units(1e-3, units.at("kg").unit));

	units["eV"] = UnitInfoParsed("", "eV", 1, mult_units(1.602e-19, units.at("J").unit));

	// TODO move this to base SI units or a separate function
	std::list<std::string> has_si_prefixes = std::list<std::string> {
		"s",
		"m",  
		// NOTE: do not include "kg" here,
		"A",  
		"K",  
		"mol",
		"cd", 
	
		"Hz", 
		"N",  
		"Pa", 
		"J",  
		"W",  
		"C",  
		"V",  
		"F",  
		"Ohm",
		"S",  
		"Wb", 
		"T",  
		"H",  
		"L",  
	
		"Gy", 
		"Sv", 

		"eV", 
	};

	for (const auto &unit_name : has_si_prefixes) {
		add_si_prefixes_of_unit(units, unit_name, units.at(unit_name).unit);
	}


	units["in"]     = UnitInfoParsed("", "in", 1, mult_units(2.54, units.at("cm").unit));
	units["inch"]   = UnitInfoParsed("", "inch", 1, units.at("in").unit);
	units["inches"] = UnitInfoParsed("", "inches", 1, units.at("in").unit);
	units["ft"]     = UnitInfoParsed("", "ft", 1, mult_units(12,   units.at("in").unit));
	units["feet"]   = UnitInfoParsed("", "feet", 1, units.at("ft").unit);
	units["foot"]   = UnitInfoParsed("", "foot", 1, units.at("ft").unit);
	units["yd"]     = UnitInfoParsed("", "yd", 1, mult_units(3, units.at("ft").unit));
	units["yard"]   = UnitInfoParsed("", "yard", 1, units.at("yd").unit);
	units["yards"]  = UnitInfoParsed("", "yards", 1, units.at("yd").unit);
	units["mi"]     = UnitInfoParsed("", "mi", 1, mult_units(5280, units.at("ft").unit));
	units["mile"]   = UnitInfoParsed("", "mile", 1, units.at("mi").unit);
	units["miles"]  = UnitInfoParsed("", "miles", 1, units.at("mi").unit);
	units["nmi"]    = UnitInfoParsed("", "nmi", 1, mult_units(1852, units.at("m").unit));

	units["mph"]    = UnitInfoParsed("", "mph", 1, div_units(units.at("mi").unit, units.at("hr").unit));
	units["kph"]    = UnitInfoParsed("", "kph", 1, div_units(units.at("km").unit, units.at("hr").unit));

	units["acre"]   = UnitInfoParsed("", "acre", 1, mult_units(4840, pow_unit(units.at("yd").unit,2)));
	units["acres"]  = UnitInfoParsed("", "acres", 1, units.at("acre").unit);

	units["tsp"]    = UnitInfoParsed("", "tsp", 1, mult_units(4.92892159375, units.at("mL").unit));
	units["tbsp"]   = UnitInfoParsed("", "tbsp", 1, mult_units(3, units.at("tsp").unit));
	units["floz"]   = UnitInfoParsed("", "floz", 1, mult_units(2, units.at("tbsp").unit));
	units["cup"]    = UnitInfoParsed("", "cup", 1, mult_units(8, units.at("floz").unit));
	units["cups"]   = UnitInfoParsed("", "cups", 1, units.at("cup").unit);
	units["pint"]   = UnitInfoParsed("", "pint", 1, mult_units(2, units.at("cup").unit));
	units["pints"]  = UnitInfoParsed("", "pints", 1, units.at("pint").unit);
	units["quart"]  = UnitInfoParsed("", "quart", 1, mult_units(2, units.at("pint").unit));
	units["quarts"] = UnitInfoParsed("", "quarts", 1, units.at("quart").unit);
	units["gal"]    = UnitInfoParsed("", "gal", 1, mult_units(4, units.at("quart").unit));
	units["gallon"] = UnitInfoParsed("", "gallon", 1, units.at("gal").unit);
	units["gallons"] = UnitInfoParsed("", "gallons", 1, units.at("gal").unit);


	units["oz"]     = UnitInfoParsed("", "oz", 1, mult_units(28.439523125, units.at("g").unit));
	units["ounce"]  = EXISTING_UNIT("ounce", "oz");
	units["ounces"] = EXISTING_UNIT("ounces", "oz");
	units["lb"]     = UnitInfoParsed("", "lb", 1, mult_units(16, units.at("oz").unit));
	units["pound"]  = UnitInfoParsed("", "pound", 1, units.at("lb").unit);
	units["pounds"] = UnitInfoParsed("", "pounds", 1, units.at("lb").unit);
	units["ton"]    = UnitInfoParsed("", "ton", 1, mult_units(2000, units.at("lb").unit));
	units["tons"]   = UnitInfoParsed("", "tons", 1, units.at("ton").unit);

	units["btu"]    = UnitInfoParsed("", "btu", 1, mult_units(1.055, units.at("kJ").unit));
	units["cal"]    = UnitInfoParsed("", "cal", 1, mult_units(4.184, units.at("J").unit));
	units["calorie"]  = EXISTING_UNIT("calorie", "cal");
	units["calories"] = EXISTING_UNIT("calories", "cal");
	units["kcal"]   = UnitInfoParsed("", "kcal", 1, mult_units(1000, units.at("cal").unit));
	units["kilocalorie"]  = EXISTING_UNIT("kilocalorie",      "kcal");
	units["kilocalories"] = EXISTING_UNIT("kilocalories",      "kcal");

	// "Food calories" have a capital C and are 1 kcal.
	// https://en.wikipedia.org/wiki/Calorie
	units["Cal"]       = EXISTING_UNIT("Cal",      "kcal");
	units["Calorie"]   = EXISTING_UNIT("Calorie",  "kcal");
	units["Calories"]  = EXISTING_UNIT("Calories", "kcal");
	units["hp"]     = UnitInfoParsed("", "hp", 1, mult_units(745.7, units.at("W").unit));
	units["slug"]   = UnitInfoParsed("", "slug", 1, mult_units(14.59390, units.at("kg").unit));
	units["lbf"]    = UnitInfoParsed("", "lbf", 1, mult_units(4.448222, units.at("N").unit));

	units["atm"]    = UnitInfoParsed("", "atm", 1, mult_units(101325,  units.at("Pa").unit));
	units["atmosphere"] = EXISTING_UNIT("atmosphere", "atm");
	units["atmospheres"] = EXISTING_UNIT("atmospheres", "atm");
	units["torr"]   = UnitInfoParsed("", "torr", 1, mult_units(1.0/760, units.at("atm").unit));
	units["mmHg"]   = UnitInfoParsed("", "mmHg", 1, units.at("torr").unit);
	units["psi"]    = UnitInfoParsed("", "psi", 1, mult_units(6.894757, units.at("kPa").unit));
	units["mmH2O"]  = UnitInfoParsed("", "mmH2O", 1, mult_units(9.80638, units.at("Pa").unit));
	units["cmH2O"]  = UnitInfoParsed("", "cmH2O", 1, mult_units(10, units.at("mmH2O").unit));

	units["ohm"]    = EXISTING_UNIT("ohm", "Ohm");
	add_si_prefixes_of_unit(units, "ohm", units.at("ohm").unit);

#if 0
	for (const auto &iter : units) {
		std::cerr << iter.first << ": " << unit_to_string(iter.second) << std::endl;
	}
#endif

}


std::unordered_map<std::string, UnitInfoParsed> get_units(void) {
	std::unordered_map<std::string, UnitInfoParsed> units = get_base_si_units();
	add_non_standard_units(units);
	return units;
}
