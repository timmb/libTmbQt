//
//  ScopedTimer.hpp
//  NorwayCloud
//
//  Created by Tim Murray Browne on 2016-05-27.
//
//

#pragma once

#include <QElapsedTimer>
#include <QDebug>
#include <ostream>
#include <iostream>

class ScopedTimer
{
public:
    ScopedTimer(char const* name, int minDurationToPrintMessage=0)
    : mName(name)
    , mMinDurationToPrintMessage(minDurationToPrintMessage)
    {
        mTimer.start();
    }
    
    ~ScopedTimer()
    {
        auto const t = mTimer.elapsed();
        if (t > mMinDurationToPrintMessage)
        {
            std::cout
            //        qDebug().nospace()
            << "ScopedTimer "<<mName<<": "<<t<<"ms"
            << std::endl
            ;
        }
        
    }
    
private:
    QElapsedTimer mTimer;
    char const* const mName;
    int mMinDurationToPrintMessage;
};
