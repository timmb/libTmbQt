#include "LambdaRunnable.h"

LambdaRunnable::LambdaRunnable(std::function<void()> function, bool shallAutoDelete)
: QRunnable()
, mFunction(std::move(function))
{
    setAutoDelete(shallAutoDelete);
}

void LambdaRunnable::run()
{
    mFunction();
}
