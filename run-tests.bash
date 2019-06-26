#!/bin/bash

set -x

if ! grep '^status = true$' <(./certified_constraint_solver models/babysat.model ) ; then
    echo "baby sat test failed" 1>&1
    exit 1
fi

if ! grep '^status = false$' <(./certified_constraint_solver models/babyunsat.model ) ; then
    echo "baby unsat test failed" 1>&1
    exit 1
fi

true
