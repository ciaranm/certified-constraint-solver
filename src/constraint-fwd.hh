/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_CONSTRAINT_FWD_HH
#define GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_CONSTRAINT_FWD_HH 1

struct Constraint;

enum class PropagationResult
{
    Consistent,
    NoChange,
    Inconsistent
};

#endif