//
//  BackingPipeline.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "Pipeline.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

class BackingPipeline : public Pipeline {
public:
    BackingPipeline(Window *window);
    virtual ~BackingPipeline();
};

}}}
