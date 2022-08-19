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

    if (std::this_thread::get_id() == _glThreadId) {
        func();
    } else {
        if (_dispatchAsync) {
            _dispatchAsync(host, func);
        }
    }
}

void GLThreadDispatch::runAsync(void *host, std::function<void(void)> func) {
    if (_dispatchAsync) {
        _dispatchAsync(host, func);
    }
}

