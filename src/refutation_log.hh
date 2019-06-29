/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_REFUTATION_LOG_HH
#define CERTIFIED_CONSTRAINT_SOLVER_GUARD_SRC_REFUTATION_LOG_HH 1

#include "refutation_log-fwd.hh"

#include <list>
#include <map>

class RefutationLogData
{
    private:
        std::map<std::pair<std::string, int>, int> _vars;
        std::map<std::string, int> _var_takes_at_least_one_value;
        std::list<std::map<std::pair<std::string, int>, int> > _why_nots;
        int _vars_start_at;

    public:
        std::unique_ptr<std::ostream> stream;
        int current_index = 0;

        RefutationLogData(const std::string &);
        ~RefutationLogData();

        auto vars_start_at(int) -> void;
        auto record_var(const std::string &, int) -> void;
        auto inverse_is_at_least_zero(const std::string &, int) -> int;

        auto var_takes_at_least_one_value(const std::string &) const -> int;
        auto record_var_takes_at_least_one_value(const std::string &, int) -> void;

        auto push_why_nots() -> void;
        auto pop_why_nots() -> void;

        auto why_not(const std::string &, int) const -> int;
        auto record_why_not(const std::string &, int, int) -> void;
};

#endif
