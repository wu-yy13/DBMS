/*
 * baseobj.h
 *
 *  Created on: 2015年10月28日
 *      Author: 吴永宇
 */

#ifndef _BASEOBJ_H_
#define _BASEOBJ_H_
#include <string.h>
#include<stdlib.h>
#include "type.h"
using namespace std;
/*
*@brief struct
*@parm void* loc
*@parm int size
*@parm Type type
*@parm bool is_null
*
*
*/
struct Object {
    void* loc;
    int size;
    TYPE type;
    bool is_null;
    Object(){};
    Object(void* location, int s, TYPE t, bool null) {
        loc = location;
        size = s;
        type = t;
        is_null = null;
    }
};

inline bool operator<(const Object& lhs, const Object& rhs) {
    int size = lhs.size > rhs.size ? lhs.size : rhs.size;
   return (memcmp(lhs.loc, rhs.loc, size) < 0);


}

inline bool operator!=(const Object& lhs, const Object& rhs) {
    int size = lhs.size > rhs.size ? lhs.size : rhs.size;
    return memcmp(lhs.loc, rhs.loc, size) != 0;

}

#endif
