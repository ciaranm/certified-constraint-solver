A Certified Constraint Solver
=============================

This is a very simple constraint programming solver whose main feature is support for proof logging.
This is a research toy to try out a few new ideas, not serious code. It is not designed to have good
performance, to be powerful, or to be something you would want to use for application purposes.

Please contact [Ciaran McCreesh](mailto:ciaran.mccreesh@glasgow.ac.uk) with any queries.

Compiling
---------

This project uses git submodules for dependencies. You should run 'git submodule update --init'
after your initial 'git clone'.

To build, type 'make'. You will need a C++17 compiler (we test with GCC 8.3, and Clang 7.0.1 on
Linux, and Xcode 10.2 on Mac OS X) and Boost (we use 1.65.1 or later, built with threads enabled).

Modelling
---------

An input file looks like this:

```
# Create two variables named a and b, each with
# domains { 1, 2, 3, 4 }
intvar a 1 4
intvar b { 1 2 3 4 }
# Post the constraint a != b
notequal a b
```

The solver also supports all different constraints:

```
intvar a 1 2
intvar b 1 2
intvar c 1 3
alldifferent 3 a b c
```

And table constraints, where a table can be reused multiple times:

```
intvar x1 0 1
intvar x2 0 1
intvar x3 0 1
intvar x4 0 1
# First we create a table, and say that each of its
# tuples is of length 3
createtable oneofthree 3
# Now we specify permitted tuples in this table
addtotable oneofthree 1 0 0
addtotable oneofthree 0 1 0
addtotable oneofthree 0 0 1
# We apply this table constraint to three variables
table oneofthree x1 x2 x3
# And we can apply it again to another three variables
table oneofthree x2 x3 x4
```

And for convenience, variables can be forced to a constant value:

```
intvar a 1 3
equal a 2
```

Finally, to create lots of variables, you can do this:

``
intvararray x 2 1 2 1 4 1 10
equal x[2,4] 5
``

This will create variables named ``x[1,1]`` through ``x[2,4]``, each with domains from 1 to 10. Note
that currently, only 2d arrays (third argument) are supported. Also, the square bracket syntax is
simply a part of the variable name, not real array dereferencing.

Running
-------

To run the solver:

```shell session
./certified_constraint_solver models/babysat.model
```

To run the solver, and produce a proof of unsat (assuming the model actually is unsat):

```shell session
./certified_constraint_solver --prove models/babyunsat.model
```

This will write an equivalent pseudo-boolean model in OPB format to ``models/babyunsat.opb``, and a
proof log to ``models/babyunsat.log``. These can then be verified using ``refpy``:

```shell session
refpy models/babyunsat.opb models/babyunsat.log
```

You can find refpy at https://github.com/StephanGocht/refpy/ .

Funding Acknowledgements
------------------------

This work was supported by the Engineering and Physical Sciences Research Council (grant number
EP/P026842/1), although they don't know that.

<!-- vim: set tw=100 spell spelllang=en : -->

