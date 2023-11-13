#ifndef SIMPLE_BASE_DATA_MANAGER_BASE_H_
#define SIMPLE_BASE_DATA_MANAGER_BASE_H_

#include "Common.h"

namespace base {
/* align up to z^n */
template < typename T_ > inline T_ AlignUp2N(const T_ v, uint32_t r) {
    return (((v + ((static_cast< T_ >(0x1) << r) - static_cast< T_ >(0x1))) >> r) << r);
}
/* align down to z^n */
template < typename T_ > inline T_ AlignDownN(const T_ v, uint32_t r) {
    return ((v >> r) << r);
}

#define ALIGN_UP_16(x) AlignUp2N(x, 4)
#define ALIGN_DOWN_16(x) AlignDownN(x, 4)

#define ALIGN_UP_2(x) AlignUp2N(x, 1)
#define ALIGN_DOWN_2(x) AlignDownN(x, 1)

#define IS_ALIGN_16(x) ((( x )&0xF) == 0x0)
#define IS_ALIGN_2(x) ((( x )&0x1) == 0x0)

} // base
#endif // SIMPLE_BASE_DATA_MANAGER_BASE_H_