#include "BeautyGaussBlurFilter.hpp"
#include "mediapipe/framework/port/logging.h"

namespace Opipe 
{
    BeautyGaussBlurFilter::BeautyGaussBlurFilter(Context *context) : FilterGroup(context)
    {

    }

    BeautyGaussBlurFilter::~BeautyGaussBlurFilter()
    {
        if (_horizontalPassFilter) {
            _horizontalPassFilter->release();
            _horizontalPassFilter = 0;
        }
    
        if (_verticalPassFilter) {
            _verticalPassFilter->release();
            _verticalPassFilter = 0;
        }
    }

    BeautyGaussBlurFilter *BeautyGaussBlurFilter::create(Context *context)
    {
        BeautyGaussBlurFilter *ret = new (std::nothrow)BeautyGaussBlurFilter(context);
        if (ret && !ret->init(context)) {
            delete ret;
            ret = 0;
        }
        return ret;
    }

    bool BeautyGaussBlurFilter::init(Context *context) {
        if (!FilterGroup::init(context)) {
            return false;
        }

        _horizontalPassFilter = BeautyGaussPassFilter::create(context, BeautyGaussPassFilter::HORIZONTAL);
        _verticalPassFilter = BeautyGaussPassFilter::create(context, BeautyGaussPassFilter::VERTICAL);

        _verticalPassFilter->addTarget(_horizontalPassFilter);

        addFilter(_verticalPassFilter);
        LOG(INFO) << "BeautyGaussBlurFilter:" << "_horizontalPassFilter:" << _horizontalPassFilter;
        LOG(INFO) << "BeautyGaussBlurFilter:" << "_verticalPassFilter:" << _verticalPassFilter;

        return true;
    }
}
