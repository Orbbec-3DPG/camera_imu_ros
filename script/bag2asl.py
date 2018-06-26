#!/usr/bin/python

# Extract images from a bag file.

import roslib;   
import rosbag
import rospy
import cv2
import sys
import os
import csv
from sensor_msgs.msg import Image
from cv_bridge import CvBridge
from cv_bridge import CvBridgeError


def msg2png(msg, path):

	bridge = CvBridge()
	img_path = path + 'data'
	if not os.path.exists(img_path):
	    os.makedirs(img_path)
	try:
		cv_image = bridge.imgmsg_to_cv2(msg, "bgr8")
	except CvBridgeError as e:
		print e
	timestr = "%s" % msg.header.stamp;
	image_name = img_path + '/'+ timestr + ".png"
	cv2.imwrite(image_name, cv_image)
	print '\rcreating images...'

def img2csvdata(msg,path):
	file_name = path + 'data.csv'
	with open(file_name, "a") as csvfile:
		writer = csv.writer(csvfile)
		if os.stat(file_name).st_size ==0:
			print "file is empty, writing headers..."
			writer.writerow(["#timestamp [ns]", "filename"])
		else:
			pass
		timestr = "%s" % msg.header.stamp;
		writer.writerow([timestr, timestr + '.png'])


def imu2csvdata(msg,path):
	file_name = path + 'data.csv'
	if not os.path.exists(path):
	    os.makedirs(path)
	with open(file_name, "a") as csvfile:
		writer = csv.writer(csvfile)
		if os.stat(file_name).st_size ==0:
			print "file is empty, writing headers..."
			writer.writerow(["#timestamp [ns]", "w_RS_S_x [rad s^-1]", "w_RS_S_y [rad s^-1]", "w_RS_S_z [rad s^-1]", "a_RS_S_x [rad s^-1]", "a_RS_S_y [rad s^-1]", "a_RS_S_z [rad s^-1]"])
		else:
			pass
		timestr = "%s" % msg.header.stamp;
		writer.writerow([timestr, msg.angular_velocity.x, msg.angular_velocity.y, msg.angular_velocity.z, msg.linear_acceleration.x, msg.linear_acceleration.y, msg.linear_acceleration.z])


if __name__ == '__main__':
    # count = 0

    # overwrite data.csv files if they already exit
    answer = raw_input("Do you want to overwrite the current data.csv files ? (yes/no) \n")
    if(answer == 'yes'):
    	for root, dirs, files in os.walk("./"):
    		for filename in files:
    			if filename == 'data.csv':
    				os.remove(os.path.join(root,filename))
    			else:
    				pass
    else:
    	pass
    for topic, msg, t in rosbag.Bag(sys.argv[1]).read_messages():
    	# if count < 10:
	    	if topic == '/cam0/image_raw':
	    		path = './cam0/'
	    		msg2png(msg, path)
	    		img2csvdata(msg, path)

	    	if topic == '/cam1/image_raw':
	    		path = './cam1/'
	    		msg2png(msg, path)
	    		# count+=1

	    	if topic == '/imu0':
	    		path = './imu0/'
	    		imu2csvdata(msg, path)



#TODO 
#verification of csv file to not append  //OK 
#add sensor calibration yaml file to the camera info topic 