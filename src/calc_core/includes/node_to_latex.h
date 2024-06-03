
#ifndef NODE_TO_LATEX_H_
#define NODE_TO_LATEX_H_

#include<string>
#include "calc_core.h"
#include "calc_core_types.h"

std::string node_to_latex(const InputInfo *parse_info);
std::string val_to_latex(const val_t *val_arg,
                         const struct calc_fmt_params &params,
                         const CalcData *calcData,
                         const std::vector<UnitInfoInput> *desired_units,
                         const unit_t                     *desired_units_val);
#endif
