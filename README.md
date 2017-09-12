Installing MDCS
===============

To build and install MDCS, make sure that the following dependencies have been
already installed:

 * Cmake
 * Boost
 * Mercury
 * Argobots
 * Margo
 
Clone the repository using 
```
git clone https://xgitlab.cels.anl.gov/sds/mdcs.git
```

Then
```
cd mdcs
mkdir build
cd build
cmake .. -G "Unix Makefiles" \
    -DBOOST=ROOT=/path/to/boost \
    -DMERCURY_ROOT=/path/to/mercury \
    -DABT_ROOT=/path/to/argobots \
    -DMARGO_ROOT=/path/to/margo \
    -DCMAKE_INSTALL_PREFIX=/path/where/to/install/mdcs
make
make install
```

How to use MDCS
===============

MDCS can be used as follows within a server to add a counter and manipulate it:

```c
#include <mdcs/mdcs.h>
#include <mdcs/mdcs-counters.h>

mdcs_initialize(mid, MDCS_TRUE); // initialize with a margo instance

mdcs_counter_t mycnt = MDCS_COUNTER_NULL; // counter handle
mdcs_counter_t mystat = MDCS_COUNTER_NULL;

// register a new counter, of type MDCS_COUNTER_LAST_INT64, with no buffer
mdcs_counter_register("example:mycounter", MDCS_COUNTER_LAST_INT64, 0, &mycnt);
// register a new counter, of type MDCS_COUNTER_STAT_DOUBLE, with 10-element buffer
mdcs_counter_register("example:mystat", MDCS_COUNTER_STAT_DOUBLE, 10, &mystat);

// push a new value to a counter
int64_t value = 42;
mdcs_counter_push(mycnt, &value);

double x = 0.34;
mdcs_counter_push(mystat, &x);

// force the buffered values to be "digested"
mdcs_counter_digest(mystat);

// get the current content of the counter
int64_t stored;
mdcs_counter_value(mycnt, &stored);

mdcs_counter_stat_double_t statistics;
mdcs_counter_value(mystat, &statistics);

mdcs_finalize(); // finalize MDCS
```

From a client, to get the value of a counter inside a remote server:

```c
#include <mdcs/mdcs.h>
#include <mdcs/mdcs-counters.h>

mdcs_initialize(mid, MDCS_FALSE); // initialize with a margo instance

mdcs_counter_id_t cid; // reference to a remote counter

// get the id of the counter
mdcs_remote_counter_get_id("example:mystat", &cid);

// get the current value
mdcs_counter_stat_double_t statistics;
mdcs_remote_counter_fetch(cid, &statistics, sizeof(statistics));

// reset the counter
mdcs_remote_counter_reset(cid);

mdcs_finalize(); // finalize MDCS
```

Right now 4 types of counters are available:

 * MDCS_COUNTER_LAST_DOUBLE and MDCS_COUNTER_LAST_INT64 respectively store the
 last double and int64_t values that get pushed into them.
 * MDCS_COUNTER_STAT_DOUBLE and MDCS_COUNTER_STAT_INT64 maintain statistics
 (count, min, max, average, variance, and last pushed value) of the values that
 are pushed to them (see the definition of their content in mdcs/mdcs-counters.h)
 
User-defined counters
=====================

This section will be updated later...
