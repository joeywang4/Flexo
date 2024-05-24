# Reproducing the results in our paper

Follow the following instructions if you want to reproduce the results in our paper.

## 1. Compile every circuits

We provide a script to compile every circuits: `compile-all.sh`.
To execute this script using the installed Flexo docker image, run the following command:

```sh
docker run -i -t --rm \
  --mount type=bind,source="$(pwd)/.."/,target=/flexo \
  flexo \
  bash -c "cd /flexo/reproduce && ./compile-all.sh"
```

It takes roughly 40 mintes to compile every circuits.
