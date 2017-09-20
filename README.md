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

This section explains how a user can define new types of counters. First of all,
to better understand how MDCS works, one has to make the distinction between
 * The type of data pushed into the counter (which we call "item type")
 * The type of data kept internally by the counter (which we call "data type")
 * The type of data read when getting the value of a counter ("value type")

For example in the MDCS_COUNTER_STAT_DOUBLE counter, the item type is `double`,
while the data type and the value type are of type `mdcs_counter_stat_double_t`,
the definition of which is found in `mdcs/mdcs-counters.h`.

Let's define a new counter type called "range tracker", which will keep track
of the difference between the minimum and the maximum values that have been
pushed to it. The item, data, and value types of this counter will be as follows:
 * The item type will be `int32_t`
 * The data type will be a structure with the min and max as `int32_t`
 * The value type will be an `uint32_t` (the value is necessarily positive and
should be able to represent the difference between a very large `int32_t` and a very small one)

We define our types as follows in `range-tracker.h`:

```c
typedef struct range_tracker_data_t {
    int32_t min;
    int32_t max;
} range_tracker_data_t;
typedef int32_t  range_tracker_item_t;
typedef uint32_t range_tracker_value_t;
```

Now we also need 4 functions to build our counter types:
 * A `reset` function to reset the counter's internal value
 * A `push_one` function to push an item to the counter
 * A `push_multi` function to push multiple items to the counter
 * A `get_value` function to get the current value of the counter

We declare their prototype in the header file `range-tracker.h`:

```c
void range_tracker_reset(range_tracker_data_t* data);
void range_tracker_get_value(range_tracker_data_t* data, range_tracker_value_t* value);
void range_tracker_push_one(range_tracker_data_t* data, range_tracker_item_t* item);
void range_tracker_push_multi(range_tracker_data_t* data, range_tracker_item_t* items, size_t n);
```

We can now define them in an implementation file `range-tracker.c`:

```c
#include <limits.h>
#include "range-tracker.h"

void range_tracker_reset(range_tracker_data_t* data)
{
    data->min = INT_MAX;
    data->max = INT_MIN;
}

void range_tracker_get_value(range_tracker_data_t* data, range_tracker_value_t* value)
{
    if(data->min == INT_MAX && data->max == INT_MIN) {
        *value = 0;
    } else {
        *value = (range_tracker_value_t)(data->max - data->min);
    }
}

void range_tracker_push_one(range_tracker_data_t* data, range_tracker_item_t* item)
{
    if(data->min > *item) {
        data->min = *item;
    }
    if(data->max < *item) {
        data->max = *item;
    }
}

void range_tracker_push_multi(range_tracker_data_t* data, range_tracker_item_t* items, size_t n)
{
    size_t i;
    for(i=0; i<n; i++) {
        range_tracker_item_t* x = items + i;
        if(data->min > *x) data->min = *x;
        if(data->max < *x) data->max = *x;
    }
}
```

Note that in our implementation of `push_multi`, we simply do a `for` loop
over what `push_one` would do. But one could think of counter types for which there is
a better way of handling items in batches than iterating through them one by one.

Let's now create our type in the server:

```c
    mdcs_counter_type_t range_tracker_type = MDCS_COUNTER_TYPE_NULL;

    mdcs_counter_type_create(sizeof(range_tracker_data_t),
                             sizeof(range_tracker_value_t),
                             sizeof(range_tracker_item_t),
                             (mdcs_reset_f)range_tracker_reset,
                             (mdcs_push_one_f)range_tracker_push_one,
                             (mdcs_push_multi_f)range_tracker_push_multi,
                             (mdcs_get_value_f)range_tracker_get_value,
                             &range_tracker_type);
```

And let's declare a counter of that type:

```c
mdcs_counter_t myrange = MDCS_COUNTER_NULL;
mdcs_counter_register("example:myrange", range_tracker_type, 0, &myrange);
```

It is now possible for the server to push items to the counter.
Remember that those items should have the type `range_tracker_item_t`.

It is also possible now for a client to get the value of the counter by
calling `mdcs_remote_counter_fetch`. Remember that the value must have the type
`range_tracker_value_t`.

Finally, one has to free the counter type before finalizing MDCS:

```c
mdcs_counter_type_destroy(range_tracker_type);
```

Note to potential contributors
==============================

If you implement counter types that you think would be useful to other users
of MDCS, don't hesite to submit pull requests to this project!

Recommendation to service implementers
======================================

Since multiple services may register their own counters, we advise service
implementors to prefix their counters with the name of their service. For
example if the service is called "example", a counter "x" in this service
can be registered as "example:x". This will avoid conflicts with other services
registering another counter "x".