# UPFlexo

A modified UPX packer that uses a weird machine to unpack.

## Build UPFlexo packers

### 0. Install Flexo packer

Please refer to the [README](../README.md) file in the parent directory about how to install Flexo compiler.

### 1. Compile AES and Simon decryption circuits

Run the following command to compile the circuits.

```sh
docker run -i -t --rm \
  --mount type=bind,source="$(pwd)/.."/,target=/flexo \
  flexo \
  bash -c "cd /flexo/UPFlexo && ./compile-circuits.sh"
```

It takes around 3 minutes to compile the decryption circuits.
The circuits are roughly 1.6 GB in size.

### 2. Compile the packers

Run the following command to compile the packers.
Note: you need [podman](https://podman.io/), cmake, and Ninja to run this bash script.

```sh
./compile-packers.sh
```

It takes around 20 minutes to compile the packers.
The packers are roughly 681 MB in size.
