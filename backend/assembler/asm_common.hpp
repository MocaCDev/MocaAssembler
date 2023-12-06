#ifndef moca_assembler_assembler_common_h
#define moca_assembler_assembler_common_h
#include "../../common.hpp"

namespace AssemblerCommon
{
    template<typename T = uslng>
        requires std::is_same<T, uslng>::value
            || std::is_same<T, usint16>::value
    inline usint16 LE_16(T data)
    {
        usint16 ret = 0;

        ret |= (data >> 8) & 0xFF;
        ret |= (data << 8) & 0xFF00;

        return ret;
    }

    inline static usint8 LE_get_rightmost_byte(usint16 data)
    {
        return (usint8)(data >> 8) & 0xFF;
    }

    inline static usint8 LE_get_leftmost_byte(usint16 data)
    {
        return (usint8)(data & 0xFF);
    }
}

#endif