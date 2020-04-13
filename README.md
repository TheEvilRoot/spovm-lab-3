СПОВМ ЛР3 (Linux)
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

`client and `server` binaries will be stored in `build/` directory.

Client will write it's output to `client.log` file. Use 

```bash
tail -f client.log
```

to monitor client output in real time.
