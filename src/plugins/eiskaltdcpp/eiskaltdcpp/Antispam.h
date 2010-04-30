/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef DCANTISPAM_H
#define DCANTISPAM_H

#include <QObject>
#include <QList>
#include <QMap>

#include "dcpp/stdinc.h"
#include "dcpp/DCPlusPlus.h"
#include "dcpp/User.h"
#include "dcpp/Singleton.h"

enum AntiSpamObjectState {
    eIN_BLACK = 0,
    eIN_GRAY,
    eIN_WHITE
};

class AntiSpam :
        public QObject,
        public dcpp::Singleton<AntiSpam>
{
    Q_OBJECT

    friend class dcpp::Singleton<AntiSpam>;
public:
    bool isInBlack(QString);
    bool isInWhite(QString);
    bool isInGray(QString);
    bool isInAny(QString);

    void move(QString, AntiSpamObjectState);

    void addToBlack(const QList<QString> &list);
    void addToWhite(const QList<QString> &list);
    void addToGray(const QList<QString> &list);
    void remFromBlack(const QList<QString> &list);
    void remFromWhite(const QList<QString> &list);
    void remFromGray(const QList<QString> &list);
    void clearBlack();
    void clearGray();
    void clearWhite();
    void clearAll();

    QList<QString> getBlack();
    QList<QString> getGray();
    QList<QString> getWhite();

    void loadSettings();
    void saveSettings();
    void loadLists();
    void saveLists();

    QString getPhrase() const;
    void setPhrase(QString &phrase);
    QList<QString> getKeys();
    void setKeys(QList<QString> &keys);

    void setAttempts(int);
    int  getAttempts() const;

    void checkUser(const QString &, const QString &, const QString &);

    friend AntiSpam& operator<<(AntiSpam&, AntiSpamObjectState);
    friend AntiSpam& operator<<(AntiSpam&, const QList<QString>&);
    friend AntiSpam& operator<<(AntiSpam&, const QString&);

private:

    AntiSpam();
    virtual ~AntiSpam();

    inline void addToList(QList<QString>&, const QList<QString>&);
    inline void remFromList(QList<QString>&, const QList<QString>&);

    void loadBlack();
    void loadWhite();
    void loadGray();
    void saveBlack();
    void saveWhite();
    void saveGray();


    void readFile(QString, QList<QString>&);
    void saveFile(QString, QList<QString>&);

    QList<QString> white_list, black_list, gray_list;

    QString phrase;
    QList<QString> keys;
    QMap< QString, int > sandbox;

    int try_count;

    AntiSpamObjectState state;//used only by operator<<

public slots:

    /** */
    void slotObjectChangeState(QString obj, AntiSpamObjectState from, AntiSpamObjectState to);

};

#endif // DCANTISPAM_H
