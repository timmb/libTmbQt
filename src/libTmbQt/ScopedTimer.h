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
    ScopedTimer(char const* name)
    : mName(name)
    {
        mTimer.start();
    }
    
    ~ScopedTimer()
    {
        std::cout
//        qDebug().nospace()
        << "ScopedTimer "<<mName<<": "<<mTimer.elapsed()<<"ms"
        << std::endl
        ;
        
        
    }
    
private:
    QElapsedTimer mTimer;
    char const* const mName;
};