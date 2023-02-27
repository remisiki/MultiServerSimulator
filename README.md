# MultiServerSimulator

This program implements the following policies to simulate job queueing through multi servers.

- fcfsLocal: First Come First Serve, all jobs are served locally (jobs from region A will not be served in region B)
- fcfsCross: First Come First Serve, all jobs may be served at a remote server if current region is congested
- fcfsCrossPart: First Come First Serve, only small jobs may be served at a remote server if current region is congested

## Requirements

`gsl>=2.7`

CPU architecture must be x86_64 to use `immintrin.h`

## Build

Make sure you have [gsl](https://www.gnu.org/software/gsl/) installed, then under root directory run 
```bash
make
```

## Usage

Using default parameters, run 
```bash
./sim
```

Options:

<table>
  <tr>
    <td><code>-h</code></td>
    <td>Show this help message.</td>
  </tr>
  <tr>
  <tr>
    <td><code>-p</code></td>
    <td>Specify policy from <code>fcfsLocal</code>, <code>fcfsCross</code>, <code>fcfsCrossPart</code>. default <code>fcfsLocal</code></td>
  </tr>
    <td><code>-t time</code></td>
    <td>Specify a simulation iteration of <code>time</code> units. default <code>1000000</code></td>
  </tr>
  <tr>
    <td><code>-n num</code></td>
    <td>Specify number of processors for each server to be <code>num</code>. This will force all servers to have the same number. default <code>48</code></td>
  </tr>
  <tr>
    <td><code>-j jobCnt</code></td>
    <td>Specify job type count as <code>jobCnt</code>. Must be set before (and together with) <code>-l</code> and <code>-s</code>. default <code>2</code></td>
  </tr>
  <tr>
    <td><code>-l [lambda...]</code></td>
    <td>Specify arrival rate. Must be set together with <code>-j</code>. <code>lambda</code> must have size of <code>jobCnt</code> and is separated by a comma (<code>,</code>, with no spaces). default <code>10,4</code></td>
  </tr>
  <tr>
    <td><code>-s [serviceTime...]</code></td>
    <td>Specify mean service time. Must be set together with <code>-j</code>. <code>serviceTime</code> must have size of <code>jobCnt</code> and is separated by a comma (<code>,</code>, with no spaces). default <code>1,4</code></td>
  </tr>
  <tr>
    <td><code>-v</code></td>
    <td>Run simulation verbosely.</td>
  </tr>
<table>

Example:
```bash
./sim -t 100000 -n 96 -j 3 -l 10,4,2 -s 1,4,10 -v -p fcfsCross
```
