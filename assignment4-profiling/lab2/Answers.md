Since  dependency_count is constant = 2^18, => size of next() - 1 = 2^18 - 1 which is simply 18 1's in binary notation. 
In chase_dependency function, it means idx is always >= 0 and <= 2^18-1
So we get all possible values of idx.
My idea is to avoid repeatedly processing this since STEPS=7, and thus we can simply precompute the values instead for all possible starting idx.

Total time reduced from ~0.045s to ~0.027s
Cache misses also visibly reduced since we are not doing cache thrashing by accessing random indexes elements everytime as in the chase_dependency function. (perf-stats-v1.txt)

On increasing history_cols to 2048. TLB misses increase a lot, highlighting some bad code
On analysing the code, i noticed that in cold_column_probe iteration is done in a column major format leading to cache misses, on swapping the order, cache misses reduce drastically from 28.91% to  4.96% also decreasing time from ~0.26s to ~0.11s (perf-stats-2048.txt)

Now the optimizations seems sufficient.