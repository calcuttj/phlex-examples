# Phlex examples

This repository contains examples of the use of the
[Phlex framework](https://github.com/framework-r-d/phlex).

## Building this repository

To use the code in this repository, first install it:

```console
git clone -b phlex-v0.4 https://github.com/Framework-R-D/phlex-examples.git
```

Now create a build directory:

```console
mkdir build-phlex-examples
cd build-phlex-examples
```

The repository can now be easily built by activating
[the environment you created when installing Phlex](https://github.com/Framework-R-D/phlex/blob/main/INSTALLATION.md#creating-and-installing-the-phlex-environment):

```console
spack env activate my-phlex-environment
spack load gcc@15 cmake  # adjust if you used a different compiler for phlex
cmake ../phlex-examples
make -j 12  # choose a suitable number of jobs for your machine
```

## Running the code

### Add your build directory to `PHLEX_PLUGIN_PATH`

```console
# Make sure you do this from your build directory
export PHLEX_PLUGIN_PATH=${PWD}:${PHLEX_PLUGIN_PATH}
```

You should now be able to run a simple `phlex` job:

### Job with C++ algorithms

```console
phlex -c ../phlex-examples/test-cpp-workflow.jsonnet
```

The output should look like:

```console
Using configuration file: ../phlex-examples/test-cpp-workflow.jsonnet
[2025-12-22 13:17:51.496] [info] Number of worker threads: 24
Registering FORM output module...
Configuration:
  output_file: output.root
  technology: ROOT_TTREE
FormOutputModule initialized
⋮
[2025-12-22 13:17:51.760] [info]
Processed layers:

  job
   │
   └ spill: 10

[2025-12-22 13:17:51.760] [info] CPU time: 0.21204s  Real time: 0.26498s
  CPU efficiency: 80.02%
[2025-12-22 13:17:51.760] [info] Max. RSS: 268.848 MB
```

### Job with an unfold

To run a job that demonstrates the `unfold` higher-order function — splitting a
parent-layer product into a child layer of data cells — use:

```console
phlex -c ../phlex-examples/test-cpp-unfold-workflow.jsonnet
```

The source provides a `std::vector<int>` per `spill`, the unfold emits one `int`
per element on a child `number` layer, a transform squares each value, and an
observer asserts the result. The output should look like:

```console
[info]
Processed layers:

  job
   │
   └ spill: 5
      │
      └ number: 15

[info] CPU time: ...
```

`number: 15` is 3 elements × 5 spills.

### Job with Python algorithms

To run a job that uses a Python algorithm, the `PYTHONPATH` environment variable
must be adjusted to include the directory with the Python module:

```console
export PYTHONPATH=$(realpath ../phlex-examples):${PYTHONPATH}
phlex -c ../phlex-examples/test-py-workflow.jsonnet
```

The output should look like:

```console
Using configuration file: ../phlex-examples/test-py-workflow.jsonnet
[2025-12-22 13:50:07.999] [info] Number of worker threads: 24
[2025-12-22 13:50:08.013] [info]
Processed layers:

  job

[2025-12-22 13:50:08.013] [info] CPU time: 0.01022s  Real time: 0.01483s
  CPU efficiency: 68.91%
[2025-12-22 13:50:08.013] [info] Max. RSS: 29.628 MB
```

> [!NOTE]
> For Phlex v0.4.0, Python data products cannot be written to output files.

### Job with a Python event generator

`py_simple_generator_hepmc.py` is a Python port of `simple_generator_hepmc.cpp`.
Phlex's Python plugin can only pass numbers and 1-D numeric arrays to the rest
of the workflow, so it cannot return a `HepMC3::GenEvent`. Instead the Python
source provides a flat particle table, and the C++ module `genevent_from_table`
turns that table into a `HepMC3::GenEvent`.

The table is one `float64` array per event, in GeV and cm. An event can have
several vertices, each with any number of outgoing particles; `vertex` is the
index (from 0) of a particle's production vertex:

```text
[event_number, n_vertices,
 then per vertex: x, y, z, t,
 then per particle: vertex, pdg, status, px, py, pz, E, mass]
```

Build tables with `GenEventTable` from `genevent_table.py` rather than by hand.
`genevent_from_table` rejects malformed tables with an error.

With `PYTHONPATH` set as above, run:

```console
phlex -c ../phlex-examples/test-py-simple-generator-hepmc-convert-save.jsonnet
```

This writes the same events as
`test-simple-generator-hepmc-convert-save.jsonnet`.

`genevent_from_table` is named `input` and outputs suffix `genevent`, the same
names as the C++ sources, so modules such as `convert_hepmcevent_to_data` and
`esp_tracking` read its events unchanged. For that reason, don't put another
source that provides `input`/`genevent` in the same job.

Generator options for `py_simple_generator_hepmc.py`: `layer`, `position`
(vertex in cm, default `'0 0 0'`) and `muon_angle` (degrees, default `100`).
`plugin_generator.py` takes `positions` instead, one `'x y z'` string per
vertex, and generates one interaction at each.

Give positions as strings such as `'0 0 1.5'`. Phlex's Python configuration
cannot read nested arrays, or numeric arrays that start with an integer and
later contain a non-integer (for example `[0, 10, 20.5]`). `phlex_config.py`
reports such values as errors instead of silently using the default.
