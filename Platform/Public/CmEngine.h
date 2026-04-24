#pragma once

#include <iostream>
#include <cassert>
#include <memory>
#include <algorithm>
#include <functional>

#include <string>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <stack>
#include <queue>
#include <deque>
#include <set>
#include <unordered_set>
#include <unordered_map>

#define ARRAY_SIZE(r) (sizeof(r) / sizeof(r[0]))
#define __FILENAME__                    (strrchr(__FILE__, '/') + 1)
#define ENUM_TO_STR(r)                  case r: return #r

#define CHIMI_ENGINE_GRAPHIC_API_VULKAN
