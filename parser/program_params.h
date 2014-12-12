#ifndef PROGRAM_PARAMS_H
#define PROGRAM_PARAMS_H

#include <parser/params/bc_params.h>
#include <parser/params/k_opt_params.h>
#include <parser/params/subgradient_params.h>

struct ProgramParams {
    KOptParams          ko;
    SubgradientParams   sg;
    BranchAndCutParams  bc;
    
    ProgramParams(KOptParams ko, SubgradientParams sg, BranchAndCutParams bc) : ko{std::move(ko)}, sg{std::move(sg)}, bc{std::move(bc)} {}
};

#endif