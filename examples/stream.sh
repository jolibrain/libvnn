#!/bin/bash

export GST_DEBUG=3

SRC="/home/sileht/Videos/simpsons/The.Simpsons.S25E06.FRENCH.Web-Dl/The.Simpsons.S25E06.FRENCH.Web-Dl.avi"

CAPS="application/x-rtp,media=(string)video,clock-rate=(int)90000,encoding-name=(string)H264,payload=(int)96"
SERVER=10.44.44.3
CLIENT=10.44.44.7
RTP_PORT=5000
RTCP_PORT_SERVER=5001
RTCP_PORT_CLIENT=5002

if [ "$1" == "server" ] ; then

gst-launch-1.0 -tv \
    rtpbin name=r \
    filesrc location="$SRC" ! decodebin ! x264enc tune=zerolatency ! rtph264pay ! $CAPS ! r.send_rtp_sink_0 \
    r.send_rtp_src_0 ! udpsink host=$CLIENT port=$RTP_PORT \
    r.send_rtcp_src_0 ! udpsink host=$CLIENT port=$RTCP_PORT_CLIENT bind-port=$RTCP_PORT_SERVER sync=false async=false \
    udpsrc port=$RTCP_PORT_SERVER ! tee name=t \
      t. ! queue ! r.recv_rtcp_sink_0 \
      t. ! queue ! fakesink dump=true async=false

else

gst-launch-1.0 -tv \
      rtpsession name=r \
      udpsrc port=$RTP_PORT caps="$CAPS" ! r.recv_rtp_sink \
        r.recv_rtp_src ! rtph264depay ! decodebin ! videoconvert ! xvimagesink \
      udpsrc port=$RTCP_PORT_CLIENT ! r.recv_rtcp_sink \
        r.send_rtcp_src ! udpsink host=$SERVER port=$RTCP_PORT_SERVER bind-port=$RTCP_PORT_CLIENT sync=false async=false
fi
