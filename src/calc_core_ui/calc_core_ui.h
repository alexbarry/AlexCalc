
#pragma once 

#include <list>
#include <vector>

#include "calc_core.h"

class CalcUiState {
	public:
	/**
	 * Called both:
	 *    - when enter is pressed (and all units in the input become
	 *      "recently used units", and
	 *    - when the "insert unit" button is pressed?
	 *      (Perhaps I'll change this now that I'm adding "input tokens" to
	 *      the CPP layer)
	 */
	// Iterable can either be a list or vector of UnitInfoInputAry
	template<typename Iterable>
	void update_recently_used_units(Iterable units);
	void update_recently_used_units(UnitInfoInputAry unit);

	void get_recently_used_units(std::list<UnitInfoInputAry> *units_out);
	void delete_recently_used_units(void);

	private:
	std::list<UnitInfoInputAry> recently_used_units;
};
