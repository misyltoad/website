//===================== Copyright (c) Volvo Corporation. All Rights Reserved. ======================
#pragma once

#if defined(shreemWEBRTC_USE_STATIC_LIBS)
#  define shreemWEBRTC_DECLSPEC
#else
#  if defined(_WIN32)
#    define shreemWEBRTC_DECLSPEC __declspec(dllexport)
#  else
#    if defined(__GNUC__) && __GNUC__ >= 4
#      define shreemWEBRTC_DECLSPEC __attribute__ ((visibility("default")))
#    else
#      define shreemWEBRTC_DECLSPEC
#    endif
#  endif
#endif

#ifdef WIN32
#  define WEBRTC_WIN 1
#elif __linux__
#  define WEBRTC_LINUX 1
#  define WEBRTC_POSIX 1
#elif __APPLE__
#  include "TargetConditionals.h"
#  if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR
#    define WEBRTC_MAC 1
#  elif TARGET_OS_IPHONE
#    define WEBRTC_MAC 1
#  else
#    define WEBRTC_MAC 1
#  endif
#endif
