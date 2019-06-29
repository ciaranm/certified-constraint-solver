/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "solve.hh"
#include "model.hh"
#include "constraint.hh"
#include "result.hh"
#include "variable.hh"

#include <list>
#include <set>

using std::endl;
using std::list;
using std::set;
using std::string;

auto search(int depth, Result & result, const Model & start_model, RefutationLog & log) -> void
{
    ++result.nodes;
    auto model = start_model;

    if (log) {
        *log->stream << "* propagation at depth " << depth << endl;
    }

    bool modified = true;
    while (modified) {
        modified = false;
        for (auto & c : model.constraints) {
            switch (c->propagate(model, log)) {
                case PropagationResult::Inconsistent: return;
                case PropagationResult::Consistent:   modified = true; break;
                case PropagationResult::NoChange:     break;
            }
        }
    }

    string branch_variable_name;
    if (auto branch_variable = model.select_branch_variable(branch_variable_name)) {
        if (log) {
            *log->stream << "* branching on variable " << branch_variable_name << " at depth " << depth << endl;
        }

        set<int> conflicts;
        auto possible_values = branch_variable->values;
        branch_variable->values.clear();
        for (auto & v : possible_values) {
            branch_variable->values = {{ v }};

            if (log) {
                *log->stream << "* guessing " << branch_variable_name << " = " << v << " at depth " << depth << endl;
                log->push_why_nots();

                for (auto & w : branch_variable->original_values)
                    if (w != v) {
                        log->record_why_not(branch_variable_name, w, log->var_takes_at_least_one_value(branch_variable_name) + 1);
                    }
            }
            search(depth + 1, result, model, log);
            if (! result.solution.empty())
                return;

            if (log) {
                *log->stream << "* got a conflict for " << branch_variable_name << " = " << v << " at depth " << depth << endl;
                conflicts.insert(log->current_index);
                log->pop_why_nots();
            }
        }

        if (log) {
            for (auto & v : branch_variable->original_values) {
                if (possible_values.count(v))
                    continue;
                conflicts.insert(log->why_not(branch_variable_name, v));
            }

            *log->stream << "* domain wipeout on variable " << branch_variable_name << " at depth " << depth << endl;
            *log->stream << "p " << log->var_takes_at_least_one_value(branch_variable_name);
            for (auto & c : conflicts)
                *log->stream << " " << c << " +";
            *log->stream << " " << conflicts.size() << " d 0" << endl;
            ++log->current_index;
        }
    }
    else
        model.save_result(result);
};

auto solve(const Model & model, RefutationLog & log, int nb_opb_vars, int nb_opb_constraints) -> Result
{
    if (log) {
        *log->stream << "refutation using f l p c 0" << endl;
        model.write_ref_header(log, nb_opb_vars, nb_opb_constraints);
        log->push_why_nots();
    }

    Result result;
    search(0, result, model, log);

    if (log && result.solution.empty()) {
        *log->stream << "c " << log->current_index << " 0" << endl;
        ++log->current_index;
    }

    return result;
}

