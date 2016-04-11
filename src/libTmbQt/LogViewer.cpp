#include "LogViewer.h"
#include "QDateTime"
#include "QFileInfo"
#include <QDebug>
#include "QBoxLayout"
#include "QComboBox"
#include "QLabel"
#include "Utilities.h"



LogViewer::LogViewer(Logger* logger, QWidget* parent /*= nullptr*/)
	: QWidget(parent)
	, mLogger(logger)
	, mConsole(new QPlainTextEdit(this))
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

	QComboBox* comboLevel = new QComboBox(this);
	comboLevel->insertItem(0, "Debug");
	comboLevel->insertItem(1, "Info");
	comboLevel->insertItem(2, "Warning");
	comboLevel->insertItem(3, "Critical");
	comboLevel->insertItem(4, "None");
	comboLevel->setCurrentIndex(msgTypeToInt(mLevel));
	connect(comboLevel, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [this](int index) {
		//gLocalSettings->setValue(LOCAL_LOG_VIEWER_LEVEL, intToMsgType(index));
		mLevel = intToMsgType(index);
	});
	toolbar->addWidget(new QLabel("Level", this));
	toolbar->addWidget(comboLevel);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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
	refresh();
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
	QString s(QString("<span style=\"color:#%5\">--- <i>%1\t %2 \t%3</i><br>%4</span>").arg(toString(message.type), datetime, message.location, message.message, colors.value(message.type)));
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

