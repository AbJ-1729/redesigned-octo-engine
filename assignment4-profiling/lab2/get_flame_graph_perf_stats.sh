#! /bin/bash
make all
taskset -c 0 perf stat -e cycles,instructions,branches,branch-misses,cache-references,cache-misses,L1-dcache-loads,L1-dcache-load-misses ./main
make perf
taskset -c 0 perf record -g ./main
perf report -i perf.data --stdio > perf.txt
perf script -i perf.data > perf.script
~/FlameGraph/stackcollapse-perf.pl perf.script > perf.folded
~/FlameGraph/flamegraph.pl perf.folded > flamegraph.svg