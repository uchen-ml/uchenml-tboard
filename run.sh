bazel build //...

rm -rf ./tboard
mkdir ./tboard
cp ./bazel-bin/src/example_util/example_util.runfiles/_main/src/example_util/example_util tboard/example_util

mkdir ./tboard/logs

cd ./tboard

./example_util logs/events.out.tfevents

tensorboard --logdir=logs
cd ..

rm -rf tboard
