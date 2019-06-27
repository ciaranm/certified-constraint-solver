TARGET := certified_constraint_solver

SOURCES := \
    constraint.cc \
    certified_constraint_solver.cc \
    model.cc \
    read_model.cc \
    refutation_log.cc \
    solve.cc \
    variable.cc

TGT_PREREQS := run-tests.bash

ifeq ($(shell uname -s), Linux)
TGT_LDLIBS := $(boost_ldlibs) -lstdc++fs
else
TGT_LDLIBS := $(boost_ldlibs)
endif

TGT_POSTMAKE := bash ./run-tests.bash
