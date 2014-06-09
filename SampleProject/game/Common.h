//
//  Common.h
//  TheForce
//
//  Created by Ming Zhao on 2/11/13.
//  Copyright (c) 2013 Tencent. All rights reserved.
//

#ifndef TheForce_Common_h
#define TheForce_Common_h

#pragma mark - Math

#define FLT_EQUAL(x, y) (fabsf((x) - (y)) < FLT_EPSILON)
#define FLT_LE(x, y)    ((x) < (y) || fabsf((x) - (y)) < FLT_EPSILON)
#define FLT_GE(x, y)    ((x) > (y) || fabsf((x) - (y)) < FLT_EPSILON)

#pragma mark - 

#define BREAK_IF(__condi__) \
if (__condi__)\
{\
    break;\
}

#define BREAK_IF_FAILED(__condi__) \
if (false == (__condi__))\
{\
__CCLOGWITHFUNCTION("File: %s, Line: %d", __FILE__, __LINE__);\
break;\
}


#endif
