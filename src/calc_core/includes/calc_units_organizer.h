#ifndef CALC_UNITS_ORGANIZER_H_
#define CALC_UNITS_ORGANIZER_H_

#include<vector>
#include<list>
#include<string>
#include<ostream>


// TODO right now, when selecting a unit, each entry is at most one unit,
// with no specified power
//
// e.g. "mph", which can be "mph^2", or
//      "m", which can be modified to "km^-1"
//
//
// I need to be able to add an entry for "metres per second", "m s^-1", and
// "km hr^-1".
//
// Also would like to have "m^2" and "ft^2" etc
//
// Does that mean each std::string should be changed to UnitInfoInputAry (vector of unit name and pow)?
// Maybe a new class that does separate prefix. Maybe not for now, since I don't think changing
// the prefixes on these combined convenience units is really important


// one piece of a unit, e.g. "hr^-1" or "m"
class UnitFactor {
	public:
	UnitFactor(std::string name, int pow=1);
	std::string to_string(void) const;

	std::string name;
	int pow;
};

// all the units that can be attached to a single value, e.g. "km hr^-1", "s"
class UnitInfo {
	public:
	UnitInfo(std::string name);
	UnitInfo(std::string name, int pow);
	UnitInfo(std::vector<UnitFactor> unit_factors);
	std::string to_string(void) const;

	std::vector<UnitFactor> unit_factors;
};


std::ostream& operator<<(std::ostream &out, const UnitFactor& arg);
std::ostream& operator<<(std::ostream &out, const UnitInfo& arg);

// User will select one of these when picking a unit to input.
// Should be selected from its `nice_name`, e.g. "seconds" or "Kelvin (Temperature)".
// Can then modify it by choosing an si prefix (if `si_prefixable` is true), and
// the variants of "unit names" (e.g. "s", "sec", "second", "seconds")
class UnitDescription {
	public:
	static UnitDescription si_prefixable(std::string nice_name, std::vector<std::string> unit_names);
	static UnitDescription si_prefixable_unitinfo(std::string nice_name, std::vector<UnitInfo> unit_names);
	static UnitDescription no_prefix(std::string nice_name, std::vector<std::string> unit_names);
	static UnitDescription no_prefix_unitinfo(std::string nice_name, std::vector<UnitInfo> unit_names);

	std::string to_json(void) const;  
	void dump(std::ostream &out, int depth=0) const;
	bool contains_unit_name(std::string name) const;

	//private:

	// List of `unit_names` is different names that equal the same thing, e.g. "s", "sec", "second"
	UnitDescription(std::string nice_name, std::vector<UnitInfo> unit_names, bool si_prefixable);
	std::string nice_name;
	std::vector<UnitInfo> unit_info_names;
	bool is_si_prefixable;
};

class UnitGroup {
	public:
	UnitGroup(std::string group_name);
	void add_group(const UnitGroup group);
	void add_unit(const UnitDescription unit);
	bool contains_unit_name(const std::string name) const;
	void dump(std::ostream &out, int depth=0) const;
	std::string to_json(void) const;
	std::string get_group_name(int group_idx) const;
	std::string get_item_json(int group_idx, int item_idx) const;
	int group_count() const;
	int get_item_count(int group_idx) const;


	private:
	std::string group_name;
	std::list<UnitGroup> child_groups;
	std::list<UnitDescription> child_units;
	UnitGroup get_group(int idx) const;
	UnitDescription get_item(int idx) const;
};


UnitGroup get_unit_info(void);

#endif
