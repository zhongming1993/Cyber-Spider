//
//  Header.h
//  p4
//
//  Created by MingZhong on 3/7/16.
//  Copyright © 2016 MingZhong. All rights reserved.
//

#ifndef INTERACTIONTUPLE_H_
#define INTERACTIONTUPLE_H_

#include <string>

struct InteractionTuple
{
    InteractionTuple()
    {}
    
    InteractionTuple(const std::string& f, const std::string& t, const std::string& c)
    : from(f), to(t), context(c)
    {}
    
    std::string from;
    std::string to;
    std::string context;
};

#endif // INTERACTIONTUPLE_H_
