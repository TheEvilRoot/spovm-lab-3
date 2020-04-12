СПОВМ ЛР2 (Linux)
----

How to run?
----

First, create cmake build directory

```bash
mkdir cmake-build
```

Next, configure cmake project

```bash
cd cmake-build
cmake ..
```

After cmake succeded, go back to project folder 

```bash
cd ..
```

and build binaries

```bash
cmake --build ./cmake-build --target all -j 2
```

`control` and `handler` binaries will be stored in `build/` directory.

