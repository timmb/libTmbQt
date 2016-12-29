#include <QRunnable>
#include <functional>

class LambdaRunnable : public QRunnable
{
public:
    
    LambdaRunnable(std::function<void()> function, bool autoDelete=true);
    
    virtual void run() override;
    
private:
    std::function<void()> mFunction;
};
