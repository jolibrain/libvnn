#!/usr/bin/python
# -*- coding: utf-8 -*-
# -*- Mode: Python -*-
# vim:si:ai:et:sw=4:sts=4:ts=4

import sys
import requests
import json
import cv2
import argparse
import os.path
import logging


def predict(service , video_path, img_count =0, dest_dir="/tmp/image_predict"):

    url = "http://localhost:8080/predict"
    data= {}
    data['service'] = service
    data['parameters'] = { 'output' : {
            'bbox': True,
            'confidence_threshold': 0.3
            }}
    data['data'] = [video_path]
    print (json.dumps(data, indent=2, separators=(',',':')))
    r = requests.post(url, data = json.dumps(data), timeout=3)
    if r.status_code == 200:
        ret_data = r.json()
        print (json.dumps(ret_data, indent=2, separators=(',',':')))
        for prediction in ret_data["body"]["predictions"]:
            print ("timestamp = {}".format(prediction["uri"]))
            try:
                img = cv2.imread("/tmp/images/full/img_{}.jpg".format(prediction["uri"]))
            except:
                raise
            for c in prediction["classes"] :
                cat = c['cat']
                bbox = c['bbox']
                cv2.rectangle(img,(int(bbox['xmin']),int(bbox['ymax'])),(int(bbox['xmax']),int(bbox['ymin'])),(255,0,0),2)
                cv2.putText(img,cat,(int(bbox['xmin']),int(bbox['ymax'])),cv2.FONT_HERSHEY_PLAIN,1,255)
            dest_path = os.path.join(dest_dir, "predict_{}.jpeg".format(img_count))
            resizedimg = cv2.resize(img,None,fx=0.5,fy=0.5)
            cv2.imshow('frame',resizedimg)
            cv2.waitKey(10)
            #cv2.imwrite(dest_path, img)
    else :
        print ("error: {} {}".format(r.status_code , r.text))

def create_service(service):
    url = "http://localhost:8080/services/" + service
    data = {
       "mllib":"caffe",
       "description":"object detection service",
       "type":"supervised",
       "parameters":{
         "input":{
           "connector":"video",
	   "height": 300,
	   "width": 300
         },
         "mllib":{
           "nclasses":2,
           "gpu":True,
           "gpuid":1
         }
       },
       "model":{
         "repository":"/home/bugsi/SqueezeNetSSD_300x300/"
       }
     }
    r = requests.put(url, json =data, timeout = 3)

def delete_service(service):
    url = "http://localhost:8080/services/" + service
    r = requests.delete(url, timeout = 3)


def main(argv):
    parser = argparse.ArgumentParser(description='predict on video')
    parser.add_argument('video_path',
            metavar='PATH',
            type = str,
            nargs = "?",
            help = 'Video path' )

    parser.add_argument('-d', '--debug',
            dest='debug', action='store_const',
            const=logging.DEBUG, default=logging.INFO,
            help='debug mode')

    args = parser.parse_args()

    service = "videoserv"
    delete_service(service)
    try:
        create_service(service)
    except:
        raise
    count = 0;
    while True:
        count +=1
        predict(service, args.video_path, count)
        if count > 1000: break

if __name__ == "__main__":
    sys.exit(main(sys.argv))


