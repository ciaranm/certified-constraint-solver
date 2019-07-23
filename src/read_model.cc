/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "read_model.hh"
#include "model.hh"
#include "constraint.hh"
#include "table_constraint.hh"
#include "not_equals_constraint.hh"
#include "equals_constant_constraint.hh"
#include "all_different_constraint.hh"
#include "variable.hh"

#include <fstream>
#include <map>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

using std::ifstream;
using std::make_shared;
using std::map;
using std::move;
using std::set;
using std::string;
using std::stringstream;
using std::to_string;
using std::vector;

InputError::InputError(const string & m) noexcept :
    _message("Input file error: " + m)
{
}

auto InputError::what() const noexcept -> const char *
{
    return _message.c_str();
}

auto read_model(const string & filename) -> Model
{
    ifstream infile{ filename };
    if (! infile)
        throw InputError{ "Error reading from '" + filename + "'" };

    string word;

    Model model;
    map<string, std::shared_ptr<Table> > tables;
    map<string, VariableID> variable_name_to_id;

    auto make_name = [&] (const string & n) -> VariableID {
        VariableID id{ int(variable_name_to_id.size()) };
        if (! variable_name_to_id.emplace(n, id).second)
            throw InputError{ "Duplicate variable '" + n + "'" };
        return id;
    };

    auto get_name = [&] (const string & n) -> VariableID {
        auto i = variable_name_to_id.find(n);
        if (i == variable_name_to_id.end())
            throw InputError{ "No variable named '" + n + "'" };
        return i->second;
    };

    while (infile >> word) {
        if (word == "intvar") {
            string name;
            string op;
            if (! (infile >> name >> op))
                throw InputError{ "Bad arguments to '" + word + "' command" };
            if (op == "{") {
                string v;
                set<int> values;
                while (true) {
                    if (! (infile >> v))
                        throw InputError{ "Bad arguments to '" + word + "' command" };
                    if (v == "}")
                        break;
                    stringstream ss{ v };
                    int val;
                    if (! (ss >> val))
                        throw InputError{ "Bad arguments to '" + word + "' command" };
                    values.insert(val);
                }
                if (! model.add_variable(name, make_name(name), make_shared<Variable>(values)))
                    throw InputError{ "Duplicate variable '" + name + "'" };
            }
            else {
                stringstream s{ op };
                int lb, ub;
                if (! (s >> lb) || ! (infile >> ub))
                    throw InputError{ "Bad arguments to '" + word + "' command" };
                if (! model.add_variable(name, make_name(name), make_shared<Variable>(lb, ub)))
                    throw InputError{ "Duplicate variable '" + name + "'" };
            }
        }
        else if (word == "intvararray") {
            string name;
            int dim;
            int s1, e1, s2, e2;
            int lb, ub;
            if (! (infile >> name >> dim))
                throw InputError{ "Bad arguments to '" + word + "' command" };
            if (dim != 2)
                throw InputError{ "Bad arguments to '" + word + "' command: only dimension 2 supported out of sheer laziness" };
            if (! (infile >> s1 >> e1 >> s2 >> e2 >> lb >> ub))
                throw InputError{ "Bad arguments to '" + word + "' command" };
            for (int i = s1 ; i <= e1 ; ++i)
                for (int j = s2 ; j <= e2 ; ++j) {
                    string full_name = name + "[" + to_string(i) + "," + to_string(j) + "]";
                    if (! model.add_variable(full_name, make_name(full_name), make_shared<Variable>(lb, ub)))
                        throw InputError{ "Duplicate variable '" + name + "'" };
                }
        }
        else if (word == "notequal") {
            string first, second;
            if (! (infile >> first >> second))
                throw InputError{ "Bad arguments to '" + word + "' command" };
            model.add_constraint(make_shared<NotEqualConstraint>(get_name(first), get_name(second)));
        }
        else if (word == "equal") {
            string first;
            int second;
            if (! (infile >> first >> second))
                throw InputError{ "Bad arguments to '" + word + "' command" };
            model.add_constraint(make_shared<EqualConstantConstraint>(get_name(first), VariableValue{ second }));
        }
        else if (word == "createtable") {
            string name;
            int arity;
            if (! (infile >> name >> arity))
                throw InputError{ "Bad arguments to '" + word + "' command" };
            if (! tables.emplace(name, make_shared<Table>(arity)).second)
                throw InputError{ "Duplicate table '" + name + "'" };
        }
        else if (word == "addtotable") {
            string name;
            if (! (infile >> name))
                throw InputError{ "Bad arguments to '" + word + "' command" };
            auto table = tables.find(name);
            if (table == tables.end())
                throw InputError{ "No table named '" + name + "'" };

            vector<VariableValue> tuple;
            for (int i = 0 ; i < table->second->arity ; ++i) {
                int value;
                if (! (infile >> value))
                    throw InputError{ "Bad arguments to '" + word + "' command" };
                tuple.push_back(VariableValue{ value });
            }
            table->second->allowed_tuples.push_back(move(tuple));
        }
        else if (word == "table") {
            string name;
            if (! (infile >> name))
                throw InputError{ "Bad arguments to '" + word + "' command" };

            auto table = tables.find(name);
            if (table == tables.end())
                throw InputError{ "No table named '" + name + "'" };

            auto constraint = make_shared<TableConstraint>(table->second);
            for (int i = 0 ; i < table->second->arity ; ++i) {
                string name;
                if (! (infile >> name))
                    throw InputError{ "Bad arguments to '" + word + "' command" };
                constraint->associate_with_variable(get_name(name));
            }
            model.add_constraint(constraint);
        }
        else if (word == "alldifferent") {
            int number;
            if (! (infile >> number))
                throw InputError{ "Bad arguments to '" + word + "' command" };

            vector<VariableID> vars;
            for (int i = 0 ; i < number ; ++i) {
                string var;
                if (! (infile >> var))
                    throw InputError{ "Bad arguments to '" + word + "' command" };
                vars.push_back(get_name(var));
            }

            auto constraint = make_shared<AllDifferentConstraint>(move(vars));
            model.add_constraint(constraint);
        }
        else if (word == "#") {
            string ignore;
            getline(infile, ignore);
        }
        else {
            throw InputError{ "Unknown command '" + word + "'" };
        }
    }

    return model;
};

