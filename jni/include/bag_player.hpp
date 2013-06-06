/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2013, Open Source Robotics Foundation
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of Willow Garage, Inc. nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

#ifndef ROSBAG_BAG_PLAYER_H
#define ROSBAG_BAG_PLAYER_H

#include <boost/foreach.hpp>

#include "rosbag/bag.h"
#include "rosbag/view.h"

namespace rosbag
{

struct BagCallback
{
    virtual void call(MessageInstance m) = 0;
};

template<class T>
class BagCallbackT : public BagCallback
{
public:
    typedef boost::function<void (boost::shared_ptr<T>)> Callback;

    BagCallbackT(Callback cb) :
        cb_(cb)
    {}

    void call(MessageInstance m) {
        cb_(m.instantiate<T>());
    }

private:
    Callback cb_;
};

class BagPlayer
{
public:
    BagPlayer(const std::string &filename) throw(BagException);
    template<class T>
        void   register_callback(const std::string &topic,
                typename BagCallbackT<T>::Callback f);
    void   start_play();
    virtual ~BagPlayer();

    Bag bag;

private:
    std::map<std::string, BagCallback *> cbs_;
};

template<class T>
void BagPlayer::register_callback(const std::string &topic,
        typename BagCallbackT<T>::Callback cb) {
    cbs_[topic] = new BagCallbackT<T>(cb);
}

}

#endif