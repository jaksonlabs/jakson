## Cost Model Design

To be able to design our Cost Model we have to define cost defining factors,
allowing an

To start we have a few system variants to consider:

  1. Amount of Real Threads (hyperthreading & without)

    This is a quite influential factor in our estimation, since a higher number
of real threads tend to allow for a higher number of spawned threads. The real
number can in this case always be read from the system.

  2. Load on Threads

    This factor maybe more or less influential strongly depending on the current
state of the system, in some situation, first and foremost when other
computational intensive processes are running, this can improve the prediction,
because effective less threads are available.  Read from system.

  3. Batch Size

    Should be included, to account for situations with fast switching threads
and longer running ones. E.g. Overhead of creating threads for small batches may
be larger than the payoff they provide, and vice versa.

  4. Complete File Size

    I'm not sure about this factor right now, in edge cases this may be quite
important but generally it'll probably end up being insignificant.  Our standard
chunk size is `5 MiB` and if we consider a file size of `~1 GiB` as small
everything larger like `open-academic-graph` as large, this factor will have
basically zero relevance.

These variables are to be considered beforehand and can be used to guess a rough
first value to start encoding with.

The following factors will be more important during runtime:

  1. Previous Duration

    Meant hereby is the difference between the duration after adjusting the
number of threads, if this is positive (the encoding of batch took longer than
expected) than this factor should be included in future iterations.  Downside of
this factor is that it may only be retroactively applied, the optimum may only
be reached in the next iteration.

  2. Number of Lines / Line Length

    About this factor I'm not sure how big the impact really is, if less lines
could be read in the same chunk size the lines are probably more complex and
splitting into more threads could be beneficial since a higher calculation time
can be expected.

While Line Count can be determined before the encoding is initialized, factors
like Previous Duration can only correct errors already passed, more like a
PID-Controller. The correct combination and weighting is here important.

Really as I'm writing this down again, I think a Gradient Descent is in this
situation more appropriate. As the changes to the threads are not completely
clear beforehand and the weighting of single factors like Thread Load and Data
Complexity are not clear, and may be analyzed at a more expensive rate than the 
actual benefit achieved through it. This way through slight learning a better result could
be achieved

## ~~Cost Model Design~~ Gradient Descent Design

### How to Gradient Descent

I propose we should use in our case a stochastic gradient descent since we try to adjust our solution "on the fly".

We would start with generating a vector for our weights, generally we use rather small values for this. I suggest a value of 0.1 for this, at the beginning, this can be changed later if desired.

Vectors we require in our algorithm are:

w - weight vector
x - input vector

other values are: 

n - learning rate, again something small 0.1 to 0.01
y - current result
py - previous result

Then to update our values according to our observation with 

w_i = w_i - n * (2* x_i (py - y))

to adjust weigths, results should be adjust over time to fit the local optimum this way

### Weight Vector
- load
- processor_count
- batch_size
- file_size

### Result

Result of the current weigths with the system context: sum(w_i * x_i)
