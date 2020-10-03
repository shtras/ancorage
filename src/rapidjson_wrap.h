// Need to disable these stupid defines outside of the include guard so can't use pragma once
// Microsoft...
#ifdef GetObject
#undef GetObject
#endif

#ifndef _RAPIDJSON_WRAP_
#define _RAPIDJSON_WRAP_
#include "rapidjson/rapidjson.h"
#include "rapidjson/document.h"
#endif

namespace rapidjson
{
typedef GenericValue<UTF16<>> WValue;
}
