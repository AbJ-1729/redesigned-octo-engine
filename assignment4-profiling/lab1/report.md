# Intro Profiling Lab Report

## 1. Optimizations Made

Old:
```
int *distance = new int[total];
std::fill(distance, distance + total, -1);
unsigned char *visited = new unsigned char[total]{};
```
New:
```
auto distance = std::make_shared<int[]>(total);
auto visited = std::make_shared<unsigned char[]>(total);
```
to fix memory leak reported by valgrind

## 2. Methodology Walkthrough

valgrind reported 2 definitely lost and 2 possibly lost memory leaks all originating directly from shortest_path_bfs function. I noticed that the memory allocated using new was never freed up from the heap, thus decided to use smart pointers instead. Since the ownership wasn't shared, I used unique pointers
as seen from "valgrind memcheck-before.txt" and "valgrind memcheck-after.txt", all the memory leaks are fixed after this change
Include before/after evidence from:

- `time`
- `perf stat`
- FlameGraph
- Callgrind/KCachegrind
- Valgrind leak summary

## 3. Correctness Evidence

Include:

- `make test`
- Final normal run output
- Checksum comparison before and after optimization

## 4. Conceptual Questions

Answer Q1.1 through Q6.1 from the README.

1.1
real measures the actual time by clock taken by the program to complete, but user implies cpu time taken by user and sys implies cpu time taken by system. since the cpu can be idle at some point waiting for other operations to complete, real != user+sys, usually >. in case of multithreading, real can also be < user+sys.
2.1
perf utilises the inbuilt hardware PMUs(Performance Monitoring Units) to calculate event counts, which are then used to calculate derived metrics through simple formulas
2.2
right side percentages tell the percentage of time spent by perf in measuring that metric, which is then extrapolated to get the total count, since due to hardware constraints(limited number of physical counters(PMU(Performance Monitoring Unit))), it can't measure all metrics simultaneously 100% of the time
2.3
if the number of metrics asked for(tracked) are small, then it perf can use dedicateed PMUs for measuring full time, in which case the metrics would be exacct, but if too many metrics are being tracked, perf extrapolates them according to time spent, in which case they might not be 100% accurate
3.1
frame pointers are pointers fixed throughout the lifetime of function call, and by analysing them along with the current instruction pointers, perf -g is able to reconstruct the exact order in which functions are called at the hardware level
3.2
self cost is the time spent executing the function itself, while inclusive cost is the time spent in the function plus the time spent in all the other functions it called, thus the cumulative time.
4.1
on compiling with -pg, compiler inserts hidden function calls inside the compiled binary so that every funciton call can be intercepted and caller, callee pairs are identified
4.2
perf works on sampling thus the function calls metrics are not completely accurate whereas in gprof since it modifies the binary itself, though it takes time to compile and is largerin size, we get the exact metrics and caller-callee 
5.1
address sanitizer detects stack overflows along with heap memory leaks, whereas valgrind memcheck detects just heap memory leaks. the key difference is address sanitizer requires recompiling into a larger binary whereas valgrind memcheck runs any binary and intercepts the calls using just in time compilation and adding memory tracking logic, thus it is slightly slower than addresssanitizer. Therefore we can use address sanitizer to detect memory leaks in canse code is recompilable or else if we just have the compiled binary, we can use valgrind memcheck.
6.1


