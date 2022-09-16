//
// Created by  jormin on 2021/6/2.
//

#include "OlaImageFrame.hpp"

namespace Opipe {

    void OlaImageFrame::createData(int width, int height, int widthStep, OlaImageFormat format) {
        std::unique_lock<std::mutex> lock(_dataLock);
        _des.width = width;
        _des.height = height;
        _des.format = format;
        _des.widthStep = widthStep;
        //目前widthStep包含了数据格式信息，比如RGBA，widthStep至少是width*4
        //TODO 后续还是需要改成格式格式决定内存大小
        _data = new char[_des.widthStep * _des.height];
    }

    void OlaImageFrame::copyData(char *data, int width, int height, int widthStep, OlaImageFormat format) {
        std::unique_lock<std::mutex> lock(_dataLock);
        _des.width = width;
        _des.height = height;
        _des.format = format;
        _des.widthStep = widthStep;
        //目前widthStep包含了数据格式信息，比如RGBA，widthStep至少是width*4
        //TODO 后续还是需要改成格式格式决定内存大小
        _data = new char[_des.widthStep * _des.height];
        memcpy(_data, data, _des.widthStep * _des.height);
    }

    void OlaImageFrame::releaseDataControl(char **data, OlaImageFrameDes &des) {
        std::unique_lock<std::mutex> lock(_dataLock);

        *data = _data;

        des.fill(_des);
        _data = nullptr;
        _des.reset();
    }

    void OlaImageFrame::releaseData() {
        std::unique_lock<std::mutex> lock(_dataLock);
        if (_data) {
            delete[] _data;
            _data = nullptr;
            _des.reset();
        }
    }

    OlaImageFrame::~OlaImageFrame() {
        releaseData();
    }


}
