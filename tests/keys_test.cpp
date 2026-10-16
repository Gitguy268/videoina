#include "keys.h"

#include <QtTest>

// Checks key names against the forms IINA's input.conf uses (mpv folds Shift into printable chars).
class KeysTest : public QObject
{
    Q_OBJECT
private slots:
    void names_data()
    {
        QTest::addColumn<int>("key");
        QTest::addColumn<int>("mods");
        QTest::addColumn<QString>("text");
        QTest::addColumn<QString>("expected");
        QTest::newRow("space") << int(Qt::Key_Space) << 0 << " " << "SPACE";
        QTest::newRow("shift+left") << int(Qt::Key_Left) << int(Qt::ShiftModifier) << "" << "Shift+LEFT";
        QTest::newRow("alt+up") << int(Qt::Key_Up) << int(Qt::AltModifier) << "" << "Alt+UP";
        QTest::newRow("ctrl+right") << int(Qt::Key_Right) << int(Qt::ControlModifier) << "" << "Ctrl+RIGHT";
        QTest::newRow("shift+pgup") << int(Qt::Key_PageUp) << int(Qt::ShiftModifier) << "" << "Shift+PGUP";
        QTest::newRow("f") << int(Qt::Key_F) << 0 << "f" << "f";
        QTest::newRow("shift+z") << int(Qt::Key_Z) << int(Qt::ShiftModifier) << "Z" << "Z";
        QTest::newRow("shift+(") << int(Qt::Key_ParenLeft) << int(Qt::ShiftModifier) << "(" << "(";
        QTest::newRow("ctrl+s") << int(Qt::Key_S) << int(Qt::ControlModifier) << "\x13" << "Ctrl+s";
        QTest::newRow("ctrl+shift+s") << int(Qt::Key_S) << int(Qt::ControlModifier | Qt::ShiftModifier) << "\x13" << "Ctrl+S";
        QTest::newRow("ctrl+[") << int(Qt::Key_BracketLeft) << int(Qt::ControlModifier) << "\x1b" << "Ctrl+[";
        QTest::newRow("alt+ctrl+]") << int(Qt::Key_BracketRight) << int(Qt::ControlModifier | Qt::AltModifier) << "" << "Ctrl+Alt+]";
        QTest::newRow("alt+v") << int(Qt::Key_V) << int(Qt::AltModifier) << "√" << "Alt+v";
        QTest::newRow("f11") << int(Qt::Key_F11) << 0 << "" << "F11";
        QTest::newRow("shift alone") << int(Qt::Key_Shift) << int(Qt::ShiftModifier) << "" << "";
    }
    void names()
    {
        QFETCH(int, key);
        QFETCH(int, mods);
        QFETCH(QString, text);
        QFETCH(QString, expected);
        QCOMPARE(mpvKeyName(key, mods, text), expected);
    }
};

QTEST_APPLESS_MAIN(KeysTest)
#include "keys_test.moc"
