/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "variable.hh"
#include "refutation_log.hh"

#include <ostream>
#include <string>

using std::endl;
using std::map;
using std::ostream;
using std::pair;
using std::string;

Variable::Variable(int lw, int ub)
{
    for ( ; lw <= ub ; ++lw)
        values.insert(lw);
    original_values = values;
}

Variable::~Variable() = default;

Variable::Variable(const Variable &) = default;

auto Variable::encode_as_opb(const string & name, ostream & s, map<pair<string, int>, int> & vars_map,
        int & nb_constraints, RefutationLog & log) const -> void
{
    nb_constraints += 2;

    for (auto & v : values) {
        int idx = vars_map.size() + 1;
        vars_map.emplace(pair{ name, v }, idx);
        log->record_var(name, v);
        s << "* x" << idx << " means " << name << " = " << v << endl;
    }

    s << "* variable " << name << " takes exactly one value" << endl;
    for (auto & v : values) {
        auto idx = vars_map.find(pair{ name, v })->second;
        s << "1 " << "x" << idx << " ";
    }
    s << ">= 1 ;" << endl;
    if (log)
        log->record_var_takes_at_least_one_value(name, ++log->current_index);

    for (auto & v : values) {
        auto idx = vars_map.find(pair{ name, v })->second;
        s << "-1 " << "x" << idx << " ";
    }
    s << ">= -1 ;" << endl;

    if (log)
        ++log->current_index;
}

