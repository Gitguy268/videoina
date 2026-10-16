#include "keys.h"

#include <QHash>

QString mpvKeyName(int key, int modifiers, const QString &text)
{
    static const QHash<int, QString> special{
        {Qt::Key_Space, "SPACE"}, {Qt::Key_Return, "ENTER"}, {Qt::Key_Enter, "KP_ENTER"},
        {Qt::Key_Escape, "ESC"}, {Qt::Key_Left, "LEFT"}, {Qt::Key_Right, "RIGHT"},
        {Qt::Key_Up, "UP"}, {Qt::Key_Down, "DOWN"}, {Qt::Key_PageUp, "PGUP"},
        {Qt::Key_PageDown, "PGDWN"}, {Qt::Key_Home, "HOME"}, {Qt::Key_End, "END"},
        {Qt::Key_Tab, "TAB"}, {Qt::Key_Backspace, "BS"}, {Qt::Key_Delete, "DEL"},
        {Qt::Key_Insert, "INS"}, {Qt::Key_MediaPlay, "PLAY"}, {Qt::Key_MediaPause, "PAUSE"},
        {Qt::Key_MediaTogglePlayPause, "PLAYPAUSE"}, {Qt::Key_MediaStop, "STOP"},
        {Qt::Key_MediaNext, "NEXT"}, {Qt::Key_MediaPrevious, "PREV"},
        {Qt::Key_VolumeUp, "VOLUME_UP"}, {Qt::Key_VolumeDown, "VOLUME_DOWN"},
        {Qt::Key_VolumeMute, "MUTE"}};

    QString name = special.value(key);
    bool ctrl = modifiers & Qt::ControlModifier, alt = modifiers & Qt::AltModifier;
    bool shift = modifiers & Qt::ShiftModifier;
    if (name.isEmpty() && key >= Qt::Key_F1 && key <= Qt::Key_F12)
        name = QString("F%1").arg(key - Qt::Key_F1 + 1);
    // With Ctrl/Alt, `text` is a control or dead char; Qt's key code is the ASCII char.
    if (name.isEmpty() && (ctrl || alt) && key > 0x20 && key < 0x7f)
        name = QChar(key >= Qt::Key_A && key <= Qt::Key_Z && !shift ? key + 32 : key);
    else if (name.isEmpty() && !text.isEmpty() && text.at(0).isPrint())
        name = text;
    else if (!name.isEmpty() && shift)
        name.prepend("Shift+"); // mpv folds Shift into printable chars itself
    if (name.isEmpty())
        return {};
    if (alt)
        name.prepend("Alt+");
    if (ctrl)
        name.prepend("Ctrl+");
    return name;
}
