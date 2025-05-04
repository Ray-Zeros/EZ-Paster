#include "qhotkey.h"
#include <QCoreApplication>
#include <QAbstractEventDispatcher>
#include <QHash>
#include <QMutex>
#include <QVariant>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

QHotkeyPrivate *QHotkeyPrivate::_instance = nullptr;

// QHotkey 实现
QHotkey::QHotkey(QObject *parent) : QObject(parent), d(QHotkeyPrivate::instance())
{
}

QHotkey::QHotkey(const QKeySequence &keySequence, bool autoRegister, QObject *parent) : 
    QObject(parent), d(QHotkeyPrivate::instance())
{
    setShortcut(keySequence, autoRegister);
}

QHotkey::~QHotkey()
{
    if (isRegistered())
        setRegistered(false);
}

bool QHotkey::setShortcut(const QKeySequence &keySequence, bool autoRegister)
{
    bool wasRegistered = isRegistered();
    if (wasRegistered)
        setRegistered(false);

    // 简单设置一下，这里可以更复杂
    QVariant var = QVariant::fromValue(keySequence);
    setProperty("shortcut", var);

    if (wasRegistered && autoRegister)
        return setRegistered(true);
    return true;
}

QKeySequence QHotkey::shortcut() const
{
    QVariant var = property("shortcut");
    return var.value<QKeySequence>();
}

bool QHotkey::isRegistered() const
{
    return property("registered").toBool();
}

bool QHotkey::setRegistered(bool registered)
{
    if (registered && !isRegistered()) {
        if (d->registerHotkey(this)) {
            setProperty("registered", QVariant(true));
            return true;
        }
        return false;
    } else if (!registered && isRegistered()) {
        if (d->unregisterHotkey(this)) {
            setProperty("registered", QVariant(false));
            return true;
        }
        return false;
    } else {
        return true; // 无需更改
    }
}

// QHotkeyPrivate 实现
QHotkeyPrivate::QHotkeyPrivate() : QObject(nullptr)
{
    QAbstractEventDispatcher::instance()->installNativeEventFilter(this);
}

QHotkeyPrivate::~QHotkeyPrivate()
{
    QAbstractEventDispatcher::instance()->removeNativeEventFilter(this);
}

QHotkeyPrivate *QHotkeyPrivate::instance()
{
    if (!_instance)
        _instance = new QHotkeyPrivate();
    return _instance;
}

bool QHotkeyPrivate::registerHotkey(QHotkey *hotkey)
{
#ifdef Q_OS_WIN
    QKeySequence keySeq = hotkey->shortcut();
    if (keySeq.isEmpty())
        return false;

    // 仅处理第一个键
    Qt::Key key = Qt::Key(keySeq[0] & ~Qt::KeyboardModifierMask);
    Qt::KeyboardModifiers mods = Qt::KeyboardModifiers(keySeq[0] & Qt::KeyboardModifierMask);
    
    // 转换为Windows修饰键
    UINT modifiers = 0;
    if (mods & Qt::AltModifier)
        modifiers |= MOD_ALT;
    if (mods & Qt::ControlModifier)
        modifiers |= MOD_CONTROL;
    if (mods & Qt::ShiftModifier)
        modifiers |= MOD_SHIFT;
    if (mods & Qt::MetaModifier)
        modifiers |= MOD_WIN;
    
    // 获取虚拟键
    UINT vk = 0;
    switch (key) {
        case Qt::Key_A: vk = 'A'; break;
        case Qt::Key_B: vk = 'B'; break;
        case Qt::Key_C: vk = 'C'; break;
        // 添加更多键... 此处简化
        default:
            // 尝试简单转换 (对于字母/数字键工作)
            vk = key;
            break;
    }
    
    if (vk == 0)
        return false;
    
    // 注册热键
    quintptr id = reinterpret_cast<quintptr>(hotkey);
    if (!RegisterHotKey(nullptr, id, modifiers, vk))
        return false;
    
    mutex.lock();
    hotkeys.insert(id, hotkey);
    mutex.unlock();
    
    return true;
#else
    // 其他平台支持将在这里添加
    Q_UNUSED(hotkey)
    return false;
#endif
}

bool QHotkeyPrivate::unregisterHotkey(QHotkey *hotkey)
{
#ifdef Q_OS_WIN
    quintptr id = reinterpret_cast<quintptr>(hotkey);
    
    mutex.lock();
    hotkeys.remove(id);
    mutex.unlock();
    
    return UnregisterHotKey(nullptr, id);
#else
    // 其他平台支持将在这里添加
    Q_UNUSED(hotkey)
    return false;
#endif
}

#ifdef Q_OS_WIN
bool QHotkeyPrivate::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType)
    Q_UNUSED(result)
    
    MSG* msg = static_cast<MSG*>(message);
    if (msg->message == WM_HOTKEY) {
        quintptr id = static_cast<quintptr>(msg->wParam);
        
        mutex.lock();
        QHotkey *hotkey = hotkeys.value(id);
        mutex.unlock();
        
        if (hotkey)
            emit hotkey->activated();
        
        return true;
    }
    
    return false;
}
#endif