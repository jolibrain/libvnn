# libvnn

## Build instuctions

Depedencies:
```shell
sudo apt-get install build-essential cmake libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev
```
Build with cmake on desktop :

```shell
mkdir build && cd build && cmake ..
make -j
```
Build with cmake ON jetson TX2:

```shell
mkdir build && cd build && cmake .. -DUSE_TX2=ON
make -j
`

## Benchmarking
### example: examples/TX2/c/TX2_video_scale_appsink.cpp
  On TX2
  decodoing to appsink without video syncthonisation for performance measurement

  input: 4K video mp4 h264
  ouput: scaled 300x300 RGBA buffer
  output FPS: 142

### example: examples/TX2/c/tx2_camera_scale_appsink.cpp
  On TX2
  TX2 on board camera, streaming 4K video at 30 fps

  input: TX2 on board CSI camera
  ouput: scaled 300x300 RGBA buffer
  output FPS: 30 (synced with cam)
  ouput: scaled 300x300 RGBA buffer
## Goal
Compare several video/mulimedia tools with accelerated GPU comput
Tools to compare:
- nvvl
- deepstream
- gstreamer on TX2

Operation to compare:  **Video frame downsizing**

Input:
- 4K  / MP4 / duration? / Colorspace? **TBD**
- 2K / MP4 / duration? / Colorspace?

Output:
- 300x300 / raw / RGB **TBD**

Execution boards:
- NVIDIA GTX 1080 (eris)
- Jetson TX2
- ???

