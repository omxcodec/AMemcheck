
# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH:= $(call my-dir)

# common
# ========================================================
libamemcheck_common_src_files := \
    libc_logging.cpp

libamemcheck_common_cflags := \
    -D_LIBC=1 \
    -Wall -Wextra

libamemcheck_common_c_includes := \
    $(LOCAL_PATH)/../inc

# ========================================================
# libamemcheck_common.a
# ========================================================

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(libamemcheck_common_src_files)
LOCAL_CFLAGS := $(libamemcheck_common_cflags) -Werror
LOCAL_C_INCLUDES := $(libamemcheck_common_c_includes)
ifdef NDK_ROOT
LOCAL_LDLIBS += -llog
else
LOCAL_SHARED_LIBRARIES += liblog
endif
LOCAL_MODULE := libamemcheck_common

include $(BUILD_STATIC_LIBRARY)

# ========================================================
# libamemcheck.so
# ========================================================
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	dlmalloc.c \
	malloc_debug_common.cpp

LOCAL_CFLAGS := $(libamemcheck_common_cflags)
LOCAL_C_INCLUDES := $(libamemcheck_common_c_includes)
LOCAL_WHOLE_STATIC_LIBRARIES := libamemcheck_common

ifdef NDK_ROOT
LOCAL_LDLIBS += -llog -ldl
else
LOCAL_SHARED_LIBRARIES += liblog libdl
endif

LOCAL_MODULE:= libamemcheck

include $(BUILD_SHARED_LIBRARY)

# ========================================================
# libamemcheck_malloc_debug_leak.so
# ========================================================
include $(CLEAR_VARS)

LOCAL_CFLAGS := \
	$(libamemcheck_common_cflags) \
	-DMALLOC_LEAK_CHECK \
    -D__arm__

LOCAL_C_INCLUDES := $(libamemcheck_common_c_includes)

LOCAL_SRC_FILES := \
	debug_map_info.c \
	debug_symbol_table.c \
	debug_ptrace.c \
	debug_stacktrace.cpp \
	malloc_debug_leak.cpp \
	malloc_debug_check.cpp

LOCAL_SHARED_LIBRARIES := libamemcheck
LOCAL_WHOLE_STATIC_LIBRARIES := libamemcheck_common

ifdef NDK_ROOT
# 问题: 当APK导入STL，但又有cpp文件需要依赖unwind.h头文件，而且使用NDK编译, 如果使用_Unwind_Backtrace函数，
#       可能就无法编译
# 原因：NDK下代码有$(ANDROID_NDK)/sources/cxx-stl/gabi++/include/unwind.h(无_Unwind_Backtrace声明和实现), 
#       它会覆盖掉$(ANDROID_NDK)/toolchains下标准的unwind.h
#       网上说法:
#       1. NDK make-standalone-toolchain.sh --stl=stlport creates compiler package with unwind.h doesn't support _Unwind_Backtrace
#          https://code.google.com/p/android/issues/detail?id=68081
#       2. Issue 357890:  Roll Clang to 204777
#          https://code.google.com/p/chromium/issues/detail?id=357890
# 解决: NDK编译时，unwind.h不要使用$(ANDROID_NDK)sources/cxx-stl下的, 而应该使用$(ANDROID_NDK)toolchains下的
#       相应的，使用unwind.h时写成include "include/unwind.h"，而不是include <unwind.h>
# $(warning "TOOLCHAIN_PREBUILT_ROOT: $(TOOLCHAIN_PREBUILT_ROOT)")
# 查找unwind.h 法1
# recursive wildcard 递归遍历目录下的所有的文件
# Cocos2dx-Android 之Makefile通用高级写法 http://blog.csdn.net/nanlus/article/details/36388549
rwildcard = $(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
TOOLCHAIN_PREBUILT_UNWIND_H :=$(strip $(call rwildcard,$(TOOLCHAIN_PREBUILT_ROOT),unwind.h))
# 查找unwind.h 法2
# TOOLCHAIN_PREBUILT_UNWIND_H := $(strip $(shell find $(TOOLCHAIN_PREBUILT_ROOT) -name unwind.h))
# $(warning "TOOLCHAIN_PREBUILT_UNWIND_H: $(TOOLCHAIN_PREBUILT_UNWIND_H)")
TOOLCHAIN_PREBUILT_GCC_ROOT := $(patsubst %/,%, $(dir $(TOOLCHAIN_PREBUILT_UNWIND_H)))/..
# $(warning "TOOLCHAIN_PREBUILT_GCC_ROOT: $(TOOLCHAIN_PREBUILT_GCC_ROOT)")
LOCAL_CFLAGS += -I$(TOOLCHAIN_PREBUILT_GCC_ROOT) -DUSE_NDK_BUILD
endif

ifdef NDK_ROOT
LOCAL_LDLIBS += -llog -ldl
else
LOCAL_SHARED_LIBRARIES += liblog libdl
endif

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= libamemcheck_malloc_debug_leak

include $(BUILD_SHARED_LIBRARY)

# ========================================================
# libamemcheck_malloc_debug_qemu.so
# ========================================================
ifeq ($(DEBUG_MALLOC_QEMU),true)

include $(CLEAR_VARS)

LOCAL_CFLAGS := \
	$(libamemcheck_common_cflags) \
	-DMALLOC_QEMU_INSTRUMENT

LOCAL_C_INCLUDES := $(libamemcheck_common_c_includes)

LOCAL_SRC_FILES := \
	malloc_debug_qemu.cpp

LOCAL_MODULE:= libamemcheck_malloc_debug_qemu
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/Android.mk

LOCAL_SHARED_LIBRARIES := libamemcheck
LOCAL_WHOLE_STATIC_LIBRARIES := libamemcheck_common

ifdef NDK_ROOT
LOCAL_LDLIBS += -llog
else
LOCAL_SHARED_LIBRARIES += liblog
endif

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)

endif # DEBUG_MALLOC_QEMU

include $(call all-makefiles-under,$(LOCAL_PATH))
