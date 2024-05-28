# Reproducing the results in our paper

Follow the following instructions if you want to reproduce the results in our paper.

## Circuits (section 5)

Follow these steps to reproduce the results in section 5 of our paper.

### 1. Compile the circuits

We provide a script to compile every circuits: `compile-all.sh`.
To execute this script using the installed Flexo docker image, run the following command:

```sh
docker run -i -t --rm \
  --mount type=bind,source="$(pwd)/.."/,target=/flexo \
  flexo \
  bash -c "cd /flexo/reproduce && ./compile-all.sh"
```

It takes around 10 mintes to compile every circuits.
The circuits are roughly 2.7 GB in size.

## UPFlexo packer (section 6)

Follow these steps to reproduce the results in section 6 of our paper.

### 1. Compile the packers

Please refer to the [readme file](/UPFlexo/README.md) under `UPFlexo/` for instructions about how to compile the packers.

### 2. Pack the example program (`ls`)

Run the `pack-all.sh` script under this directory to pack the example program, which is the `ls` binary from Ubuntu 23.04.
It takes around 20 seconds to pack the example program, and the packed programs are roughly 233 MB in size.
