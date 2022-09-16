//
//  OPreStreamingCalculator.cpp
//  Quaramera
//
//  Created by wangrenzhu2021 on 2021/11/25.
//  Copyright © 2021 alibaba. All rights reserved.
//

#include "OlaPreStreaming.hpp"
#include "OlaPreStreamingIMP.hpp"
#include <vector>

namespace Opipe {
    
    OlaPreStreaming* OlaPreStreaming::create(OpipeDispatch *dispatch) {
        return new OlaPreStreamingIMP(dispatch);
    }
}
