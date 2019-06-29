/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "refutation_log.hh"

#include <fstream>
#include <memory>

using std::make_unique;
using std::ofstream;
using std::pair;
using std::string;

RefutationLogData::RefutationLogData(const string & s) :
    current_index(0)
{
    stream = make_unique<ofstream>(s);
}

RefutationLogData::~RefutationLogData() = default;

auto RefutationLogData::var_takes_at_least_one_value(const string & s) const -> int
{
    return _var_takes_at_least_one_value.find(s)->second;
}

auto RefutationLogData::record_var_takes_at_least_one_value(const string & s, int v) -> void
{
    _var_takes_at_least_one_value.emplace(s, v);
}

auto RefutationLogData::why_not(const string & s, int v) const -> int
{
    for (auto n = _why_nots.rbegin() ; n != _why_nots.rend() ; ++n) {
        auto w = n->find(pair{ s, v });
        if (w != n->end())
            return w->second;
    }

    return 0;
}

auto RefutationLogData::push_why_nots() -> void
{
    _why_nots.emplace_back();
}

auto RefutationLogData::pop_why_nots() -> void
{
    _why_nots.pop_back();
}

auto RefutationLogData::record_why_not(const string & s, int v, int c) -> void
{
    _why_nots.back().emplace(pair{ s, v }, c);
}

auto RefutationLogData::record_var(const string & n, int v) -> void
{
    _vars.emplace(pair{ n, v }, _vars.size());
}

auto RefutationLogData::inverse_is_at_least_zero(const string & n, int v) -> int
{
    return _vars_start_at + 2 * _vars.find(pair{ n, v })->second + 1;
}

auto RefutationLogData::vars_start_at(int n) -> void
{
    _vars_start_at = n;
}

