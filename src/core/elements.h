//
// Copyright (c) 2023 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "Circuit.h"
#include "Pin.h"
#include "Bus.h"

Pin dLatch(Pin data, Pin enable);

Bus multiplexer(Bus input);

Pin tLatch(Pin toggle);
