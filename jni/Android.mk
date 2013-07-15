LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := rosbag_ndk_player_demo
LOCAL_SRC_FILES := src/test.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_LDLIBS := -landroid
LOCAL_STATIC_LIBRARIES := android_native_app_glue rosbag

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,rosbag_ndk_player)
