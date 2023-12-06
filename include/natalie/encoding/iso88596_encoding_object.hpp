#pragma once

#include <assert.h>
#include <initializer_list>

#include "natalie/encoding/single_byte_encoding_object.hpp"
#include "natalie/string_object.hpp"

namespace Natalie {

using namespace TM;

class Iso88596EncodingObject : public SingleByteEncodingObject {
public:
    Iso88596EncodingObject();
};

}
