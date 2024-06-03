
#include <iostream>

#include "calc_json.h"

int main(void) {
	const int group_count = alexcalc_get_unit_info_group_count();
	for (int group_idx=0; group_idx<group_count; group_idx++) {
		const int item_count = alexcalc_get_unit_info_group_item_count(group_idx);

		char group_name[512];
		alexcalc_get_unit_info_json_group_name(group_name, sizeof(group_name), group_idx);
		group_name[sizeof(group_name)-1] = '\0';
		std::string group_name_str(group_name);
		std::cout << "############# " << std::endl;
		std::cout << "### Group: \"" << group_name_str << "\"" << std::endl;
		std::cout << "### has items: " << item_count << std::endl;
		std::cout << "############# " << std::endl;
		std::cout << std::endl;
		for (int item_idx=0; item_idx<item_count; item_idx++) {
			char item_info[2048];
			alexcalc_get_unit_info_json_item(item_info, sizeof(item_info), group_idx, item_idx);
			item_info[sizeof(item_info)-1] = '\0';
			std::string item_info_str(item_info);
			std::cout << item_idx << ": " << item_info << std::endl;
		}
		std::cout << std::endl;
		std::cout << std::endl;
	}
}
