TARGET := make_sudoku_model

SOURCES := \
    make_sudoku_model.cc

ifeq ($(shell uname -s), Linux)
TGT_LDLIBS := $(boost_ldlibs) -lstdc++fs
else
TGT_LDLIBS := $(boost_ldlibs)
endif
