#ifndef MAIN_H
#define MAIN_H

#include "compatibility.h"

#ifdef OS_MAC
#import <Cocoa/Cocoa.h>

@interface SDLMain : NSObject
@end
#endif

#endif
