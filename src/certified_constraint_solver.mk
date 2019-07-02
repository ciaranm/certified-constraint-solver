TARGET := certified_constraint_solver

SOURCES := \
    all_different_constraint.cc \
    certified_constraint_solver.cc \
    constraint.cc \
    model.cc \
    not_equals_constraint.cc \
    proof.cc \
    read_model.cc \
    solve.cc \
    table_constraint.cc \
    variable.cc

TGT_PREREQS := run-tests.bash

ifeq ($(shell uname -s), Linux)
TGT_LDLIBS := $(boost_ldlibs) -lstdc++fs
else
TGT_LDLIBS := $(boost_ldlibs)
endif

TGT_POSTMAKE := bash ./run-tests.bash
