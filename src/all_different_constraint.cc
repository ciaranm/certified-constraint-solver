/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "all_different_constraint.hh"
#include "model.hh"
#include "proof.hh"
#include "variable.hh"

#include <algorithm>
#include <functional>
#include <iomanip>
#include <list>
#include <ostream>
#include <set>
#include <type_traits>
#include <utility>
#include <variant>

using std::decay_t;
using std::endl;
using std::function;
using std::is_same_v;
using std::list;
using std::map;
using std::min;
using std::move;
using std::optional;
using std::ostream;
using std::pair;
using std::set;
using std::string;
using std::tuple;
using std::variant;
using std::vector;
using std::visit;

AllDifferentConstraint::AllDifferentConstraint(vector<VariableID> && v) :
    _vars(move(v))
{
}

AllDifferentConstraint::~AllDifferentConstraint() = default;

auto build_matching(
        const set<pair<VariableID, VariableValue> > & edges,
        const set<VariableID> & lhs,
        set<VariableID> & left_covered,
        set<VariableValue> & right_covered,
        set<pair<VariableID, VariableValue> > & matching
        ) -> void
{
    // start with a greedy matching
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
            // we've included another value
            right_covered.insert(path_endpoint);

            // reconstruct the augmenting path to figure out how we did it,
            // going backwards
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
}

auto AllDifferentConstraint::_prove_matching_is_too_small(
        Model & model,
        Proof & proof,
        const set<pair<VariableID, VariableValue> > & edges,
        const set<VariableID> & lhs,
        const set<VariableID> & left_covered,
        const set<pair<VariableID, VariableValue> > & matching
        ) const -> void
{
    map<VariableValue, VariableID> inverse_matching;
    for (auto & [ l, r ] : matching)
        inverse_matching.emplace(r, l);

    set<VariableID> hall_variables;
    set<VariableValue> hall_values;

    // there must be at least one thing uncovered, and this will
    // necessarily participate in a hall violator
    for (auto & v : lhs)
        if (! left_covered.count(v)) {
            hall_variables.insert(v);
            break;
        }

    // either we have found a hall violator, or we have a spare value
    // on the right
    while (true) {
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

        // not_subset_witness must be matched to something not yet in
        // hall_variables
        VariableID add_to_hall_variable = inverse_matching.find(not_subset_witness)->second;
        hall_variables.insert(add_to_hall_variable);
        hall_values.insert(not_subset_witness);
    }

    proof.proof_stream() << "* found a hall violator" << endl;

    // each variable in the violator has to take at least one value that is
    // left in its domain...
    map<VariableID, int> left_proofs;
    for (auto & v : hall_variables) {
        auto var = model.get_variable(v);

        proof.proof_stream() << "p " << proof.line_for_var_takes_at_least_one_value(v);
        for (auto & w : *var->original_values)
            if (! var->values.count(w))
                proof.proof_stream() << " " << proof.line_for_var_not_equal_value(v, w) << " +";
        proof.proof_stream() << " " << (var->original_values->size() + 1) << " d 0" << endl;
        proof.next_proof_line();
        left_proofs.emplace(v, proof.last_proof_line());
    }

    // each value in the component can only be used once
    proof.proof_stream() << "p 0 ";
    for (auto & v : hall_values) {
        proof.proof_stream() << _constraint_numbers.find(v)->second << " + ";
    }

    // cancel out anything not in the hall set, either because it has been
    // eliminated already, or because the variable isn't involved
    for (auto & v : _vars) {
        auto var = model.get_variable(v);
        for (auto & w : *var->original_values)
            if (hall_values.count(w) && ((! hall_variables.count(v)) || ! var->values.count(w)))
                proof.proof_stream() << " " << proof.line_for_var_val_is_at_least_zero(v, w) << " +";
    }

    for (auto & [ _, l] : left_proofs)
        proof.proof_stream() << " " << l << " +";

    proof.proof_stream() << " " << (left_proofs.size() + 1) << " d 0" << endl;
    proof.next_proof_line();
}

auto AllDifferentConstraint::propagate(Model & model, optional<Proof> & proof, set<VariableID> & changed_vars) const -> bool
{
    // find a matching to check feasibility
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

    build_matching(edges, lhs, left_covered, right_covered, matching);

    // is our matching big enough?
    if (left_covered.size() != lhs.size()) {
        // nope. we've got a maximum cardinality matching that leaves at least
        // one thing on the left uncovered. possibly output a proof, then
        // return indicating a contradiction.
        if (proof)
            _prove_matching_is_too_small(model, *proof, edges, lhs, left_covered, matching);

        return false;
    }
    else {
        // we have a matching that uses every variable. however, some edges may
        // not occur in any maximum cardinality matching, and we can delete
        // these. first we need to build the directed matching graph...
        using Vertex = variant<VariableID, VariableValue>;
        map<Vertex, list<Vertex> > edges_out_from;
        map<VariableID, list<VariableValue> > edges_out_from_variable, edges_in_to_variable;
        map<VariableValue, list<VariableID> > edges_out_from_value, edges_in_to_value;

        for (auto & [ f, t ] : edges)
            if (matching.count(pair{ f, t })) {
                edges_out_from[t].push_back(f);
                edges_out_from_value[t].push_back(f);
                edges_in_to_variable[f].push_back(t);
            }
            else {
                edges_out_from[f].push_back(t);
                edges_out_from_variable[f].push_back(t);
                edges_in_to_value[t].push_back(f);
            }

        // now we need to find strongly connected components...
        map<Vertex, int> indices, lowlinks, components;
        list<Vertex> stack;
        set<Vertex> enstackinated;
        set<Vertex> all_vertices;
        int next_index = 0, number_of_components = 0;

        for (auto & v : _vars) {
            all_vertices.emplace(v);
            for (auto & w : model.get_variable(v)->values)
                all_vertices.emplace(w);
        }

        function<auto (Vertex) -> void> scc;
        scc = [&] (Vertex v) -> void {
            indices.emplace(v, next_index);
            lowlinks.emplace(v, next_index);
            ++next_index;
            stack.emplace_back(v);
            enstackinated.emplace(v);

            for (auto & w : edges_out_from[v]) {
                if (! indices.count(w)) {
                    scc(w);
                    lowlinks[v] = min(lowlinks[v], lowlinks[w]);
                }
                else if (enstackinated.count(w)) {
                    lowlinks[v] = min(lowlinks[v], lowlinks[w]);
                }
            }

            if (lowlinks[v] == indices[v]) {
                Vertex w;
                do {
                    w = stack.back();
                    stack.pop_back();
                    enstackinated.erase(w);
                    components.emplace(w, number_of_components);
                } while (v != w);
                ++number_of_components;
            }
        };

        for (auto & v : all_vertices)
            if (! indices.count(v))
                scc(v);

        // every edge in the original matching is used, and so cannot be
        // deleted
        auto used_edges = matching;

        // for each unmatched vertex, bring in everything that could be updated
        // to take it
        {
            set<Vertex> to_explore{ rhs.begin(), rhs.end() }, explored;
            for (auto & [ _, t ] : matching)
                to_explore.erase(t);

            while (! to_explore.empty()) {
                Vertex v = *to_explore.begin();
                to_explore.erase(v);
                explored.insert(v);

                visit([&] (const auto & x) {
                        if constexpr (is_same_v<decay_t<decltype(x)>, VariableID>) {
                            for (auto & t : edges_in_to_variable[x]) {
                                used_edges.emplace(x, t);
                                if (! explored.count(t))
                                    to_explore.insert(t);
                            }
                        }
                        else {
                            for (auto & t : edges_in_to_value[x]) {
                                used_edges.emplace(t, x);
                                if (! explored.count(t))
                                    to_explore.insert(t);
                            }
                        }
                        }, v);
            }
        }

        // every edge that starts and ends in the same component is also used
        for (auto & [ f, t ] : edges)
            if (components.find(f)->second == components.find(t)->second)
                used_edges.emplace(f, t);

        // anything left can be deleted
        for (auto & [ delete_var_name, delete_value ] : edges) {
            if (used_edges.count(pair{ delete_var_name, delete_value }))
                continue;

            auto delete_var = model.get_variable(delete_var_name);
            if (delete_var->values.count(delete_value)) {
                if (proof) {
                    // we know a hall set exists, but we have to find it. starting
                    // from but not including the end of the edge we're deleting,
                    // everything reachable forms a hall set.
                    set<Vertex> to_explore, explored;
                    set<VariableID> hall_left;
                    set<VariableValue> hall_right;
                    to_explore.insert(delete_value);
                    while (! to_explore.empty()) {
                        Vertex n = *to_explore.begin();
                        to_explore.erase(n);
                        explored.insert(n);

                        visit([&] (const auto & x) -> void {
                            if constexpr (is_same_v<decay_t<decltype(x)>, VariableID>) {
                                hall_left.emplace(x);
                                for (auto & t : edges_out_from_variable[x]) {
                                    if (! explored.count(t))
                                        to_explore.insert(t);
                                }
                            }
                            else {
                                hall_right.emplace(x);
                                for (auto & t : edges_out_from_value[x]) {
                                    if (! explored.count(t))
                                        to_explore.insert(t);
                                }
                            }
                        }, n);
                    }

                    proof->proof_stream() << "* all different, found hall set, "
                        << model.original_name(delete_var_name) << " not equal " << int{ delete_value } << endl;

                    proof->proof_stream() << "p 0";
                    for (auto & h : hall_left)
                        proof->proof_stream() << " " << proof->line_for_var_takes_at_least_one_value(h) << " +";
                    for (auto & w : hall_right)
                        proof->proof_stream() << " " << _constraint_numbers.find(w)->second << " +";

                    int cancel_count = 0;

                    // non-deletion, non-hall variables could have values in
                    // the hall set
                    for (auto & v : _vars) {
                        if (v != delete_var_name && ! hall_left.count(v)) {
                            auto var = model.get_variable(v);
                            for (auto & w : *var->original_values)
                                if (hall_right.count(w))
                                    proof->proof_stream() << " " << proof->line_for_var_val_is_at_least_zero(v, w) << " +";
                        }
                    }

                    // we might also be showing that the deletion variable
                    // cannot take the other values inside the hall set either,
                    // which is true but irrelevant for this particular
                    // deduction
                    for (auto & r : hall_right)
                        if (r != delete_value && delete_var->original_values->count(r))
                            proof->proof_stream() << " " << proof->line_for_var_val_is_at_least_zero(delete_var_name, r) << " +";

                    // hall variables could have values not in the hall set,
                    // in which case they have been eliminated previously
                    for (auto & h : hall_left) {
                        auto var = model.get_variable(h);
                        for (auto & v : *var->original_values)
                            if (! hall_right.count(v)) {
                                ++cancel_count;
                                proof->proof_stream() << " " << proof->line_for_var_not_equal_value(h, v) << " +";
                            }
                    }

                    proof->proof_stream() << " " << (cancel_count + 1) << " d 0" << endl;
                    proof->next_proof_line();
                    proof->proved_var_not_equal_value(delete_var_name, delete_value, proof->last_proof_line());
                }

                delete_var->values.erase(delete_value);
                changed_vars.emplace(delete_var_name);

                // have we triggered a domain wipeout? if so, not point in
                // doing the rest of the propagation
                if (delete_var->values.empty()) {
                    if (proof) {
                        proof->proof_stream() << "* all different, domain wipeout" << endl;
                        proof->proof_stream() << "p " << proof->line_for_var_takes_at_least_one_value(delete_var_name);
                        for (auto & v : *delete_var->original_values)
                            proof->proof_stream() << " " << proof->line_for_var_not_equal_value(delete_var_name, v) << " +";
                        proof->proof_stream() << " " << (delete_var->original_values->size() + 1) << " d 0" << endl;
                        proof->next_proof_line();
                    }
                    return false;
                }
            }
        }

        return true;
    }
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

auto AllDifferentConstraint::priority() const -> int
{
    return 2;
}

