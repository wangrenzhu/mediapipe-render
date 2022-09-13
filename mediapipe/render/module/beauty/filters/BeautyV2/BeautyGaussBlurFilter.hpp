#ifndef BeautyGaussBlurFilter_hpp
#define BeautyGaussBlurFilter_hpp

#include "mediapipe/render/core/FilterGroup.hpp"
#include "mediapipe/render/core/Context.hpp"
#include "BeautyGaussPassFilter.hpp"

NS_GI_BEGIN

class BeautyGaussBlurFilter : public FilterGroup {
public:
    virtual ~BeautyGaussBlurFilter();
    
    static BeautyGaussBlurFilter* create(Context *context);
    bool init(Context *context);
    
protected:
    BeautyGaussBlurFilter(Context *context);
    
private:
    BeautyGaussPassFilter *_verticalPassFilter = nullptr;
    BeautyGaussPassFilter *_horizontalPassFilter = nullptr;
};


NS_GI_END

#endif /* BeautyGaussBlurFilter_hpp */
