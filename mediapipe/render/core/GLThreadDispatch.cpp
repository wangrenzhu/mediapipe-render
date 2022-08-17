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
     LOG(ERROR)<<"######  std::this_thread::get_id()  " << std::this_thread::get_id();
     LOG(ERROR)<<"######  _glThreadId  " << _glThreadId;
    
    if (std::this_thread::get_id() == _glThreadId) {
        func();
    } else {
        assert("not support run sync in gl thread now");
    }
}

void GLThreadDispatch::runAsync(void *host, std::function<void(void)> func) {
    LOG(ERROR)<<"---------------xxxxxxx";
    if (_dispatchAsync) {
        _dispatchAsync(host, func);
    }
}

