//
// Created by  jormin on 2021/6/23.
//

#include "GLThreadDispatch.h"
#include <assert.h>
#include "mediapipe/framework/port/logging.h"



using namespace Opipe;


GLThreadDispatch::GLThreadDispatch(std::thread::id glThreadId, DispatchAsyncFunction dispatchAsyncFunction) : _glThreadId(glThreadId), _dispatchAsync(dispatchAsyncFunction) {
}

void GLThreadDispatch::runSync(void *host, std::function<void(void)> func) {
     LOG(INFO) << "######  std::this_thread::get_id()  " << std::this_thread::get_id();
     LOG(INFO) << "######  _glThreadId  " << _glThreadId;
    
    if (std::this_thread::get_id() == _glThreadId) {
        func();
    } else {
        if (_dispatchAsync) {
            LOG(INFO) << "###### _dispatchAsync host:" << host;
            _dispatchAsync(host, func);
        }
    }
}

void GLThreadDispatch::runAsync(void *host, std::function<void(void)> func) {
    LOG(INFO) << "---------------xxxxxxx";
    if (_dispatchAsync) {
        LOG(INFO) << "---------------xxxxxxx22222";
        _dispatchAsync(host, func);
    }
}

