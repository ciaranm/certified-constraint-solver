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

auto search(int depth, Result & result, const Model & start_model, optional<Proof> & proof) -> void
{
    ++result.nodes;
    auto model = start_model;

    if (proof)
        proof->proof_stream() << "* propagation at depth " << depth << endl;

    if (! model.propagate(proof)) {
        if (proof)
            proof->proof_stream() << "* propagation detected inconsistency at depth " << depth << endl;

        return;
    }

    auto [ branch_variable_name, branch_variable ] = model.select_branch_variable();
    if (branch_variable) {
        if (proof)
            proof->proof_stream() << "* branching at depth " << depth << endl;

        auto possible_values = branch_variable->values;
        branch_variable->values.clear();
        for (auto & v : possible_values) {
            branch_variable->values = {{ v }};

            if (proof)
                proof->enstackinate_guess(branch_variable_name, model.original_name(branch_variable_name), v);

            search(depth + 1, result, model, proof);

            if (! result.solution.empty())
                return;

            if (proof)
                proof->incorrect_guess();
        }

        if (proof) {
            proof->proof_stream() << "* domain wipeout at depth " << depth << endl;
            proof->domain_wipeout(branch_variable_name, *branch_variable);
        }
    }
    else
        model.save_result(result);
};

auto solve(const Model & model, optional<Proof> & proof) -> Result
{
    if (proof) {
        model.start_proof(*proof);
    }

    Result result;

    search(0, result, model, proof);

    if (proof && result.solution.empty()) {
        proof->proof_stream() << "u opb >= 1 ;" << endl;
        proof->next_proof_line();
        proof->proof_stream() << "c " << proof->last_proof_line() << " 0" << endl;
    }

    return result;
}

