/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "solve.hh"
#include "model.hh"
#include "constraint.hh"
#include "proof.hh"
#include "result.hh"
#include "variable.hh"

#include <iomanip>
#include <set>

using std::endl;
using std::optional;
using std::set;
using std::string;

auto search(int depth, Result & result, const Model & start_model, optional<Proof> & proof) -> void
{
    ++result.nodes;
    auto model = start_model;

    if (proof) {
        proof->proof_stream() << "* propagation at depth " << depth << endl;
    }

    if (! model.propagate(proof)) {
        if (proof)
            proof->proof_stream() << "* propagation detected inconsistency at depth " << depth << endl;
        return;
    }

    auto [ branch_variable_name, branch_variable ] = model.select_branch_variable();
    if (branch_variable) {
        if (proof)
            proof->proof_stream() << "* branching at depth " << depth << endl;

        set<int> conflicts;
        auto possible_values = branch_variable->values;
        branch_variable->values.clear();
        for (auto & v : possible_values) {
            branch_variable->values = {{ v }};

            if (proof) {
                proof->proof_stream() << "* guessing x" << proof->variable_value_mapping(branch_variable_name, v) << endl;
                proof->push_context();

                for (auto & w : *branch_variable->original_values)
                    if (w != v) {
                        proof->proof_stream() << "p " << proof->line_for_var_takes_at_most_one_value(branch_variable_name);
                        for (auto & x : *branch_variable->original_values)
                            if (x != v && x != w)
                                proof->proof_stream() << " " << proof->line_for_var_val_is_at_least_zero(branch_variable_name, x) << " +";
                        proof->proof_stream() << " 0" << endl;
                        proof->next_proof_line();
                        proof->proved_var_not_equal_value(branch_variable_name, w, proof->last_proof_line());
                    }
            }

            search(depth + 1, result, model, proof);

            if (! result.solution.empty())
                return;

            if (proof) {
                proof->proof_stream() << "* got a conflict at depth " << depth << endl;
                conflicts.insert(proof->last_proof_line());
                proof->pop_context();
            }
        }

        if (proof) {
            for (auto & v : *branch_variable->original_values) {
                if (possible_values.count(v))
                    continue;
                conflicts.insert(proof->line_for_var_not_equal_value(branch_variable_name, v));
            }

            proof->proof_stream() << "* domain wipeout at depth " << depth << endl;
            proof->proof_stream() << "p " << proof->line_for_var_takes_at_least_one_value(branch_variable_name);
            for (auto & c : conflicts)
                proof->proof_stream() << " " << c << " +";
            proof->proof_stream() << " " << (conflicts.size() + 1) << " d 0" << endl;
            proof->next_proof_line();
        }
    }
    else
        model.save_result(result);
};

auto solve(const Model & model, optional<Proof> & proof) -> Result
{
    if (proof) {
        model.start_proof(*proof);
        proof->push_context();
    }

    Result result;
    search(0, result, model, proof);

    if (proof && result.solution.empty()) {
        proof->proof_stream() << "c " << proof->last_proof_line() << " 0" << endl;
        proof->next_proof_line();
    }

    return result;
}

