#!/bin/bash

set -e

srcdir=$(pwd)

mkdir -p _ccache
export CCACHE_BASEDIR="$(pwd)"
export CCACHE_DIR="${CCACHE_BASEDIR}/_ccache"

ccache --zero-stats
ccache --show-stats
export CCACHE_DISABLE=true
meson \
        _build $srcdir
unset CCACHE_DISABLE

cd _build

ninja
ccache --show-stats
