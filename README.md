# cache-simulator

#### To calculate read hit/miss, write hit/miss of given benchmark
#### Command-Line Parameter
    1. nk: the capacity of the cache in kilobytes (an int )
    2. assoc: the associativity of the cache (an int )
    3. blocksize: the size of a single cache block in bytes (an int )
    4. repl: the replacement policy (a char ); 'l' means LRU, 'r' means random.
    
#### Input
Read traces from the standard input. Each line on the standard input will be a lowercase 'r' (for read) or 'w' (for write) followed by a space and then a 64-bit hexadecimal number giving the address of the memory access. For example, a snippet of a trace file looks like this:
```
r 56ecd8
r 47f639
r 7ff0001ff
w 47f63e
r 4817ef
r 7d5ab8
```

#### Output
The output should be a single line of six numbers separated by spaces. These six numbers are:
  1. The total number of misses,
  2. The percentage of misses (i.e. total misses divided by total accesses),
  3. The total number of read misses,
  4. The percentage of read misses (i.e. total read misses divided by total read accesses),
  5. The total number of write misses,
  6. The percentage of write misses (i.e. total write misses divided by total write accesses).

#### Command Line Format
example:
```
Input:
$ ./cache 2048 64 64 l

output:
cache size   : 2048 KB
associativity: 64-ways
block size   : 64 B
replacement policy : l
read   hit: 937193
read  miss: 55703
write  hit: 7055
write miss: 49
Total  Miss rate: 5.575200%
Read   Miss rate: 5.610155%
write  Miss rate: 0.689752%
```

