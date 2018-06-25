
#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "imu_filter.h"
// #include <time.h>
using namespace std;

ros::Publisher imu_raw_pub_;
ros::Publisher imu_filtered_pub_;
ros::Duration offset(0);
ros::Time last_time_;
bool initialized_ = false;
ImuFilter filter_;
double q0,q1,q2,q3;
double orientation_variance_ = 0;

bool isWhitespace(std::string s)
{
    for(unsigned int index = 0; index < s.length(); index++)
    {
        if(!std::isspace(s[index]))
            return false;
    }
    return true;
}   //ok

void filter_data(sensor_msgs::Imu imu_raw_data)
{
    //sensor_msgs::Imu imu_data = imu_raw_data;
    ros::Time current_time_ = imu_raw_data.header.stamp;
    if(!initialized_)
    {
        ROS_INFO("First Imu message received");
        last_time_ = current_time_;
        initialized_ = true;

    }
    else
    {
        sensor_msgs::Imu imu_data = imu_raw_data;
        imu_data.header.stamp = ros::Time::now();
        imu_data.header.frame_id = "base_imu_filtered";
        geometry_msgs::Vector3& ang_vel = imu_data.angular_velocity;
        geometry_msgs::Vector3& lin_acc = imu_data.linear_acceleration;
        float dt = (current_time_ - last_time_).toSec();
        last_time_ = current_time_;
        std::cout<<"last_time: "<< last_time_ <<endl;
        std::cout<<"dt: "<< dt <<endl;
        filter_.madgwickAHRSupdateIMU(
            ang_vel.x, ang_vel.y, ang_vel.z,
            lin_acc.x, lin_acc.y, lin_acc.z,
            dt);
        filter_.getOrientation(q0,q1,q2,q3);
        imu_data.orientation.w = q0;
        imu_data.orientation.x = q1;
        imu_data.orientation.y = q2;
        imu_data.orientation.z = q3;

        imu_data.orientation_covariance[0] = orientation_variance_;
        imu_data.orientation_covariance[1] = 0.0;
        imu_data.orientation_covariance[2] = 0.0;
        imu_data.orientation_covariance[3] = 0.0;
        imu_data.orientation_covariance[4] = orientation_variance_;
        imu_data.orientation_covariance[5] = 0.0;
        imu_data.orientation_covariance[6] = 0.0;
        imu_data.orientation_covariance[7] = 0.0;
        imu_data.orientation_covariance[8] = orientation_variance_;
        imu_filtered_pub_.publish(imu_data);
    }

}
void get_imu_data()
{
    ifstream file("/dev/ttyACM0");
    string line;
    string t;
    float tf;
    int count = 0 ;
    float acc_scale = 0.000598;
    float gyro_scale = 0.000153;

    sensor_msgs::Imu imu_data;
    imu_data.header.frame_id = "base_imu";
    // imu_data.header.stamp = ros::Time::now();


    while(std::getline(file,line))
    {
        if (!line.empty() && !isWhitespace(line))
        {
            stringstream strs(line);
            // imu_data.header.stamp = ros::Time::now() + offset;
            imu_data.header.stamp = ros::Time::now() ;
            while(getline(strs, t, ' '))
            {
                tf = atof(t.c_str());
                switch(count)
                {
                case 0:
                    // cout<< "IMU timestamp: " << t << endl;
                    break;
                case 1:
                    imu_data.linear_acceleration.x = tf*acc_scale;
                    break;
                case 2:
                    imu_data.linear_acceleration.y = tf*acc_scale;
                    break;
                case 3:
                    imu_data.linear_acceleration.z = tf*acc_scale;
                    break;
                case 4:
                    imu_data.angular_velocity.x = tf*gyro_scale;
                    break;
                case 5:
                    imu_data.angular_velocity.y = tf*gyro_scale;
                    break;
                case 6:
                    imu_data.angular_velocity.z = tf*gyro_scale;
                    break;
                default:
                    break;
                }
                count++;
            }
        }
        count = 0;

        filter_data(imu_data);
        imu_raw_pub_.publish(imu_data);
    }
    file.close();
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "orbbec_imu");
    ros::NodeHandle nh;
    imu_raw_pub_ = nh.advertise<sensor_msgs::Imu>("imu_data_raw", 20);
    imu_filtered_pub_ = nh.advertise<sensor_msgs::Imu>("imu_data_filtered",20); 

    ros::Rate rate(30);
        while ( ros::ok( ) )
        {
            get_imu_data();
            ros::spinOnce();

            rate.sleep();
       }
    // get_imu_data();
    // ros::spin();

    // exit (EXIT_SUCCESS);

    return 0;
}