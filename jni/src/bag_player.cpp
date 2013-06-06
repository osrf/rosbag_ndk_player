#include "bag_player.hpp"

#define foreach BOOST_FOREACH

namespace rosbag
{

BagPlayer::BagPlayer(const std::string &fname) throw(BagException) {
    bag.open(fname, rosbag::bagmode::Read);
}

BagPlayer::~BagPlayer() {
    bag.close();
}

void BagPlayer::start_play() {

    std::vector<std::string> topics;
    std::pair<std::string, BagCallback *> cb;
    foreach(cb, cbs_)
        topics.push_back(cb.first);

    View view(bag, TopicQuery(topics));

    foreach(MessageInstance const m, view)
    {
        if (cbs_.find(m.getTopic()) == cbs_.end())
            continue;

        //TODO: handle delay

        cbs_[m.getTopic()]->call(m);
    }
}

}

