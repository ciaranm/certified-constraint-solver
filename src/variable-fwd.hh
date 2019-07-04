/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_VARIABLE_FWD_HH
#define GLASGOW_CONSTRAINT_SOLVER_GUARD_SRC_VARIABLE_FWD_HH 1

#include "strong_typedef.hpp"

struct Variable;

using VariableID = jss::strong_typedef<struct VariableIDTag, int,
      jss::strong_typedef_properties::comparable,
      jss::strong_typedef_properties::hashable>;

using VariableValue = jss::strong_typedef<struct VariableValueTag, int,
      jss::strong_typedef_properties::comparable,
      jss::strong_typedef_properties::hashable>;

#endif
