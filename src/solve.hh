/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_SOLVE_HH
#define GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_SOLVE_HH 1

#include "model-fwd.hh"
#include "result-fwd.hh"
#include "refutation_log.hh"

auto solve(const Model & model, RefutationLog & ref, int nb_opb_vars, int nb_opb_constraints) -> Result;

#endif
