#!/bin/bash

set -x

if ! grep '^status = false$' <(./certified_constraint_solver models/nosearchunsat.model ) ; then
    echo "no search unsat test failed" 1>&1
    exit 1
fi

if ! grep '^status = false$' <(./certified_constraint_solver models/nosearchunsattable.model ) ; then
    echo "no search unsat table test failed" 1>&1
    exit 1
fi

if ! grep '^status = true$' <(./certified_constraint_solver models/babysat.model ) ; then
    echo "baby sat test failed" 1>&1
    exit 1
fi

if ! grep '^status = false$' <(./certified_constraint_solver models/babyunsat.model ) ; then
    echo "baby unsat test failed" 1>&1
    exit 1
fi

if ! grep '^status = true$' <(./certified_constraint_solver models/babytable.model ) ; then
    echo "baby table test failed" 1>&1
    exit 1
fi

if ! grep '^status = true$' <(./certified_constraint_solver models/reusetable.model ) ; then
    echo "reuse table test failed" 1>&1
    exit 1
fi

true

