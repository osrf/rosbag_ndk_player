#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <android/log.h>
#include <boost/bind.hpp>
#include <boost/format.hpp>

#include <android_native_app_glue.h>

#include <std_msgs/String.h>
#include <std_msgs/Int32.h>

#include <rosbag/bag.h>

void log(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    __android_log_vprint(ANDROID_LOG_INFO, "RBA", msg, args);
    va_end(args);
}

#define LASTERR strerror(errno)

rosbag::Bag bag;

void open_bag() {
    log("opening bag");
    try {
        bag.open("/sdcard/test.bag", rosbag::bagmode::Write);
    } catch (rosbag::BagException e) {
        log("could not open bag file for writing: %s, %s", e.what(), LASTERR);
        return;
    }
}

void record_message(rosbag::Bag &b) {

    static int count = 0;

    std_msgs::String str;
    str.data = boost::str(boost::format("foo%1%") % count);

    std_msgs::Int32 i;
    i.data = count;

    log("writing stuff into bag");
    try {
        bag.write("chatter", ros::Time::now(), str);
        bag.write("numbers", ros::Time::now(), i);
    } catch (const std::exception &e) {
        log("Oops! could not write to bag: %s, %s", e.what(), strerror(errno));
        return;
    }
}

int32_t handle_event(android_app *app, AInputEvent *e) {
    record_message(bag);
    return 0;
}

void handle_cmd(android_app *app, int32_t c) {
    if (c == APP_CMD_LOST_FOCUS) {
        bag.close();
    }
}

/* android stuff below */

void ev_loop(android_app *papp) {
    int32_t lr;
    int32_t le;
    bool first = true;
    android_poll_source *ps;

    log("starting event loop");
    ros::Time::init();

    while (true) {
        lr = ALooper_pollAll(-1, NULL, &le, (void **) &ps);
        if (lr < 0) {
            break;
        }
        if (ps) {
            log("event received");
            if (first) {
                open_bag();
                papp->onInputEvent = handle_event;
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
