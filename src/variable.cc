/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "variable.hh"

Variable::Variable(int lw, int ub)
{
    for ( ; lw <= ub ; ++lw)
        values.insert(lw);
}

Variable::~Variable() = default;

Variable::Variable(const Variable &) = default;

