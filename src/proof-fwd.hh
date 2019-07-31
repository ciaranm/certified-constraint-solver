/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_PROOF_FWD_HH
#define CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_PROOF_FWD_HH 1

#include "strong_typedef.hpp"

struct Proof;

using UnderlyingVariableID = jss::strong_typedef<struct UnderlyingVariableIDTag, int,
      jss::strong_typedef_properties::streamable>;

using ProofLineNumber = jss::strong_typedef<struct ProofLineNumberTag, int,
      jss::strong_typedef_properties::comparable,
      jss::strong_typedef_properties::streamable>;

#endif
