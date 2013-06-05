#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <android/log.h>
#include "rosbag/bag.h"
#include "rosbag/view.h"

#include "std_msgs/String.h"
#include "std_msgs/Int32.h"

#include <boost/foreach.hpp>

#include <android_native_app_glue.h>

void log(const char *msg, ...) {
    va_list args;
    va_start(args, msg);
    __android_log_vprint(ANDROID_LOG_INFO, "RBA", msg, args);
    va_end(args);
}

#define LASTERR strerror(errno)
#define foreach BOOST_FOREACH

namespace rosbag
{
    class BagPlayer
    {
    public:
        BagPlayer(const std::string &filename);

        template<class T>
            void   register_callback(const std::string &topic,
                    boost::function<void (boost::shared_ptr<T> &)> f);

        void   start_play();

    private:
        Bag bag_;
        std::map<std::string, BagCallback *> cbs_;
    };

    BagPlayer::BagPlayer(const std::string &topic) {
        //TODO: open bag
    }

    template<T>
    void   BagPlayer::register_callback(const std::string &topic,
            boost::function<void (boost::shared_ptr<T> &)> f) {
        cbs_[topic] = new BagCallbackT<T>(f);
    }

    void BagPlayer::start_play() {

        std::vector<std::string> topic;
        foreach(std::pair<std::string, BagCallback *> const cb, cbs_)
            topics.push_back(me.first);

        View view(bag, TopicQuery(topics));

        foreach(rosbag::MessageInstance const m, view)
        {
            //TODO: handle delay, and pass message to appropriate callback
        }
    }

    struct BagCallback
    {
        virtual void call(MessageInstance m) = 0;
    };

    template<T>
        class BagCallbackT : BagCallback
    {
        public:
            BagCallback(boost::function<void (boost::shared_ptr<T> &)> f) :
                cb_(f)
        {}

            void call(MessageInstance m) {
                cb_(m.instantiate<T>());
            }

        private:
            boost::function<void (boost::shared_ptr<T> &)> cb_;
    };

}

void chatters_callback(std_msgs::StringPtr &s) {
    log("chatter message played: %s", s->data.c_str());
}

void numbers_callback(std_msgs::Int32Ptr &i) {
    log("numbers message played: %d", i->data);
}

void play_bag() {
    rosbag::BagPlayer bag_player("/sdcard/test.bag");

    bag_player.register_callback<std_msgs::Int32>("numbers",
            boost::bind(numbers_callback, _1));
    bag_player.register_callback<std_msgs::String>("chatter",
            boost::bind(chatters_callback, _1));

    bag_player.start_play();
}


/* android stuff below */

void ev_loop(android_app *papp) {
    int32_t lr;
    int32_t le;
    bool first = true;
    android_poll_source *ps;

    app_dummy();

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
    ev_loop(papp);
}
