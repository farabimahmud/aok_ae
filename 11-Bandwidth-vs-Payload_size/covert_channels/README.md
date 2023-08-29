# Covert Channel Attack

To realize this attack. First build the sender and the receiver 
```
make sender;
make receiver;
```

Update the parameters in the `util.h` accordingly. 

Sender tries to send 0/1 alternatively.

Detects a bit by measuring the execution time of the prefetchw instruction  within the clock length of config->interval.

Detect a bit 1 if the exection time > Prefetcher Miss latency
Detect a bit 0 otherwise


You should be able to define the round using the ROUNDS variable inside `util.h`