/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "solve.hh"
#include "model.hh"
#include "constraint.hh"
#include "proof.hh"
#include "result.hh"
#include "variable.hh"

#include <iomanip>
#include <list>
#include <set>
#include <utility>

using std::endl;
using std::list;
using std::optional;
using std::pair;
using std::set;
using std::string;
using std::to_string;

auto search(int depth, Result & result, const Model & start_model, optional<Proof> & proof, list<pair<VariableID, VariableValue> > & stack) -> void
{
    ++result.nodes;
    auto model = start_model;

    if (proof) {
        proof->proof_stream() << "* propagation at depth " << depth << endl;
    }

    if (! model.propagate(proof)) {
        if (proof) {
            proof->proof_stream() << "* propagation detected inconsistency at depth " << depth << endl;
            if (proof->asserty())
                proof->assert_what_we_just_did("propagation failure");
        }
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
                stack.push_back(pair{ branch_variable_name, v });

                proof->proof_stream() << "* guessing";
                for (auto & [ s, t ] : stack)
                    proof->proof_stream() << " " << model.original_name(s) << "=" << int{ t }
                        << " (" << "x" + to_string(proof->variable_value_mapping(s, t)) << ")";
                proof->proof_stream() << endl;

                proof->push_context();

                // generate conflict clauses for everything that has already been removed from the domain
                for (auto & w : *branch_variable->original_values)
                    if (w != v) {
                        proof->proof_stream() << "p " << proof->line_for_var_takes_at_most_one_value(branch_variable_name);
                        for (auto & x : *branch_variable->original_values)
                            if (x != v && x != w)
                                proof->proof_stream() << " " << proof->line_for_var_val_is_at_least_zero(branch_variable_name, x) << " +";
                        proof->proof_stream() << " 0" << endl;
                        proof->next_proof_line();
                        proof->proved_var_not_equal_value(branch_variable_name, w, proof->last_proof_line());

                        if (proof->asserty())
                            proof->assert_we_proved_var_not_equal_value(branch_variable_name, w, "recalling previous assertion");
                    }
            }

            search(depth + 1, result, model, proof, stack);

            if (! result.solution.empty())
                return;

            if (proof) {
                stack.pop_back();
                proof->proof_stream() << "* got a conflict at depth " << depth << endl;

                proof->proof_stream() << "p " << proof->last_proof_line() << " " << proof->line_for_var_val_is_at_most_one(branch_variable_name, v) << " + 2 d 0" << endl;
                proof->next_proof_line();

                conflicts.insert(proof->last_proof_line());

                if (proof->asserty())
                    proof->assert_we_proved_var_not_equal_value(branch_variable_name, v, "following a conflict");
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

            if (proof->asserty())
                proof->assert_what_we_just_did("after a domain wipeout");
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

    list<pair<VariableID, VariableValue> > stack;
    if (proof)
        proof->set_active_stack(&stack);

    search(0, result, model, proof, stack);

    if (proof && result.solution.empty()) {
        proof->proof_stream() << "c " << proof->last_proof_line() << " 0" << endl;
        proof->next_proof_line();
    }

    return result;
}

