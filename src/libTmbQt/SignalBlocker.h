#include <QObject>

class SignalBlocker
{
public:
	/// object's signals will be blocked for the lifetime of this instances
	SignalBlocker(QObject* object);
	virtual ~SignalBlocker();

private:
	QObject* mObject;
};