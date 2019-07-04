BUILD_DIR := intermediate
TARGET_DIR := ./

SUBMAKEFILES := src/certified_constraint_solver.mk

override CXXFLAGS += -O3 -march=native -std=c++17 -Isrc/ -Istrong_typedef/ -W -Wall -g -ggdb3 -pthread

ifeq ($(shell uname -s), Linux)
override LDFLAGS += -pthread -lstdc++fs
boost_ldlibs := -lboost_thread -lboost_system -lboost_program_options
else
override LDFLAGS += -pthread
boost_ldlibs := -lboost_thread-mt -lboost_system-mt -lboost_program_options-mt -lboost_filesystem-mt
endif
