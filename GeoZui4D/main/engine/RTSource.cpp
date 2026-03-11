// Copyright 2000-2026 Center for Coastal and Ocean Mapping, University of New Hampshire
// SPDX-License-Identifier: Apache-2.0


#include "engine/RTSource.h"

RTSource::RTSource()
{
	previousTime = currentTime = 0.0;
	glossOverHiccups = 1;
}

RTSource::~RTSource()
{
}

