cd /home/luis-kilmer/NeoDev/guitar-visualizer/build
rm -rf *
cmake -DOF_ROOT=/opt/openFrameworks ..
make -j$(nproc)