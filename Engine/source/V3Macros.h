#pragma once
#include <iostream>

#define info(x) std::cout << "V3::[info] " << x << std::endl;
#define infof(x, ...) std::cout << "V3::[info] "; printf((x), __VA_ARGS__); std::cout << std::endl;
#define warn(x) std::cout << "V3::[warn] " << x << std::endl;
#define warnf(x, ...) std::cout << "V3::[warn] "; printf((x), __VA_ARGS__); std::cout << std::endl;
#define critical(x) std::cout << "V3::[critical] " << x << std::endl;
#define criticalf(x, ...) std::cout << "V3::[critical] "; printf((x), __VA_ARGS__); std::cout << std::endl;
#define debug(x) std::cout << "V3::[debug] " << x << std::endl;
#define debugf(x, ...) std::cout << "V3::[debug] "; printf((x), __VA_ARGS__); std::cout << std::endl;
#define vkcritical(x) std::cout << "VK->[critical] " << x << std::endl;
#define vkcriticalf(x, ...) std::cout << "VK->[critical] "; printf((x), __VA_ARGS__); std::cout << std::endl;
#define vkdebug(x) std::cout << "VK->[debug] " << x << std::endl;
#define vkdebugf(x, ...) std::cout << "VK->[debug] "; printf((x), __VA_ARGS__); std::cout << std::endl;
#define vkwarn(x) std::cout << "VK->[warn] " << x << std::endl;
#define vkwarnf(x, ...) std::cout << "VK->[warn] "; printf((x), __VA_ARGS__); std::cout << std::endl;