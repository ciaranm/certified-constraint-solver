/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "solve.hh"
#include "model.hh"
#include "constraint.hh"
#include "result.hh"
#include "variable.hh"

using std::endl;
using std::string;

auto search(int depth, Result & result, const Model & start_model, RefutationLog & log) -> void
{
    ++result.nodes;
    auto model = start_model;

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
        if (! log) {
            auto possible_values = branch_variable->values;
            branch_variable->values.clear();
            for (auto & v : possible_values) {
                branch_variable->values = {{ v }};
                search(depth + 1, result, model, log);
                if (! result.solution.empty())
                    return;
            }
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
    }

    Result result;
    search(0, result, model, log);

    if (log && result.solution.empty()) {
        *log->stream << "c " << log->current_index << " 0" << endl;
        ++log->current_index;
    }

    return result;
}

