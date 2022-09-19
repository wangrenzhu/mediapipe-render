/*
 * GPUImage-x
 *
 * Copyright (C) 2017 Yijin Wang, Yiqian Wang
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#if defined(__APPLE__)
#import <Foundation/Foundation.h>
#elif defined(__ANDROID__)
#include <android/log.h>
#endif


#include "GPUImageUtil.h"
#if DEBUG
#define openLog 1
#else
#define openLog 0
#endif
#define ANDROID_LOG_INFO 'gpu_log_info'

#include "mediapipe/framework/port/logging.h"

namespace Opipe {


    __attribute__((no_sanitize("address", "memory")))
    std::string  str_format(const char *fmt, ... )
    {

        char* buffer = new char[40240];
        va_list args;
        va_start(args, fmt);
        vsprintf(buffer, fmt, args);
        va_end(args);
        std::string strResult(buffer);
        delete[] buffer;
        return strResult;
    }

    void Log(const std::string &tag, const std::string &format, ...) {
        char buffer[10240];
        va_list args;
        va_start(args, format);
        vsprintf(buffer, format.c_str(), args);
        va_end(args);

        LOG(INFO) << tag << ": " << buffer;
    }

    /**
     * 总是会输出日志，ERROR级别的日志
     */
    void LogE(const std::string &tag, const std::string &format, ...) {
        char buffer[10240];
        va_list args;
        va_start(args, format);
        vsprintf(buffer, format.c_str(), args);
        va_end(args);

        LOG(ERROR) << tag << ": " << buffer;
    }

}
