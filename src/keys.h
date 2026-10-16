#pragma once

#include <QString>

// mpv input.conf key name for a Qt key event, e.g. "Ctrl+LEFT", "Alt+v", "(" ; empty if unmapped.
QString mpvKeyName(int key, int modifiers, const QString &text);
