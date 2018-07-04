
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
#include <dirent.h>

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
#define BUFFSIZE 1024

using namespace std;
using namespace cv;

image_transport::Publisher  image_pub;
ros::Publisher imu_pub;

Mat cameraFrame (IMAGE_HEIGHT,IMAGE_WIDTH, CV_8UC1);
typedef enum 
{
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
} io_method;

struct buffer 
{
    void *  start;
    size_t  length;
};

char *dev_name = NULL;
static char *imu_port = NULL;
static int fd = -1;
struct buffer *buffers = NULL;
static unsigned int  n_buffers = 0;
bool idCheck = false;
string OrbbecIdVendor = "2bc5";
string OrbbecIdProduct = "0f01";

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

    if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) 
    {
        switch (errno) 
        {
            case EAGAIN:
                return 0;

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
    while (1) 
    {
        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO (&fds);
        FD_SET (fd, &fds);

        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        r = select (fd + 1, &fds, NULL, NULL, &tv);

        if (-1 == r) 
        {
            if (EINTR == errno)
                    continue;
            errno_exit ("select");
        }
        if (0 == r) 
        {
            fprintf (stderr, "select timeout\n");
            exit (EXIT_FAILURE);
        }

        if (read_frame ())
            break;
    }
        
}

static void init_mmap(void)
{
    struct v4l2_requestbuffers req;
    CLEAR (req);

    req.count = 4;
    req.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) 
    {
        if (EINVAL == errno) 
        {
                fprintf (stderr, "%s does not support "
                         "memory mapping\n", dev_name);
                exit (EXIT_FAILURE);
        } 
        else 
        {
                errno_exit ("VIDIOC_REQBUFS");
        }
    }

    if (req.count < 2) 
    {
        fprintf (stderr, "Insufficient buffer memory on %s\n",
                 dev_name);
        exit (EXIT_FAILURE);
    }

    buffers = (buffer *)calloc (req.count, sizeof (*buffers));

    if (!buffers) 
    {
        fprintf (stderr, "Out of memory\n");
        exit (EXIT_FAILURE);
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) 
    {
        struct v4l2_buffer buf;

        CLEAR (buf);

        buf.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index  = n_buffers;

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

    if (0 == xioctl (fd, VIDIOC_CROPCAP, &cropcap)) 
    {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        crop.c = cropcap.defrect; /* reset to default */

        if (-1 == xioctl (fd, VIDIOC_S_CROP, &crop)) 
        {
            switch (errno) 
            {
                case EINVAL:
                    /* Cropping not supported. */
                    break;
                default:
                    /* Errors ignored. */
                    break;
            }
        }
    }
 
    CLEAR (fmt);

    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = IMAGE_WIDTH;
    fmt.fmt.pix.height      = IMAGE_HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

    if (-1 == xioctl (fd, VIDIOC_S_FMT, &fmt))
            errno_exit ("VIDIOC_S_FMT");

    min = fmt.fmt.pix.width ;
    fmt.fmt.pix.bytesperline = min;
    min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
    fmt.fmt.pix.sizeimage = min;

    init_mmap ();
}


static void start_capturing(void)
{
    enum v4l2_buf_type type;

    for (int i = 0; i < n_buffers; ++i) {
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


vector<string> getFiles(string cate_dir)
{
    vector<string> files;
    DIR *dir;
    struct dirent *ptr;

    if ((dir=opendir(cate_dir.c_str())) == NULL)
    {
    cout<< ("Open dir error...") <<endl;
            exit(1);
    }

    while ((ptr=readdir(dir)) != NULL)
    {
        if (ptr->d_type == 10)
        {
        files.push_back(ptr->d_name);
        }
        else
            continue;
    }
    closedir(dir);
    sort(files.begin(), files.end());
    return files;

}

string get_device_name (string path)
{
    string filename = path +"name";
    ifstream device_name {filename};
    string line;
    std::getline(device_name, line);
    return line;
}

string get_usb_bus_id(string video4linux_path)
{
    ssize_t read_symlink;
    char buf[BUFFSIZE];
    cout<< "video4linux_path" << video4linux_path << endl;
    read_symlink = readlink((char*)video4linux_path.c_str(),buf,BUFFSIZE);
    string bus_id(buf +9, buf+12);
    return bus_id;
}


bool check_vendor_id(string camera_class_path)
{
    string id_vendor_file = camera_class_path +"/idVendor";
    string id_product_file = camera_class_path +"/idProduct";
    ifstream id_vendor_name {id_vendor_file};
    string id_vendor;
    std::getline(id_vendor_name, id_vendor);
    string id_product;
    ifstream id_product_name {id_product_file};
    std::getline(id_product_name, id_product);
    if (id_vendor == OrbbecIdVendor && id_product == OrbbecIdProduct)
        return true;
    else
        return false;
}

string get_orbbec_camera_path ()
{
    string videoFilePath = "/sys/class/video4linux/";
//    string imuFilePath = "/sys/class/tty/";
    string Orbbec_name = "ORBBEC";

    vector<string> files = getFiles(videoFilePath);
    for (int i = 0; i< files.size(); i++)
    {
        string videoFileName = videoFilePath + files[i] + "/";
        string video_device_name = get_device_name(videoFileName);
        string bus_id = get_usb_bus_id(videoFileName+"device");
        string usb_bus_path = "/sys/bus/usb/devices/";
        bool id_check = check_vendor_id(usb_bus_path+bus_id);
        idCheck = &id_check;

        if (video_device_name.find(Orbbec_name) != string::npos && idCheck)
        {
            cout << "Found Orbbec Camera:"<< endl;
            cout << video_device_name<< endl;
            cout << "port: /dev/" << files[i] <<endl;
            return "/dev/"+files[i];
        }

        else
        {
            if (i = files.size() -1)
            {
            cout << "Orbbec Camera not found...\n" << "exiting...\n";
            exit(1);
            }
        }
    }

}

void get_orbbec_camera_dev()
{
    const char* tmp_dev_name = get_orbbec_camera_path().c_str();
    int length = strlen(tmp_dev_name);
    dev_name = new char[length + 1];
    strcpy(dev_name,tmp_dev_name);
}



int main(int argc, char** argv)
{
    ros::init(argc, argv, "orbbec_camera");
    ros::NodeHandle nh;
    image_transport::ImageTransport it(nh);
    image_pub = it.advertise("/camera/image_raw",10);
    imu_pub = nh.advertise<sensor_msgs::Imu>("IMU_data", 20); 

    // dev_name = "/dev/video2";
    // imu_port = "/dev/ttyACM0"; 

    get_orbbec_camera_dev();
    open_device();
    init_device();
    start_capturing ();

    ros::Rate rate(30);
    while (ros::ok())
    {
        mainloop();
        ros::spinOnce();
        rate.sleep();
    }

    uninit_device ();
    close_device ();
    return 0;
}
