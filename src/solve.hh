/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_SOLVE_HH
#define GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_SOLVE_HH 1

#include "model-fwd.hh"
#include "result-fwd.hh"
#include "proof-fwd.hh"

#include <optional>

auto solve(const Model & model, std::optional<Proof> & proof) -> Result;

#endif
