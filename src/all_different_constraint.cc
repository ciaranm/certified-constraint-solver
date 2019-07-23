/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "all_different_constraint.hh"
#include "model.hh"
#include "proof.hh"
#include "variable.hh"

#include <iomanip>
#include <ostream>
#include <set>
#include <utility>

using std::endl;
using std::map;
using std::move;
using std::optional;
using std::ostream;
using std::pair;
using std::set;
using std::string;
using std::tuple;
using std::vector;

AllDifferentConstraint::AllDifferentConstraint(vector<VariableID> && v) :
    _vars(move(v))
{
}

AllDifferentConstraint::~AllDifferentConstraint() = default;

auto AllDifferentConstraint::propagate(Model & model, optional<Proof> & proof, set<VariableID> & changed_vars) const -> bool
{
    // first, unit propagate
    for (bool changed = true ; changed ; ) {
        changed = false;
        for (unsigned i = 0 ; i < _vars.size() ; ++i) {
            for (unsigned j = 0 ; j < _vars.size() ; ++j) {
                if (i == j)
                    continue;

                auto i_var = model.get_variable(_vars[i]);
                auto & i_values = i_var->values;
                auto j_var = model.get_variable(_vars[j]);
                auto & j_values = j_var->values;

                if (1 != i_values.size())
                   continue;

                auto j_cannot_be = *i_values.begin();
                if (! j_values.count(j_cannot_be))
                    continue;

                j_values.erase(j_cannot_be);
                changed_vars.insert(_vars[j]);

                changed = true;
                if (proof) {
                    // i_var must take a single value j_cannot_be. we know i_var must take
                    // at least one value...
                    set<int> conflicts;
                    // and we know it cannot take any of the other values...
                    for (auto & v : *i_var->original_values)
                        if (v != j_cannot_be)
                            conflicts.insert(proof->line_for_var_not_equal_value(_vars[i], v));

                    // now j cannot take the value j_cannot_be
                    proof->proof_stream() << "* all different" << endl;

                    // establish what i is
                    proof->proof_stream() << "p " << proof->line_for_var_takes_at_least_one_value(_vars[i]);
                    for (auto & c : conflicts)
                        proof->proof_stream() << " " << c << " +";
                    proof->proof_stream() << " " << (conflicts.size() + 1) << " d ";

                    // add in the at-most-one constraint, and remove the
                    // irrelevant parts
                    proof->proof_stream() << _constraint_numbers.find(j_cannot_be)->second << " +";

                    for (unsigned k = 0 ; k < _vars.size() ; ++k) {
                        if (k != i && k != j)
                            if (model.get_variable(_vars[k])->original_values->count(j_cannot_be))
                                proof->proof_stream() << " " << proof->line_for_var_val_is_at_least_zero(_vars[k], j_cannot_be) << " +";
                    }

                    proof->proof_stream() << " 2 d 0" << endl;
                    proof->next_proof_line();
                    proof->proved_var_not_equal_value(_vars[j], j_cannot_be, proof->last_proof_line());
                }

                if (j_values.empty()) {
                    if (proof) {
                        proof->proof_stream() << "* got domain wipeout on all_different" << endl;
                        proof->proof_stream() << "p " << proof->line_for_var_takes_at_least_one_value(_vars[j]);
                        for (auto & v : *model.get_variable(_vars[j])->original_values)
                            proof->proof_stream() << " " << proof->line_for_var_not_equal_value(_vars[j], v) << " +";
                        proof->proof_stream() << " " << (model.get_variable(_vars[j])->original_values->size() + 1) << " d 0" << endl;
                        proof->next_proof_line();
                    }
                    return false;
                }
            }
        }
    }

    // now find a matching to check feasibility
    set<VariableID> lhs{ _vars.begin(), _vars.end() };
    set<VariableValue> rhs;
    set<pair<VariableID, VariableValue> > edges;

    for (auto & v : _vars) {
        auto & values = model.get_variable(v)->values;
        for (auto w : values) {
            rhs.emplace(w);
            edges.emplace(pair{ v, w });
        }
    }

    set<VariableID> left_covered;
    set<VariableValue> right_covered;
    set<pair<VariableID, VariableValue> > matching;

    // find a greedy matching
    for (auto & e : edges) {
        if ((! left_covered.count(e.first)) && (! right_covered.count(e.second))) {
            left_covered.insert(e.first);
            right_covered.insert(e.second);
            matching.insert(e);
        }
    }

    // now augment
    while (true) {
        set<VariableID> reached_on_the_left;
        set<VariableValue> reached_on_the_right;

        map<VariableValue, VariableID> how_we_got_to_on_the_right;
        map<VariableID, VariableValue> how_we_got_to_on_the_left;

        // start from exposed variables
        set_difference(lhs.begin(), lhs.end(), left_covered.begin(), left_covered.end(),
                inserter(reached_on_the_left, reached_on_the_left.begin()));

        bool still_searching = true, found_a_path = false;
        VariableValue path_endpoint;
        while (still_searching && ! found_a_path) {
            still_searching = false;

            // for each potential left-to-right edge that is not in the matching,
            // that starts with something on the left...
            for (auto & [ var, val ] : edges) {
                if (reached_on_the_left.count(var) && ! matching.count(pair{ var, val })) {
                    // we've found something we can reach on the right
                    if (reached_on_the_right.insert(val).second) {
                        how_we_got_to_on_the_right.insert(pair{ val, var });
                        // is it exposed?
                        if (! right_covered.count(val)) {
                            found_a_path = true;
                            path_endpoint = val;
                            break;
                        }
                        else {
                            still_searching = true;
                        }
                    }
                }
            }

            // if we've not grown our right set, or if we've already found a
            // path, we're done
            if (found_a_path || ! still_searching)
                break;
            still_searching = false;

            // now, for each potential right-to-left edge that is in the matching,
            // that starts with something we've reached on the right...
            for (auto & [ var, val ] : edges) {
                if (reached_on_the_right.count(val) && matching.count(pair{ var, val })) {
                    // we've found something we can reach on the left
                    if (reached_on_the_left.insert(var).second) {
                        how_we_got_to_on_the_left.insert(pair{ var, val });
                        still_searching = true;
                    }
                }
            }
        }

        if (found_a_path) {
            // reconstruct the augmenting path, going backwards
            while (true) {
                // find how we got to the thing on the right...
                auto how_right = how_we_got_to_on_the_right.find(path_endpoint);

                // is the thing on the left exposed?
                if (! left_covered.count(how_right->second)) {
                    left_covered.insert(how_right->second);
                    matching.insert(pair{ how_right->second, path_endpoint });
                    break;
                }
                else {
                    // nope, we must have reached this from the right
                    auto how_left = how_we_got_to_on_the_left.find(how_right->second);
                    matching.erase(pair{ how_right->second, how_left->second });
                    matching.insert(pair{ how_right->second, path_endpoint });

                    path_endpoint = how_left->second;
                }
            }
        }
        else
            break;
    }

    // is our matching big enough?
    if (left_covered.size() == lhs.size())
        return true;

    map<VariableValue, VariableID> inverse_matching;
    for (auto & [ l, r ] : matching)
        inverse_matching.emplace(r, l);

    // we've got a maximum cardinality matching that leaves at least one thing
    // on the left uncovered.
    if (proof) {
        set<VariableID> hall_variables;
        set<VariableValue> hall_values;

        // there must be at least one thing uncovered
        for (auto & v : lhs)
            if (! left_covered.count(v)) {
                hall_variables.insert(v);
                break;
            }

        while (true) {
            // either we have a hall violator, or we have a spare value on the
            // right
            set<VariableValue> n_of_hall_variables;
            for (auto & [ l, r ] : edges)
                if (hall_variables.count(l))
                    n_of_hall_variables.insert(r);

            bool is_subset = true;
            VariableValue not_subset_witness;
            for (auto & v : n_of_hall_variables)
                if (! hall_values.count(v)) {
                    is_subset = false;
                    not_subset_witness = v;
                    break;
                }

            // have we found a hall violator?
            if (is_subset)
                break;

            if (! right_covered.count(not_subset_witness))
                throw "oops";

            // not_subset_witness must be matched to something not yet in
            // hall_variables
            VariableID add_to_hall_variable = inverse_matching.find(not_subset_witness)->second;
            hall_variables.insert(add_to_hall_variable);
            hall_values.insert(not_subset_witness);
        }

        proof->proof_stream() << "* found a hall violator" << endl;
        proof->proof_stream() << "* variables:";
        for (auto & v : hall_variables)
            proof->proof_stream() << " " << model.original_name(v);
        proof->proof_stream() << endl;
        proof->proof_stream() << "* values:";
        for (auto & v : hall_values)
            proof->proof_stream() << " " << int{ v };
        proof->proof_stream() << endl;

        // each variable in the violator has to take at least one value that is
        // left in its domain...
        proof->proof_stream() << "* all different, failed matching, lhs" << endl;
        map<VariableID, int> left_proofs;
        for (auto & v : hall_variables) {
            auto var = model.get_variable(v);

            proof->proof_stream() << "p " << proof->line_for_var_takes_at_least_one_value(v);
            for (auto & w : *var->original_values)
                if (! var->values.count(w))
                    proof->proof_stream() << " " << proof->line_for_var_not_equal_value(v, w) << " +";
            proof->proof_stream() << " 0" << endl;
            proof->next_proof_line();
            proof->proof_stream() << "p " << proof->last_proof_line() << " " << (var->original_values->size() + 1) << " d 0" << endl;
            proof->next_proof_line();
            left_proofs.emplace(v, proof->last_proof_line());
        }

        proof->proof_stream() << "p 0";
        for (auto & [ _, l] : left_proofs)
            proof->proof_stream() << " " << l << " +";
        proof->proof_stream() << " 0" << endl;
        proof->next_proof_line();
        auto lhs = proof->last_proof_line();

        // each value in the component can only be used once
        proof->proof_stream() << "* all different, failed matching, rhs" << endl;
        proof->proof_stream() << "p 0 ";
        for (auto & v : hall_values) {
            proof->proof_stream() << _constraint_numbers.find(v)->second << " + ";
        }

        // cancel out anything not in the hall set, either because it has been
        // eliminated already, or because the variable isn't involved
        for (auto & v : _vars) {
            auto var = model.get_variable(v);
            for (auto & w : *var->original_values)
                if (hall_values.count(w) && ((! hall_variables.count(v)) || ! var->values.count(w)))
                    proof->proof_stream() << " " << proof->line_for_var_val_is_at_least_zero(v, w) << " +";
        }
        proof->proof_stream() << " 0" << endl;
        proof->next_proof_line();

        // bring it together
        proof->proof_stream() << "* all different, failed matching, bringing it together" << endl;
        proof->proof_stream() << "p " << lhs << " " << proof->last_proof_line() << " + " << (left_proofs.size() + 1) << " d 0" << endl;
        proof->next_proof_line();
    }

    return false;
}

auto AllDifferentConstraint::start_proof(const Model & model, Proof & proof) -> void
{
    proof.model_stream() << "* all different" << endl;

    set<VariableValue> all_values;
    for (unsigned i = 0 ; i < _vars.size() ; ++i) {
        auto v = model.get_variable(_vars[i]);
        all_values.insert(v->values.begin(), v->values.end());
    }

    // each value must be unused by all but one variable that can take it
    for (auto & k : all_values) {
        for (unsigned i = 0 ; i < _vars.size() ; ++i) {
            auto v = model.get_variable(_vars[i]);
            if (v->values.count(k))
                proof.model_stream() << "-1 x" << proof.variable_value_mapping(_vars[i], k) << " ";
        }
        proof.model_stream() << ">= -1 ;" << endl;
        proof.next_model_line();
        _constraint_numbers.emplace(k, proof.last_model_line());
    }
}

auto AllDifferentConstraint::associated_variables() const -> set<VariableID>
{
    set<VariableID> result{ _vars.begin(), _vars.end() };
    return result;
}

