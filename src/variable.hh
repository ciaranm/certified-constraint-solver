/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_VARIABLE_HH
#define GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_VARIABLE_HH 1

#include <set>

struct Variable
{
    Variable(int lw, int ub);
    Variable(const Variable &);
    ~Variable();

    std::set<int> values;
};

#endif
