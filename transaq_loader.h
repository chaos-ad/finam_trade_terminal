#pragma once

#include <string>
#include <stdexcept>

namespace transaq {
namespace loader {

void  unload_library(void * library);
void* load_library(std::string const& path);
void* load_function(void * library, std::string const& name);

template <class Type>
Type load_fun(void * library, std::string const& name)
{
    return reinterpret_cast<Type>( load_function(library, name) );
}

}
}
