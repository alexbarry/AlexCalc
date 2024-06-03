#include<unordered_map>

#include "calc_core.h"
#include "calc_core_ui.h"

// It really sucks that there is no base class for a container and that
// I have to use templates for something as simple as this
template<typename Iterable>
void CalcUiState::update_recently_used_units(Iterable units) {
	// the recently units should always refer to the most recent units first.
	// The argument contains the most recently used units, in that order.
	// So the new "recent units" list should start with the argument `units`.
	// Then add any non duplicate elements from the old `recently_used_units`.
	std::unordered_map<std::string, bool> seen_units;

	std::list<UnitInfoInputAry> old_recent_units = this->recently_used_units;
	this->recently_used_units.clear();


	for (UnitInfoInputAry unit : units) {
		if (seen_units.find(unit.to_string()) == seen_units.end()) {
			this->recently_used_units.push_back(unit);
		}
		seen_units[unit.to_string()] = true;
	}

	for (UnitInfoInputAry unit : old_recent_units) {
		if (seen_units.find(unit.to_string()) == seen_units.end()) {
			this->recently_used_units.push_back(unit);
		}
		seen_units[unit.to_string()] = true;
	}
}
template void CalcUiState::update_recently_used_units<std::list<UnitInfoInputAry>>(std::list<UnitInfoInputAry>);
template void CalcUiState::update_recently_used_units<std::vector<UnitInfoInputAry>>(std::vector<UnitInfoInputAry>);

void CalcUiState::get_recently_used_units(std::list<UnitInfoInputAry> *units_out) {
	// Is there a better way to transfer contents from one list to another?
	*units_out = this->recently_used_units;
}

void CalcUiState::delete_recently_used_units() {
	this->recently_used_units.clear();
}
