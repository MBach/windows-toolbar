#ifndef MUSICSEARCHENGINE_H
#define MUSICSEARCHENGINE_H

#include <QDir>
#include <QFileInfo>
#include <QTimer>

#include "miamcore_global.h"
#include <model/sqldatabase.h>

/**
 * \brief		The MusicSearchEngine class
 * \author      Matthieu Bachelier
 * \copyright   GNU General Public License v3
 */
class MIAMCORE_LIBRARY MusicSearchEngine : public QObject
{
	Q_OBJECT
private:
	SqlDatabase _db;
	QTimer *_timer;
	QStringList _delta;

public:
	static bool isScanning;

	MusicSearchEngine(QObject *parent = nullptr);

	void setDelta(const QStringList &delta);

	void setWatchForChanges(bool b);

public slots:
	void doSearch();

private slots:
	void watchForChanges();

signals:
	void aboutToSearch();

	void progressChanged(int);

	void searchHasEnded();
};

#endif // MUSICSEARCHENGINE_H
