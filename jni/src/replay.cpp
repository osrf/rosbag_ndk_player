#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <android/log.h>
#include <boost/bind.hpp>

#include <android_native_app_glue.h>

#include <std_msgs/String.h>
#include <std_msgs/Int32.h>

#include "bag_player.hpp"

rosbag::BagPlayer *bp;

void log(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    __android_log_vprint(ANDROID_LOG_INFO, "RBA", msg, args);
    va_end(args);
}

#define LASTERR strerror(errno)

void chatters_callback(const std_msgs::String::ConstPtr& s) {
    log("chatter message played: %s", s->data.c_str());
    ros::Time t = bp->get_time();
    log("time: %fs", t.toSec());
}

void numbers_callback(const std_msgs::Int32::ConstPtr& i) {
    log("numbers message played: %d", i->data);
    ros::Time t = bp->get_time();
    log("time: %fs", t.toSec());
}

void play_bag() {
    try {
        rosbag::BagPlayer bag_player("/sdcard/test.bag");
        bp = &bag_player;

        bag_player.register_callback<std_msgs::Int32>("numbers",
                boost::bind(numbers_callback, _1));
        bag_player.register_callback<std_msgs::String>("chatter",
                boost::bind(chatters_callback, _1));

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
                play_bag();
                first = false;
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
