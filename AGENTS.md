## gem5 compilation

gem5 is compiled with scons. A typical build command (and one used frequently during development and tesitng) is:

  ```sh
  scons build/ALL/gem5.opt -j<number_of_cores>
  ```
This command builds gem5 for all supported architectures (`ALL`) in optimized mode (`.opt`).

## gem5 Testing

1. `cd tests && ./main.py run --length=quick`. This command runs the gem5 test suite with a quick length setting. (`long` and `very-long` can also be used for more extensive testing.). This will compile the gem5 binaries needed. If the binaries are already compiled pass `--skip-build` to skip the build step.

2. `scons build/ALL/unittests.opt -j<number_of_threads>`. This command builds and runs the gem5 unit tests.
