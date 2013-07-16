#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <time.h>
#include <android/log.h>
#include <boost/bind.hpp>

#include <android_native_app_glue.h>

#include <sensor_msgs/Image.h>
#include <sensor_msgs/Imu.h>

#include "rosbag/bag_player.h"

rosbag::BagPlayer *bp;

void log(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    __android_log_vprint(ANDROID_LOG_INFO, "RBA", msg, args);
    va_end(args);
}



// from android samples
/* return current time in seconds */
static double now(void) {

  struct timespec res;
  clock_gettime(CLOCK_REALTIME, &res);
  return res.tv_sec + (double) res.tv_nsec / 1e9;

}


#define LASTERR strerror(errno)

void imu_callback(const sensor_msgs::Imu::ConstPtr& s) {
  log("sensor_msgs::Imu recived on /imu with time %s", s->header.stamp.toSec());
  ros::Time t = bp->get_time();
  log("time: %f realtime %g", t.toSec(), now());
}

void image_callback(const sensor_msgs::Image::ConstPtr& i) {
  log("sensor_msgs::Image on /images with time %g played:", i->header.stamp.toSec());
  ros::Time t = bp->get_time();
  log("time: %f realtime %g", t.toSec(), now());
}

void testbag() {
    rosbag::Bag bag;

    log("initializing time");
    ros::Time::init();

    log("opening bag");
    try {
        bag.open("/sdcard/test2.bag", rosbag::bagmode::Write);
    } catch (rosbag::BagException e) {
        log("could not open bag file for writing: %s, %s", e.what(), LASTERR);
        return;
    }

    sensor_msgs::Image image;
    image.height = 480;
    image.width = 640;



    sensor_msgs::Imu imu;
    for (uint i = 0; i < 9; i++)
    {
      imu.orientation_covariance[i] = (double)i+1.0;
    }

    log("writing stuff into bag");
    try {

      for (double t = 0; t < 100; t++) {
        ros::Time s = ros::Time::now() + ros::Duration().fromSec(t/30.0);
        image.header.stamp = s;
        bag.write("images", s , image);
        bag.write("imu", s, imu);
      }
    } catch (const std::exception &e) {
        log("Oops! could not write to bag: %s, %s", e.what(), strerror(errno));
        return;
    }

    log("closing bag");
    bag.close();
}


void play_bag() {
    try {
        rosbag::BagPlayer bag_player("/sdcard/test2.bag");
        bp = &bag_player;

        bag_player.register_callback<sensor_msgs::Image>("images",
                boost::bind(image_callback, _1));
        bag_player.register_callback<sensor_msgs::Imu>("imu",
                boost::bind(imu_callback, _1));

        bag_player.start_play();
        
        bag_player.set_time_scale(0.5);
        bag_player.start_play();
        bag_player.set_time_scale(2.0);

        bag_player.start_play();
    } catch (rosbag::BagException e) {
        log("error while replaying: %s, %s", e.what(), LASTERR);
        return;
    }
}

/* android stuff below */

void ev_loop(android_app *papp) {
    int32_t lr;
    int32_t le;
    bool first = true;
    bool second = false;
    android_poll_source *ps;

    log("starting event loop");

    while (true) {
        lr = ALooper_pollAll(-1, NULL, &le, (void **) &ps);
        if (lr < 0) {
            break;
        }
        if (ps) {
            log("event received");
            if (first) {
                testbag();
                first = false;
                second = true;
            }
            if (second) {
                log("ready? launching rosbag read!");
                play_bag();
                second = false;
            }
            ps->process(papp, ps);
        }
        if (papp->destroyRequested) {
            log("quitting event loop");
            return;
        }
    }
}

void android_main(android_app *papp) {
    app_dummy();
    ev_loop(papp);
}
