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
    return _why_not.find(pair{ s, v })->second;
}

auto RefutationLogData::record_why_not(const string & s, int v, int c) -> void
{
    _why_not.emplace(pair{ s, v }, c);
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

