//
//  OpipeDispatch.hpp
//  Opipe
//
//  Created by wangrenzhu2021 on 2021/12/14.
//  Copyright © 2021 ola. All rights reserved.
//

#ifndef OpipeDispatch_hpp
#define OpipeDispatch_hpp

#include <stdio.h>
#include <condition_variable>

#include "Context.hpp"
#include "GLThreadDispatch.h"
#include <thread>
class dispatch_queue;

namespace Opipe {
    class OpipeDispatch {
    public:
        void flushSharedInstance();
        
    public:
        void runSync(std::function<void(void)> func, Context::ContextType type = Context::GPUImageContext);
        void runAsync(std::function<void(void)> func, Context::ContextType type = Context::GPUImageContext,
                      bool async = true);

        void setGLThreadDispatch(GLThreadDispatch *glDispatch){
            _glThreadDispatch = glDispatch;
        }


        OpipeDispatch(Context *context,
                           void *id = nullptr,
                           GLThreadDispatch *glDispatch = nullptr);

        ~OpipeDispatch();

    private:
    #if defined(__APPLE__)
        dispatch_queue* _contextQueue;
    #endif
        dispatch_queue* _contextQueueOffline;
        dispatch_queue* _contextQueueIO;
        GLThreadDispatch* _glThreadDispatch = nullptr;
        std::thread::id _glThreadId;
        void* _id ;
    public:
        Context *_context;
    };
}

#endif /* OpipeDispatch_hpp */
