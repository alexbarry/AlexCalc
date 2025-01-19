#include <iostream>
#include <sstream>
#include "calc_units_organizer.h"

// Only needed for SI_PREFIXES, should move it to a separate file
#include "calc_units.h"

std::ostream& operator<<(std::ostream &out, const UnitFactor& arg) {
	return out << arg.to_string();
}
std::ostream& operator<<(std::ostream &out, const UnitInfo& arg) {
	return out << arg.to_string();
}

UnitFactor::UnitFactor(std::string name, int pow) {
	this->name = name;
	this->pow  = pow;
}
std::string UnitFactor::to_string(void) const {
	std::string output;
	output = this->name;
	if (this->pow != 1) {
		output += "^" + std::to_string(this->pow);
	}
	return output;
}

UnitInfo::UnitInfo(std::vector<UnitFactor> unit_factors) {
	this->unit_factors = unit_factors;
}

UnitInfo::UnitInfo(std::string name, int pow) {
	this->unit_factors.push_back(UnitFactor(name, pow));
}

UnitInfo::UnitInfo(std::string name) {
	this->unit_factors.push_back(UnitFactor(name));
}

std::string UnitInfo::to_string(void) const {
	std::string output;
	bool first = true;
	for (const UnitFactor &unit_factor : this->unit_factors) {
		if (!first) {
			output += " ";
		}
		first = false;
		output += unit_factor.to_string();
	}
	return output;
}

static std::vector<UnitInfo> unit_names_to_unit_info_ary(const std::vector<std::string> unit_names) {
	std::vector<UnitInfo> unit_info_ary;
	for (const auto &unit_name : unit_names) {
		unit_info_ary.push_back(UnitInfo(unit_name));
	}
	return unit_info_ary;
}


UnitDescription UnitDescription::si_prefixable(std::string nice_name, std::vector<std::string> unit_names) {
	return UnitDescription(nice_name, unit_names_to_unit_info_ary(unit_names), true);
}

UnitDescription UnitDescription::si_prefixable_unitinfo(std::string nice_name, std::vector<UnitInfo> unit_info_vec) {
	return UnitDescription(nice_name, unit_info_vec, true);
}

UnitDescription UnitDescription::no_prefix(std::string nice_name, std::vector<std::string> unit_names) {
	return UnitDescription(nice_name, unit_names_to_unit_info_ary(unit_names), false);
}

UnitDescription UnitDescription::no_prefix_unitinfo(std::string nice_name, std::vector<UnitInfo> unit_info_vec) {
	return UnitDescription(nice_name, unit_info_vec, false);
}

UnitGroup::UnitGroup(std::string group_name) {
	this->group_name = group_name;
}
void UnitGroup::add_group(const UnitGroup group) {
	this->child_groups.push_back(group);
}
void UnitGroup::add_unit(const UnitDescription unit) {
	this->child_units.push_back(unit);
}

// used only for debugging to see if any units were missed from here, I think
bool UnitDescription::contains_unit_name(std::string name) const {
	for (const auto &this_name : this->unit_info_names) {
		if (this_name.unit_factors.size() != 1) { continue; }
		const std::string unit_name = this_name.unit_factors.at(0).name;
		if (unit_name == name) {
			return true;
		}
		for (const auto &iter : SI_PREFIXES) {
			std::string prefix = iter.first;
			if (prefix + unit_name == name) {
				return true;
			}
		}
	}
	return false;
}

UnitDescription::UnitDescription(std::string nice_name,
                                 std::vector<UnitInfo> unit_info_names,
                                 bool si_prefixable) {
	this->nice_name  = nice_name;
	this->unit_info_names = unit_info_names;
	this->is_si_prefixable = si_prefixable;
}

// used only for debugging to see if any units were missed from here, I think
bool UnitGroup::contains_unit_name(std::string name) const {
	for (const auto &group : this->child_groups) {
		bool found = group.contains_unit_name(name);
		if (found) { return true; }
	}
	for (const auto &unit : this->child_units) {
		bool found = unit.contains_unit_name(name);
		if (found) { return true; }
	}
	return false;
}

void UnitDescription::dump(std::ostream &out, int depth) const {
	static const char spacer[] = "    "; // TODO put in class
	for (int i=0; i<depth; i++) {
		out << spacer;
	}
	out << this->nice_name;
	out << " {";
	bool first = true;
	for (const auto &unit_info : this->unit_info_names) {
		if (!first) {
			out << ", ";
		}
		first = false;
		out << unit_info.to_string();
	}
	out << "}";
	out << std::endl;
}
	

void UnitGroup::dump(std::ostream &out, int depth) const {
	static const char spacer[] = "    "; // TODO put in class
	for (int i=0; i<depth; i++) {
		out << spacer;
	}
	out << this->group_name << std::endl;

	for (const auto &group : this->child_groups) {
		group.dump(out, depth + 1);
	}
	for (const auto &unit_info : this->child_units) {
		unit_info.dump(out, depth + 1);
	}
}


UnitGroup get_distance_units(void) {
	UnitGroup group("Distance");
	group.add_unit(UnitDescription::si_prefixable("metre",          {"m"}));
	group.add_unit(UnitDescription::no_prefix("lightyear",      {"ly"}));
	group.add_unit(UnitDescription::no_prefix("astronomical unit", {"au"}));
	group.add_unit(UnitDescription::no_prefix("parsec",         {"pc"}));
	group.add_unit(UnitDescription::no_prefix("inch",           {"in", "inch"}));
	group.add_unit(UnitDescription::no_prefix("foot",           {"ft", "feet", "foot"}));
	group.add_unit(UnitDescription::no_prefix("yard",           {"yard", "yd"}));
	group.add_unit(UnitDescription::no_prefix("mile",           {"mile", "mi"}));
	group.add_unit(UnitDescription::no_prefix("nautical mile",  {"nmi"}));
	return group;
}

UnitGroup get_area_units(void) {
	UnitGroup group("Area");
	group.add_unit(UnitDescription::si_prefixable_unitinfo("square metre",          {UnitInfo("m", 2)}));
	group.add_unit(UnitDescription::no_prefix("hectare",          {"ha"}));
	group.add_unit(UnitDescription::no_prefix("acre",             {"acre"}));
	group.add_unit(UnitDescription::no_prefix_unitinfo("square inches",        {UnitInfo("in", 2)}));
	group.add_unit(UnitDescription::no_prefix_unitinfo("square feet",          {UnitInfo("ft", 2)}));
	group.add_unit(UnitDescription::no_prefix_unitinfo("square yard",          {UnitInfo("yd", 2)}));
	group.add_unit(UnitDescription::no_prefix_unitinfo("square miles",         {UnitInfo("mi", 2)}));
	return group;
}

UnitGroup get_volume_units(void) {
	UnitGroup group("Volume");
	group.add_unit(UnitDescription::si_prefixable_unitinfo("cubic metre",          {UnitInfo("m", 3)}));
	group.add_unit(UnitDescription::si_prefixable("Litre",          {"L"}));
	group.add_unit(UnitDescription::no_prefix("teaspoon",       {"tsp"}));
	group.add_unit(UnitDescription::no_prefix("tablespoon",     {"tbsp"}));
	group.add_unit(UnitDescription::no_prefix("fluid ounce",    {"floz"}));
	group.add_unit(UnitDescription::no_prefix("cup",            {"cup"}));
	group.add_unit(UnitDescription::no_prefix("gallon",         {"gallon", "gal"}));
	group.add_unit(UnitDescription::no_prefix("quart",          {"quart"}));
	group.add_unit(UnitDescription::no_prefix("pint",           {"pint"}));
	group.add_unit(UnitDescription::no_prefix_unitinfo("cubic inches",        {UnitInfo("in", 3)}));
	group.add_unit(UnitDescription::no_prefix_unitinfo("cubic feet",          {UnitInfo("ft", 3)}));
	group.add_unit(UnitDescription::no_prefix_unitinfo("cubic yard",          {UnitInfo("yd", 3)}));
	group.add_unit(UnitDescription::no_prefix_unitinfo("cubic miles",         {UnitInfo("mi", 3)}));

	return group;
}

UnitGroup get_time_units(void) {
	UnitGroup group("Time");
	group.add_unit(UnitDescription::si_prefixable("second",          {"s"}));
	group.add_unit(UnitDescription::no_prefix("minute",          {"minute", "min"}));
	group.add_unit(UnitDescription::no_prefix("hour",            {"hr", "hour"}));
	group.add_unit(UnitDescription::no_prefix("day",             {"day"}));
	group.add_unit(UnitDescription::no_prefix("month",           {"month"}));
	group.add_unit(UnitDescription::no_prefix("year",            {"year", "yr"}));
	return group;
}

UnitGroup get_speed_units(void) {
	UnitGroup group("Speed");
	group.add_unit(UnitDescription::no_prefix_unitinfo("metres per second",	{
	    	UnitInfo({UnitFactor("m"), UnitFactor("s",-1)}),
		}));
	group.add_unit(UnitDescription::no_prefix_unitinfo("kilometres per hour", {
	    	UnitInfo({UnitFactor("km"), UnitFactor("hr",-1)}),
	    	UnitInfo("kph")
		}));
	group.add_unit(UnitDescription::no_prefix("miles per hour", {"mph"}));
	group.add_unit(UnitDescription::no_prefix_unitinfo("feet per second", {
		UnitInfo({UnitFactor("ft"), UnitFactor("s", -1)})
		}));

	
	return group;
}

UnitGroup get_mass_and_force_units(void) {
	UnitGroup group("Mass/Force");
	group.add_unit(UnitDescription::si_prefixable("gram",        {"g"}));
	group.add_unit(UnitDescription::si_prefixable("Newton",      {"N"}));
	group.add_unit(UnitDescription::si_prefixable("metric ton",  {"ton"}));
	group.add_unit(UnitDescription::no_prefix("pound (mass)",    {"lb"}));
	group.add_unit(UnitDescription::no_prefix("pound (force)",   {"lbf"}));
	group.add_unit(UnitDescription::no_prefix("ounce",           {"oz"}));
	group.add_unit(UnitDescription::no_prefix("slug",            {"slug"}));
	return group;
}

UnitGroup get_pressure_units(void) {
	UnitGroup group("Pressure");
	group.add_unit(UnitDescription::si_prefixable("Pascal",          {"Pa"}));
	group.add_unit(UnitDescription::no_prefix("atmosphere",      {"atm"}));
	group.add_unit(UnitDescription::no_prefix("torr",            {"torr"}));
	group.add_unit(UnitDescription::no_prefix("mm Hg",           {"mmHg"}));
	group.add_unit(UnitDescription::no_prefix("pounds per square inch", {"psi"}));
	group.add_unit(UnitDescription::no_prefix("cm H2O",          {"cmH2O"}));
	group.add_unit(UnitDescription::no_prefix("mm H2O",          {"mmH2O"}));
	return group;
}

UnitGroup get_energy_and_electricity_units(void) {
	UnitGroup group("Energy/Electricity/Frequency");
	group.add_unit(UnitDescription::si_prefixable("Hertz",           {"Hz"}));
	group.add_unit(UnitDescription::si_prefixable("Joule (Energy)",  {"J"}));
	group.add_unit(UnitDescription::si_prefixable("electron Volt",   {"eV"}));
	group.add_unit(UnitDescription::no_prefix("calorie",         {"cal"}));
	group.add_unit(UnitDescription::no_prefix("kilocalorie",     {"kcal"}));
	group.add_unit(UnitDescription::no_prefix("British thermal unit", {"btu"}));
	group.add_unit(UnitDescription::si_prefixable("Watt (Power)",    {"W"}));
	group.add_unit(UnitDescription::no_prefix("horsepower",      {"hp"}));
	//group.add_unit(UnitDescription::no_prefix("Watt hour",       {"W"}));
	group.add_unit(UnitDescription::si_prefixable("Volt (Voltage)",       {"V"}));
	group.add_unit(UnitDescription::si_prefixable("Amp (Current)",        {"A"}));
	group.add_unit(UnitDescription::si_prefixable("Ohm (Resistance)",     {"Ohm"}));
	group.add_unit(UnitDescription::si_prefixable("Coulomb (Charge)",     {"C"}));
	group.add_unit(UnitDescription::si_prefixable("Siemen (Conductance)", {"S"}));
	group.add_unit(UnitDescription::si_prefixable("Tesla",                {"T"}));
	group.add_unit(UnitDescription::si_prefixable("Webber",               {"Wb"}));
	group.add_unit(UnitDescription::si_prefixable("Henry (Inductance)",   {"H"}));
	group.add_unit(UnitDescription::si_prefixable("Farad (Capacitance)",  {"F"}));
	return group;
}

UnitGroup get_misc_units(void) {
	UnitGroup group("Miscellaneous");
	group.add_unit(UnitDescription::si_prefixable("Kelvin (Temperature)", {"K"}));
	group.add_unit(UnitDescription::si_prefixable("moles",                {"mol"}));
	group.add_unit(UnitDescription::si_prefixable("candella",             {"cd"}));
	group.add_unit(UnitDescription::si_prefixable("Gray",                 {"Gy"}));
	group.add_unit(UnitDescription::si_prefixable("Sievert",              {"Sv"}));
	return group;
}


#if 0
checked for unit_name: C, found = 0
checked for unit_name: pint, found = 0
checked for unit_name: Sv, found = 0
checked for unit_name: Gy, found = 0
checked for unit_name: g, found = 0
checked for unit_name: pc, found = 0
checked for unit_name: ton, found = 0
#endif

UnitGroup get_unit_info(void) {
	UnitGroup root("root");
	root.add_group(get_distance_units());
	root.add_group(get_area_units());
	root.add_group(get_volume_units());
	root.add_group(get_time_units());
	root.add_group(get_speed_units());
	root.add_group(get_pressure_units());
	root.add_group(get_mass_and_force_units());
	root.add_group(get_energy_and_electricity_units());
	root.add_group(get_misc_units());
	
	return root;
}

// TODO need a "can have an exponent" option,
// true for everything except an array of units or units with a non default exponent already
// defined (e.g. cubic metres, km hr^-1)
// TODO maybe instead just look at pow in UI? But there is an array...
// maybe if more than one unit, or if pow is not 1?
// Could make decision either in C++ or in UI.
// Maybe it belongs in UI since technically a future UI could handle it
std::string UnitDescription::to_json(void) const {
	std::stringstream ss;
	ss << "{";
	ss << "\"nice_name\": \"" << this->nice_name << "\"";
	ss << ",";

	ss << "\"unit_names\": [" ;
	bool first = true;
	for (const auto &unit_name : this->unit_info_names) {
		if (!first) { ss << ","; }
		first = false;
		ss << "[";
		bool first2 = true;
		for (const auto &unit_factor : unit_name.unit_factors) {
			if (!first2) { ss << ", "; }
			first2 = false;
			ss << "{";
			ss << "\"name\": ";
			ss << "\"" << unit_factor.name << "\"";
			ss << ", ";
			ss << "\"pow\": " << unit_factor.pow;
			ss << "}";
		}
		ss << "]";
	}
	ss << "]";
	ss << ",";


	ss << "\"si_prefixable\": " << (this->is_si_prefixable ? "true" : "false");
	ss << "}";
	return ss.str();
}

std::string UnitGroup::to_json(void) const {
	std::stringstream ss;
	ss << "{";
	ss << "\"group_name\": \"" << this->group_name << "\",";
	ss << "\"child_groups\": [";
	{
		bool first = true;
		for (const UnitGroup &group : this->child_groups) {
			if (!first) {
				ss << ", ";
			}
			first = false;
			ss << group.to_json();
		}
	}
	ss << "]";
	ss << ",";

	ss << "\"child_units\": [";
	{
		bool first = true;
		for (const UnitDescription &group : this->child_units) {
			if (!first) {
				ss << ", ";
			}
			first = false;
			ss << group.to_json();
		}
	}
	ss << "]";
	ss << "}";
	return ss.str();
}

int UnitGroup::group_count() const {
    return this->child_groups.size();

}
int UnitGroup::get_item_count(int group_idx) const {
    return this->get_group(group_idx).child_units.size();
}

UnitGroup UnitGroup::get_group(int idx) const {
	auto iter = this->child_groups.begin();
	while (idx > 0) {
		iter++;
		idx--;
	}
	return *iter;
}
UnitDescription UnitGroup::get_item(int idx) const {
	auto iter = this->child_units.begin();
	while (idx > 0) {
		iter++;
		idx--;
	}
	return *iter;
}

std::string UnitGroup::get_group_name(int group_idx) const {
	UnitGroup group = this->get_group(group_idx);
	return group.group_name;
}
std::string UnitGroup::get_item_json(int group_idx, int item_idx) const {
	UnitDescription item = this->get_group(group_idx).get_item(item_idx);
	return item.to_json();
}

// 
// g++ src/calc_core/calc_units_organizer.cpp -I src/calc_core/includes/ -o bin/organizer -D TEST_UNIT_ORGANIZER && bin/organizer
// g++ src/calc_core/calc_units_organizer.cpp src/calc_core/calc_units.cpp src/calc_core/calc_core.cpp src/calc_core/calc_core_exceptions.cpp -I src/calc_core/includes/ -o bin/organizer -D TEST_UNIT_ORGANIZER && bin/organizer
#ifdef TEST_UNIT_ORGANIZER
int main(void) {
	UnitGroup root = get_unit_info();
	root.dump(std::cout);
	auto units = get_units();
	int found_count = 0;
	int not_found_count = 0;
	for (const auto& iter : units) {
		std::string unit_name = iter.first;
		bool found = root.contains_unit_name(unit_name);
		if (!found) {
			not_found_count += 1;
			std::cout << "checked for unit_name: " << unit_name << ", found = " << found << std::endl;
		} else {
			found_count += 1;
		}
		
	}
	std::cout << "Found: " << found_count << std::endl;
	std::cout << "Not found: " << not_found_count << std::endl;
}
#endif
