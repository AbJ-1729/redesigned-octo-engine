#! /bin/bash

make perf
taskset -c 0 perf record -g ./grid_bfs
perf report -i perf.data --stdio > perf.txt
perf script -i perf.data > perf.script
~/FlameGraph/stackcollapse-perf.pl perf.script > perf.folded
~/FlameGraph/flamegraph.pl perf.folded > flamegraph.svg