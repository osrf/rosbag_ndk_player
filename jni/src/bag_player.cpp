#include "bag_player.hpp"

#define foreach BOOST_FOREACH

namespace rosbag
{

BagPlayer::BagPlayer(const std::string &fname) throw(BagException) {
    bag.open(fname, rosbag::bagmode::Read);
    ros::Time::init();
    View v(bag);
    bag_start_ = v.getBeginTime();
    bag_end_ = v.getEndTime();
    last_message_time_ = ros::Time(0);
    time_scale_ = 1.0;
}

BagPlayer::~BagPlayer() {
    bag.close();
}

ros::Time BagPlayer::get_time() {
    return last_message_time_;
}

void BagPlayer::set_start(const ros::Time &start) {
    bag_start_ = start;
}

void BagPlayer::set_end(const ros::Time &end) {
    bag_end_ = end;
}

void BagPlayer::set_time_scale(double scale) {
    time_scale_ = scale;
}

ros::Time BagPlayer::real_time(const ros::Time &msg_time) {
    return play_start_ + (msg_time - bag_start_) * time_scale_;
}

void BagPlayer::start_play() {

    std::vector<std::string> topics;
    std::pair<std::string, BagCallback *> cb;
    foreach(cb, cbs_)
        topics.push_back(cb.first);

    View view(bag, TopicQuery(topics), bag_start_, bag_end_);
    play_start_ = ros::Time::now();

    foreach(MessageInstance const m, view)
    {
        if (cbs_.find(m.getTopic()) == cbs_.end())
            continue;

        ros::Time::sleepUntil(real_time(m.getTime()));

        last_message_time_ = m.getTime(); /* this is the recorded time */
        cbs_[m.getTopic()]->call(m);
    }
}

void BagPlayer::unregister_callback(const std::string &topic) {
    delete cbs_[topic];
    cbs_.erase(topic);
}

}

