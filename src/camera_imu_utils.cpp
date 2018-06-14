
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fstream>
#include <cassert>

#include <asm/types.h>          /* for videodev2.h */

#include <linux/videodev2.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>
#include <thread>

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/Imu.h>

#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480
#define CLEAR(x) memset (&(x), 0, sizeof (x))

using namespace std;
using namespace cv;

image_transport::Publisher  image_pub;
ros::Publisher imu_pub;

Mat cameraFrame (IMAGE_HEIGHT,IMAGE_WIDTH, CV_8UC1);
typedef enum {
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
} io_method;

struct buffer {
        void *  start;
        size_t  length;
};


static char *dev_name = NULL;
static char *imu_port = NULL;
//static io_method io = IO_METHOD_MMAP;
static int fd = -1;
struct buffer *buffers = NULL;
static unsigned int  n_buffers = 0;


static void errno_exit(const char * s)
{
        fprintf (stderr, "%s error %d, %s\n",s, errno, strerror (errno));
        exit (EXIT_FAILURE);
}
static int xioctl (int fd, int  request,  void * arg)
{
        int r;
        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);
        return r;
}

bool isWhitespace(std::string s)
{
    for(unsigned int index = 0; index < s.length(); index++)
    {
        if(!std::isspace(s[index]))
            return false;
    }
    return true;
}   //ok
static void process_image(struct buffer *pbuffer)
{
//    unsigned char y2,u,v;
    unsigned char y1;
    unsigned char *data=(unsigned char *)pbuffer->start;
    uint64_t camera_timestamp = *(uint64_t*)(pbuffer->start);
    if(pbuffer->length == IMAGE_HEIGHT*IMAGE_WIDTH *2)
    {
        // cout<<"Camera timestamp: "<<*(uint64_t*)(pbuffer->start) <<endl;
        for(int i=0;i<IMAGE_HEIGHT;i++)
        {
            uchar *m_data = cameraFrame.ptr<uchar>(i);
            for(int j=0;j<IMAGE_WIDTH;j++)
            {
                y1 = *( data + (i*IMAGE_WIDTH+j)*2);

                m_data[j] = y1;
            }
        }
        uchar *firstRow = cameraFrame.ptr<uchar>(0);
        uchar *secondRow  = cameraFrame.ptr<uchar>(1);
        for(int i=0;i<4;i++)
            firstRow[i] = secondRow[i];
        ros::Time timestamp = ros::Time::now();
        if( image_pub.getNumSubscribers() > 0 )
        {
            cv_bridge::CvImage image_msg;
            image_msg.header.frame_id = "base_camera";
            image_msg.header.stamp = timestamp;
            image_msg.encoding = "mono8";
            image_msg.image = cameraFrame;
            sensor_msgs::Image image_msgs;
            image_msg.toImageMsg(image_msgs);
            image_pub.publish(image_msgs);
        }


    }
    else
    {
        cerr<<"Wrong frame read!"<<endl;
        exit(EXIT_FAILURE);
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
    imu_data.header.stamp = ros::Time::now();
    imu_data.header.frame_id = "base_imu";


    while(std::getline(file,line))
    {
        if (!line.empty() && !isWhitespace(line))
        {
            stringstream strs(line);
            while(getline(strs, t, ' '))
            {
                tf = atof(t.c_str());
                switch(count)
                {
                case 0:
                    cout<< "IMU timestamp: " << t << endl;
                    break;
                case 1:
                    // cout<< "acc_x: " << tf*acc_scale << endl;
                    imu_data.linear_acceleration.x = tf*acc_scale;
                    break;
                case 2:
                    // cout<< "acc_y: " << tf*acc_scale << endl;
                    imu_data.linear_acceleration.y = tf*acc_scale;
                    break;
                case 3:
                    // cout<< "acc_z: " << tf*acc_scale << endl;
                    imu_data.linear_acceleration.z = tf*acc_scale;
                    break;
                case 4:
                    // cout<< "gyro_x: " << tf*gyro_scale << endl;
                    imu_data.angular_velocity.x = tf*gyro_scale;
                    break;
                case 5:
                    // cout<< "gyro_y: " << tf*gyro_scale  << endl;
                    imu_data.angular_velocity.y = tf*gyro_scale;
                    break;
                case 6:
                    // cout<< "gyro_z: " << tf*gyro_scale  << endl;
                    imu_data.angular_velocity.z = tf*gyro_scale;
                    break;
                default:
                    break;
                }
//                cout<<t<< endl;
                count++;
            }
        }
        count = 0;

        
        imu_pub.publish(imu_data);
    }
    exit(1);
}

static int read_frame(void)
{
    struct v4l2_buffer buf;
        CLEAR (buf);
                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;

            if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
                    switch (errno) {
                    case EAGAIN:
                            return 0;

            case EIO:
                /* Could ignore EIO, see spec. */

                /* fall through */

            default:
                errno_exit ("VIDIOC_DQBUF");
            }
        }
               assert (buf.index < n_buffers);
            process_image (&buffers[buf.index]);

        if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
            errno_exit ("VIDIOC_QBUF");

    return 1;
}
static void mainloop (void)
{
//    unsigned int count;
//        count = true;
        while (true) {
                for (;;) {
                        fd_set fds;
                        struct timeval tv;
                        int r;

                        FD_ZERO (&fds);
                        FD_SET (fd, &fds);

                        /* Timeout. */
                        tv.tv_sec = 2;
                        tv.tv_usec = 0;

                        r = select (fd + 1, &fds, NULL, NULL, &tv);

                        if (-1 == r) {
                                if (EINTR == errno)
                                        continue;
                                errno_exit ("select");
                        }
                        if (0 == r) {
                                fprintf (stderr, "select timeout\n");
                                exit (EXIT_FAILURE);
                        }

            if (read_frame ())
                            break;

            /* EAGAIN - continue select loop. */
                }
        }
}

static void init_mmap(void)
{
    struct v4l2_requestbuffers req;

        CLEAR (req);

        req.count               = 4;
        req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory              = V4L2_MEMORY_MMAP;

    if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) {
                if (EINVAL == errno) {
                        fprintf (stderr, "%s does not support "
                                 "memory mapping\n", dev_name);
                        exit (EXIT_FAILURE);
                } else {
                        errno_exit ("VIDIOC_REQBUFS");
                }
        }

        if (req.count < 2) {
                fprintf (stderr, "Insufficient buffer memory on %s\n",
                         dev_name);
                exit (EXIT_FAILURE);
        }

        buffers = (buffer *)calloc (req.count, sizeof (*buffers));

        if (!buffers) {
                fprintf (stderr, "Out of memory\n");
                exit (EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                struct v4l2_buffer buf;

                CLEAR (buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = n_buffers;

                if (-1 == xioctl (fd, VIDIOC_QUERYBUF, &buf))
                        errno_exit ("VIDIOC_QUERYBUF");

                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start =
                        mmap (NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd, buf.m.offset);
                if (MAP_FAILED == buffers[n_buffers].start)
                        errno_exit ("mmap");
        }
}
static void open_device(void)
{
    struct stat st;
    if (-1 == stat (dev_name, &st))
    {
        fprintf (stderr, "Cannot identify '%s': %d, %s\n",
                 dev_name, errno, strerror (errno));
        exit (EXIT_FAILURE);
    }

    if (!S_ISCHR (st.st_mode))
    {
        fprintf (stderr, "%s is no device\n", dev_name);
        exit (EXIT_FAILURE);
    }

    fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);
    if (-1 == fd)
    {
        fprintf (stderr, "Cannot open '%s': %d, %s\n",
                 dev_name, errno, strerror (errno));
        exit (EXIT_FAILURE);
    }

}

static void init_device(void)
{
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
    unsigned int min;

    if (-1 == xioctl (fd, VIDIOC_QUERYCAP, &cap))
    {
        if (EINVAL == errno)
        {
            fprintf (stderr, "%s is no V4L2 device\n",dev_name);
            exit (EXIT_FAILURE);
        }
        else
        {
            errno_exit ("VIDIOC_QUERYCAP");
        }
    }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
        {
            fprintf (stderr, "%s is no video capture device\n",
                     dev_name);
            exit (EXIT_FAILURE);
        }
        CLEAR (cropcap);

                cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

                if (0 == xioctl (fd, VIDIOC_CROPCAP, &cropcap)) {
                        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        crop.c = cropcap.defrect; /* reset to default */

                        if (-1 == xioctl (fd, VIDIOC_S_CROP, &crop)) {
                                switch (errno) {
                                case EINVAL:
                                        /* Cropping not supported. */
                                        break;
                                default:
                                        /* Errors ignored. */
                                        break;
                                }
                        }
                } else {
                        /* Errors ignored. */
                }


                CLEAR (fmt);

                fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                fmt.fmt.pix.width       = IMAGE_WIDTH;
                fmt.fmt.pix.height      = IMAGE_HEIGHT;
                fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
                fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

                if (-1 == xioctl (fd, VIDIOC_S_FMT, &fmt))
                        errno_exit ("VIDIOC_S_FMT");

                /* Note VIDIOC_S_FMT may change width and height. */

            /* Buggy driver paranoia. */
            min = fmt.fmt.pix.width ;
//            if (fmt.fmt.pix.bytesperline < min)
                fmt.fmt.pix.bytesperline = min;
            min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
//            if (fmt.fmt.pix.sizeimage < min)
                fmt.fmt.pix.sizeimage = min;

init_mmap ();
}


static void start_capturing(void)
{
    unsigned int i;
    enum v4l2_buf_type type;

    for (i = 0; i < n_buffers; ++i) {
                struct v4l2_buffer buf;

            CLEAR (buf);

            buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory      = V4L2_MEMORY_MMAP;
            buf.index       = i;

            if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
                        errno_exit ("VIDIOC_QBUF");
    }

    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (-1 == xioctl (fd, VIDIOC_STREAMON, &type))
        errno_exit ("VIDIOC_STREAMON");

}


static void uninit_device(void)
{
    unsigned int i;
    for (i = 0; i < n_buffers; ++i)
        if (-1 == munmap (buffers[i].start, buffers[i].length))
            errno_exit ("munmap");

    free (buffers);
}

static void close_device(void)
{
        if (-1 == close (fd))
            errno_exit ("close");

        fd = -1;
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "orbbec_camera");
    ros::NodeHandle nh;
    image_transport::ImageTransport it(nh);
    image_pub = it.advertise("/camera/image_raw",10);
    imu_pub = nh.advertise<sensor_msgs::Imu>("IMU_data", 20); 

    dev_name = "/dev/video2";
    imu_port = "/dev/ttyACM0";
    open_device();
    init_device();
    start_capturing ();

    // thread t1{mainloop};
    // thread t2{get_imu_data};
    // t1.join();
    // t2.join();

    ros::Rate rate(30);
        while ( ros::ok( ) )
        {
            mainloop();
            // get_imu_data();
            ros::spinOnce();

            rate.sleep();
       }

    uninit_device ();
    close_device ();
    exit (EXIT_SUCCESS);

    return 0;
}
