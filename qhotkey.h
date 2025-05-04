#ifndef QHOTKEY_H
#define QHOTKEY_H

#include <QObject>
#include <QKeySequence>
#include <QAbstractNativeEventFilter>
#include <QHash>
#include <QMutex>
#include <QVariant>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class QHotkey;

class QHotkeyPrivate : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    QHotkeyPrivate();
    ~QHotkeyPrivate();

    static QHotkeyPrivate *instance();

    bool registerHotkey(QHotkey *hotkey);
    bool unregisterHotkey(QHotkey *hotkey);


    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result);


private:
    static QHotkeyPrivate *_instance;

    QHash<quintptr, QHotkey*> hotkeys;
    QMutex mutex;
};

class QHotkey : public QObject
{
    Q_OBJECT

public:
    explicit QHotkey(QObject *parent = nullptr);
    explicit QHotkey(const QKeySequence &keySequence, bool autoRegister = true, QObject *parent = nullptr);
    ~QHotkey();

    bool setShortcut(const QKeySequence &keySequence, bool autoRegister = true);
    QKeySequence shortcut() const;

    bool isRegistered() const;
    bool setRegistered(bool registered);

signals:
    void activated();

private:
    friend class QHotkeyPrivate;
    QHotkeyPrivate *d;
};

#endif // QHOTKEY_H