#ifndef SETTINGS_H
#define SETTINGS_H

#include <QFileInfo>
#include <QPushButton>
#include <QSettings>

#include "miamcore_global.h"

class MIAMCORE_LIBRARY Settings : public QSettings
{
	Q_OBJECT

private:
	/** The unique instance of this class. */
	static Settings *settings;

	/** Private constructor. */
	Settings(const QString &organization = "MmeMiamMiam",
			 const QString &application = "MmeMiamMiamMusicPlayer");

	/** Store the size of each font used in the app. */
	QMap<QString, QVariant> fontPointSizeMap;

	/** Store the family of each font used in the app. */
	QMap<QString, QVariant> fontFamilyMap;

	QList<QVariant> locations;

	QMap<QString, QByteArray> stylesheets;

	QMap<QString, QVariant> columnStates;

	Q_ENUMS(FontFamily)
	Q_ENUMS(PlaylistDefaultAction)

public:
	enum FontFamily{PLAYLIST, LIBRARY, MENUS};

	enum PlaylistDefaultAction{AskUserForAction	= 0,
							   SaveOnClose		= 1,
							   DiscardOnClose	= 2};

	/** Singleton Pattern to easily use Settings everywhere in the app. */
	static Settings* getInstance();

	qreal bigCoverOpacity() const;

	/** Returns the actual size of media buttons. */
	int buttonsSize() const;

	/** Returns true if buttons are displayed without any border. */
	bool buttonsFlat() const;

	/** Returns true if the background color in playlist is using alternatative colors. */
	bool colorsAlternateBG() const;

	bool copyTracksFromPlaylist() const;

	/** Returns the size of a cover. */
	int coverSize() const;

	QColor customColors(QPalette::ColorRole cr) const;

	/** Custom icons in CustomizeTheme */
	const QString customIcon(QPushButton *, bool toggled = false) const;

	const QString dragAndDropBehaviour() const;

	/** Returns the font of the application. */
	QFont font(const FontFamily fontFamily);

	/** Sets the font of the application. */
	int fontSize(const FontFamily fontFamily);

	/** Custom icons in CustomizeTheme */
	bool hasCustomIcon(QPushButton *) const;

	/** Returns true if big and faded covers are displayed in the library when an album is expanded. */
	bool isBigCoverEnabled() const;

	/** Returns true if covers are displayed in the library. */
	bool isCoversEnabled() const;

	bool isCustomColors() const;

	/** Returns true if the button in parameter is visible or not. */
	bool isMediaButtonVisible(const QString & buttonName) const;

	/** Returns true if stars are visible and active. */
	bool isStarDelegates() const;

	/** Returns the language of the application. */
	QString language();

	/** Returns all music locations. */
	QStringList musicLocations() const;

	/// PlayBack options
	qint64 playbackSeekTime() const;

	PlaylistDefaultAction playbackDefaultActionForClose() const;

	bool playbackKeepPlaylists() const;

	bool playbackRestorePlaylistsAtStartup() const;

	QByteArray restoreColumnStateForPlaylist(int playlistIndex) const;

	void saveColumnStateForPlaylist(int playlistIndex, const QByteArray &state);

	void setCustomColorRole(QPalette::ColorRole cr, const QColor &color);

	/** Custom icons in CustomizeTheme */
	void setCustomIcon(QPushButton *, const QString &buttonName);

	/** Sets the language of the application. */
	void setLanguage(const QString &lang);

	void setMusicLocations(const QStringList &locations);

	void setShortcut(const QString &objectName, int keySequence);

	int shortcut(const QString &objectName) const;

	QMap<QString, QVariant> shortcuts() const;

	/** Returns the actual theme name. */
	QString theme() const;

	/** Returns volume from the slider. */
	int volume() const;

public slots:

	void setBigCoverOpacity(int v);

	void setBigCovers(bool b);

	/** Sets a new button size. */
	void setButtonsSize(const int &s);
	void setButtonsFlat(bool b);

	/// Colors
	void setColorsAlternateBG(bool b);

	void setCopyTracksFromPlaylist(bool b);

	void setCovers(bool b);
	void setCoverSize(int s);

	void setCustomColors(bool b);

	/** Sets if stars are visible and active. */
	void setDelegates(const bool &value);

	void setDragAndDropBehaviour();

	/** Sets the font of a part of the application. */
	void setFont(const FontFamily &fontFamily, const QFont &font);

	/** Sets the font size of a part of the application. */
	void setFontPointSize(const FontFamily &fontFamily, int i);

	/** Sets if the button in parameter is visible or not. */
	void setMediaButtonVisible(const QString & buttonName, const bool &value);

	/// PlayBack options
	void setPlaybackSeekTime(int t);
	void setPlaybackDefaultActionForClose(PlaylistDefaultAction action);
	void setPlaybackKeepPlaylists(bool b);
	void setPlaybackRestorePlaylistsAtStartup(bool b);

	/** Sets a new theme. */
	void setThemeName(const QString &theme);

	/** Sets volume from the slider. */
	void setVolume(int v);

signals:
	void themeHasChanged();

	void fontHasChanged(const FontFamily &fontFamily, const QFont &font);
};

Q_DECLARE_METATYPE(QPalette::ColorRole)

#endif // SETTINGS_H
