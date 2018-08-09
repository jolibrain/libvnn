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
```

## Benchmarking

### ON Jetston TX2
#### File honeyBees.mp4
  - Duration: 227 seconds
  - Video format:  4096x2304, 24 fps,  h264 (High),  yuv420p 
  - bitrate: 22277 kb/s
  
#### File bbb_60.mp4
  - Duration: 60 seconds
  - Video format: 3840x2160, 30 fps,  h264 (High 4:4:4P),  yuv420p 
  - bitrate: 338450 kb/s


|  File     | Bitrate     | Decoding ratio | FPS   |
| ------    | --------    | ---------------| ----- |
| honeyBees |  22,2 Mb/s  | 5.89           | 141   |
| bbb_60    |  338,4 Mb/s | 1.93           | 58    |

  


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

