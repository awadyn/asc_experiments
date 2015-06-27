# ASC - Automatically Scalable Computation

## Compiling

### Normal

    make

### Benchmarking

    make -B CFLAGS="-Ofast -march=native -flto -DNDEBUG"

## Verification

How do we gain confidence that the automatic parallel speedups that we are
measuring are real?

For example, how do we guard against the possibility of somehow fooling
ourselves into thinking that we are seeing `asc` effect parallel speedups when
in fact it is (for example) merely making a good guess at the correct output,
printing its guess, and stopping early?

Clearly we could try picking an input and compare the corresponding output
of the program when run on bare metal to the output of the program when run
under `asc`, but it's hard to say how much confidence per successful experiment
this should give us.  For example, maybe *we* are actually poor sources of
entropy, and `asc` is simply predicting which inputs we might try for this
experiment and pre-calculating the correct answers while we sleep!

We don't actually believe that `asc` is malicious.  The point is: when
we make our results public, we must be prepared for the fact that a statement
of automatic parallel speedups even for a restricted class of programs is a
strong claim and will attract attention.  We'd like to have some confidence
that our work will hold up under that scrutiny, so it is prudent for us to adopt
a somewhat adversarial approach to verifying `asc`.

As a step in this direction, we'll use the 3-SUBSET-SUM solver program that is
distributed with `asc` under `kernels/3sum.c`.  This solver is a well-studied
instance[1] of a general class of linear satisfiability solvers: given as
input a multiset `S` of size `n`, where each element is an integer in the
range `[-N, +N]`, it solves for three elements `a`, `b`, and `c` in `S` such
that `a + b + c = 0`.  If such a solution exists, it prints the three integers
as its output, otherwise it prints nothing.  As a tiny example: for `n = 5` and
`N = 14`, the input might be `S = {1, 1, -10, 2, 9}`, for which the correct
output is `(1, -10, 9)`, since `1 + -10 + 9 = 0`.

Note that `3sum.c` has the option to uniform randomly *generate* its input `S`
if we supply it with a random seed `x` as input.
This option gives us the ability to control the input distribution, and thus
control the probability that `asc` is somehow faking its parallel speedups.
We do this by a sort of *commitment scheme*, where we pick the random seed
`x` *itself* at uniform random, and *first* run `asc 3sum x` and write
down the (claimed) output. Only after `asc` has committed to a (claimed) output
for our input `x` do we *then* run `3sum x` to check it.

We now describe one round of this verification protocol.  The random seed
we show will (almost certainly) be different if you try this experiment on
your computer.  Please do try this experiment, and notify us immediately
if `asc` fails the test!

First we pick a seed `x` at uniform random using the UNIX
shell's builtin randomization variable:

    $ x=$RANDOM
    $ echo $x
    16698

Then we select size parameters for the 3-SUBSET-SUM problem that we'd like
to solve: i.e., the multiset size `n` and the range `N` of the input integers.
As a tiny example, if we input `(x, n, N) = (1, 5, 14)`, `./3sum` will
generate the input `S = {1, 1, -10, 2, 9}`; i.e., 5 integers uniformly randomly
sampled from `[-14, +14]`.  We'll use larger parameters for the present
experiment:

    $ n=5000
    $ N=1750000000

Then we fetch a neural network that (it is claimed) can help `asc` speed up
execution of the `3sum` solver for arbitrary inputs:

    $ advice=http://silicoinformatics.seas.harvard.edu/models
    $ wget $advice/3sum/address/400190/16/3sum.net

Now we run the `3sum` solver under `asc`, with options to `asc` making it
compatible with the neural network that we downloaded (i.e., the neural network
learned a target function `f` that takes state vectors having a program counter
of `0x400190` to the sixteenth future instance of such a state vector), using
the UNIX shell's builtin facility for timing execution:

    $ time ./asc -a 400190/16 3sum.net 3sum $x $n $N
    ...
    -41247404	 58258350	-17010946
    real	0m29.838s
    user	0m6.135s
    sys	    0m0.036s

We write down the (claimed) output and the fact that `asc` executed the program
in about 29.9 seconds. Having forced `asc` to commit to an output, we now run
the program on bare metal on the same input:

    $ time ./3sum $x $n $N
    -41247404	 58258350	-17010946
    real	0m40.692s
    user	0m40.708s
    sys	    0m0.005s

We verify that the output is the same as when run under `asc`, and verify that
`-41247404 + 58258350 + -17010946 = 0`.  We conclude that `asc` (correctly)
sped up the execution of the `3sum` solver by a factor of 1.36, at least on
this random input.

By repeating this experiment (drawing a uniform random seed each time) we
believe that we can exponentially drive down the probability (at least under the
uniform input distribution) that the parallel speedups we are measuring for
`asc` are not real (at least for this solver).

For example, here is how to do a similar probabilistic check of parallel
speedup for the FACTOR kernel:

    newkey() { dc -e "$(primes 3000000000 3000010000 | shuf -n 2) * p"; }
    key=$(newkey)
    echo $key
    time ./asc -i 92274727 -a 400177 factor.net factor $key
    time ./factor $key

As another example, here is how to do a probabilistic check of parallel
speedup for the COLLATZ kernel:

    newbase() { echo $[9000000000000000000 + $RANDOM * $RANDOM * $RANDOM]; }
    base=$(newbase)
    echo $base
    time ./asc -a 40020d/1e4 collatz.net collatz $base 10000000
    time ./collatz $base 10000000

[[1] 3-SUBSET-SUM](https://en.wikipedia.org/wiki/3SUM)

## Calculating speedup

    awk '{old=$2;
          new=$1;
          speedup=old/new;
          percent=(1 - (1/speedup))*100;
          printf "%7.4f\t%7.4f\t%7.4f\t%7.3f\n", new, old, speedup, percent}'

[[2] Amdahl's Law](https://en.wikipedia.org/wiki/Amdahl%27s_law)

## Interesting experiments

### 3SUM

    ./asc -a 400190/16 3sum.net 3sum $RANDOM 5000 1750000000

### FACTOR

    ./asc -i 92274727 -a 400177 factor.net factor 9000037038035596873

### COLLATZ

    ./asc -a 40020d/1e4 collatz.net collatz 9223372036500000000 10000000

### LOOP

    ./asc -i 50331678 loop.net loop 9223372000000000000 100000000000

### COUNTER

    ./asc -i 134217816 -n 100 counter.net counter -1

### NOPS

    ./asc --singlestep -n 100000 nops.net nops -1

