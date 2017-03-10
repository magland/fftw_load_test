# fftw_load_test

In the most recent version of MountainSort, the pre-processing step has been the most time-consuming operation and the bottleneck of the spike sorting pipeline. Sometimes it was 2-3 times slower than clustering itself. How could that be?

I've been working on this issue for many days, and the the last thing I expected was that there was a limitation in the FFTW library itself. Could it be that fftw doesn't do well when multiple processes are running at the same time?!

Try it out for yourself and let me know if I'm making a mistake here.

## Installation

First you need Qt5, fftw, and nodejs on your system. See http://github.com/magland/mountainlab for instructions on that.

```bash
git clone [this repo url]
cd fftw_load_test
qmake
make
```

Then try to run a single fftw process
```bash
./fftw_load_test.js --num_processes=1 --task=fftw
```

Here's what I get:
```bash
magland@jm3:~/dev/fftw_load_test$ ./fftw_load_test.js --num_processes=1 --task=fftw
Calling: bin/fftw_load_test --task=fftw
Creating plan...
Running fftw load test for M,N=4,40000000
Elapsed time (sec) for M,N=4,40000000 (1.20901e+7 nums per sec): 13.234
```

Now try with 6 processes:
```bash
magland@jm3:~/dev/fftw_load_test$ ./fftw_load_test.js --num_processes=6 --task=fftw
Calling: bin/fftw_load_test --task=fftw
Calling: bin/fftw_load_test --task=fftw
Calling: bin/fftw_load_test --task=fftw
Calling: bin/fftw_load_test --task=fftw
Calling: bin/fftw_load_test --task=fftw
Calling: bin/fftw_load_test --task=fftw
Creating plan...
Creating plan...
Creating plan...
Creating plan...
Creating plan...
Creating plan...
Running fftw load test for M,N=4,40000000
Running fftw load test for M,N=4,40000000
Running fftw load test for M,N=4,40000000
Running fftw load test for M,N=4,40000000
Running fftw load test for M,N=4,40000000
Running fftw load test for M,N=4,40000000
Elapsed time (sec) for M,N=4,40000000 (1.10079e+7 nums per sec): 14.535
Elapsed time (sec) for M,N=4,40000000 (1.02328e+7 nums per sec): 15.636
Elapsed time (sec) for M,N=4,40000000 (1.01183e+7 nums per sec): 15.813
Elapsed time (sec) for M,N=4,40000000 (6.55845e+6 nums per sec): 24.396
Elapsed time (sec) for M,N=4,40000000 (6.16642e+6 nums per sec): 25.947
Elapsed time (sec) for M,N=4,40000000 (6.11084e+6 nums per sec): 26.183
```

Wow! It's slower. And unpredictable! That's what I've been dealing with in bandpass_filter of MountainSort.

Compare that with a simple floating-point operations test:
```bash
magland@jm3:~/dev/fftw_load_test$ ./fftw_load_test.js --num_processes=1 --task=flops
Calling: bin/fftw_load_test --task=flops
Running fftw load test for M,N=100,40000000
Elapsed time (sec) for M,N=100,40000000 (1.08903e+9 nums per sec): 3.673
Print the result to force computation: 3.96e+18
```

That's for one process. Now try 6 processes:

```bash
Elapsed time (sec) for M,N=100,40000000 (1.0721e+9 nums per sec): 3.731
Print the result to force computation: 3.96e+18
Elapsed time (sec) for M,N=100,40000000 (1.07585e+9 nums per sec): 3.718
Print the result to force computation: 3.96e+18
Elapsed time (sec) for M,N=100,40000000 (1.06496e+9 nums per sec): 3.756
Print the result to force computation: 3.96e+18
Elapsed time (sec) for M,N=100,40000000 (1.03146e+9 nums per sec): 3.878
Print the result to force computation: 3.96e+18
Elapsed time (sec) for M,N=100,40000000 (1.0296e+9 nums per sec): 3.885
Print the result to force computation: 3.96e+18
Elapsed time (sec) for M,N=100,40000000 (1.03573e+9 nums per sec): 3.862
Print the result to force computation: 3.96e+18
```

It's as expected!

Does FFTW really have this serious (IMO) limitation?

A possible explanation is that fftw does fancy optimization for single process execution and somehow does not consider the multiple independent processes scenario.

Or maybe this only happens on my computer(s)? I tested on my desktop and my server.

Then I tested on my laptop, and the multiple simultaneous processes caused fftw_exec to crash. Maybe I'm doing something wrong, but if so, I'd like to know what.
