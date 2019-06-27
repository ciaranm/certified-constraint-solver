/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_REFUTATION_LOG_FWD_HH
#define CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_REFUTATION_LOG_FWD_HH 1

#include <iosfwd>
#include <memory>

struct RefutationLogData;

using RefutationLog = std::unique_ptr<RefutationLogData>;

#endif
