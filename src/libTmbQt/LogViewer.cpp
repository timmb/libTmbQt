#include "LogViewer.h"
#include "QDateTime"
#include "QFileInfo"
#include <QDebug>
#include "QBoxLayout"
#include "QComboBox"
#include "QLabel"
#include "Utilities.h"
#include "SignalBlocker.h"


QtMsgType LogViewer::comboBoxIndexToQtMsgType(int index)
{
	switch (index)
	{
	case 0:
	default:
		return QtDebugMsg;
	case 1:
		return QtInfoMsg;
	case 2:
		return QtWarningMsg;
	case 3:
		return QtCriticalMsg;
	case 4:
		return QtFatalMsg;
	}
}

int LogViewer::qtMsgTypeToComboBoxIndex(QtMsgType level)
{
	switch (level)
	{
	case QtDebugMsg:
	default:
		return 0;
	case QtInfoMsg:
		return 1;
	case QtWarningMsg:
		return 2;
	case QtCriticalMsg:
		return 3;
	case QtFatalMsg:
		return 4;
	}
}

int LogViewer::numQtMsgTypes()
{
    return 5;
}


LogViewer::LogViewer(Logger* logger, QWidget* parent /*= nullptr*/)
	: QWidget(parent)
	, mConsole(new QPlainTextEdit(this))
    , mLogger(logger)
	, mLevelComboBox(nullptr)
    , mLevel(QtWarningMsg)
    , mMutex(QMutex::Recursive)
{
	Q_ASSERT(mLogger != nullptr);
	mLogger->addListener(this);
	connect(mLogger, &QObject::destroyed, [this] { mLogger = nullptr; });

	//int level = QSettings().value() gLocalSettings->value(LOCAL_LOG_VIEWER_LEVEL).toInt();

	//mLevel = intToMsgType(level);


	mConsole->setMaximumBlockCount(10000);
	mConsole->setReadOnly(true);
	mConsole->setUndoRedoEnabled(false);

	QVBoxLayout* layout = new QVBoxLayout;
	setLayout(layout);
	QHBoxLayout* toolbar = new QHBoxLayout;
	layout->addLayout(toolbar);
	layout->addWidget(mConsole);

	mLevelComboBox = new QComboBox(this);
	mLevelComboBox->insertItem(0, "Debug");
	mLevelComboBox->insertItem(1, "Info");
	mLevelComboBox->insertItem(2, "Warning");
	mLevelComboBox->insertItem(3, "Critical");
	mLevelComboBox->insertItem(4, "None");
	mLevelComboBox->setCurrentIndex(msgTypeToInt(mLevel));
	connect(mLevelComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int index) {
		//gLocalSettings->setValue(LOCAL_LOG_VIEWER_LEVEL, intToMsgType(index));
		//setLevel(intToMsgType(index));

		setLevel(comboBoxIndexToQtMsgType(index));
	});
	toolbar->addWidget(new QLabel("Level", this));
	toolbar->addWidget(mLevelComboBox);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	setLevel(intToMsgType(QSettings().value("logViewerLevel").toInt()));

	refresh();

	//VERIFY(connect(gLocalSettings, SIGNAL(valueChanged(QString const&, QVariant const&)), this, SLOT(localSettingChanged(QString const&, QVariant const&))));

	QTimer* updateTimer = new QTimer(this);
	updateTimer->setInterval(100);
	VERIFY(connect(updateTimer, SIGNAL(timeout()), this, SLOT(update())));
	updateTimer->start();
}

LogViewer::~LogViewer()
{
	if (mLogger != nullptr)
	{
		mLogger->removeListener(this);
	}
}

void LogViewer::setLevel(QtMsgType level)
{
	mLevel = level;
	{
		SignalBlocker _(mLevelComboBox);
		mLevelComboBox->setCurrentIndex(qtMsgTypeToComboBoxIndex(level));
	}
	refresh();
	QSettings().setValue("logViewerLevel", level);
}

void LogViewer::handle(LogMessage const& message)
{
	QMutexLocker lock(&mMutex);
	p_handle(message);
}

//void LogViewer::localSettingChanged(QString const& key, QVariant const& value)
//{
//	if (key == LOCAL_LOG_VIEWER_LEVEL)
//	{
//		Q_ASSERT(value.type() == QVariant::Int);
//		int index = value.toInt();
//		QtMsgType level = index == 0 ? QtDebugMsg
//			: index == 1 ? QtInfoMsg
//			: index == 2 ? QtWarningMsg
//			: index == 3 ? QtCriticalMsg
//			: QtFatalMsg;
//		setLevel(level);
//	}
//}

void LogViewer::p_handle(LogMessage const& message)
{
	static QMap<QtMsgType, QString> const colors = {
		{ QtDebugMsg, "888" },
		{ QtInfoMsg, "222" },
		{ QtWarningMsg, "f44" },
		{ QtCriticalMsg, "f00" },
		{ QtFatalMsg, "f00" },
	};

	QString datetime = message.time.toString("yyyy-MM-dd HH:mm:ss.zzz");
	QString s(QString("<span style=\"color:#%5\"><small>--- <i>%1\t %2 \t%3</i></small><br>%4</span>").arg(toString(message.type), datetime, message.location, message.message, colors.value(message.type)));
	{
		QMutexLocker lock(&mMutex);
		mBuffer << s;
	}
}

void LogViewer::update()
{
	// avoid appending html with the buffer locked as can cause deadlock
	QStringList buffer;
	{
		QMutexLocker lock(&mMutex);
		qSwap(mBuffer, buffer);
	}
	for (QString& s : buffer)
	{
		mConsole->appendHtml(std::move(s));
	}
}

void LogViewer::refresh()
{
	QList<std::shared_ptr<LogMessage const>> buffer = mLogger->getHistory();

	{
		QMutexLocker lock(&mMutex);
		mConsole->clear();
		for (auto const& m : buffer)
		{
			if (shouldHandle(m->type))
			{
				p_handle(*m);
			}
		}
	}
}

