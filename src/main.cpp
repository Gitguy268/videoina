#include "mpvitem.h"

#include <QCommandLineParser>
#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QTimer>
#include <QUrl>
#include <clocale>

int main(int argc, char *argv[])
{
    // MpvItem renders through OpenGL (mpv's render API), so pin the scene graph to it.
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    QGuiApplication app(argc, argv);
    app.setApplicationName("Videoina");
    std::setlocale(LC_NUMERIC, "C"); // libmpv requires it; Qt resets it above

    QCommandLineParser cli;
    cli.addPositionalArgument("files", "Media files or URLs to play.");
    // Hidden smoke-test hook used by CI: grab the window after N ms, save it, quit.
    QCommandLineOption shot("screenshot", "Save a window screenshot and quit.", "png");
    QCommandLineOption after("screenshot-after", "Delay before the screenshot, ms.", "ms", "3000");
    cli.addOptions({shot, after});
    cli.addHelpOption();
    cli.process(app);

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] { QCoreApplication::exit(1); },
                     Qt::QueuedConnection);
    engine.loadFromModule("Videoina", "Main");
    if (engine.rootObjects().isEmpty())
        return 1;

    auto window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
    auto mpv = window->findChild<MpvItem *>();
    QObject::connect(mpv, &MpvItem::shutdown, &app, &QCoreApplication::quit);
    const auto files = cli.positionalArguments();
    for (int i = 0; i < files.size(); i++) {
        QUrl url = QUrl::fromUserInput(files[i], QDir::currentPath(), QUrl::AssumeLocalFile);
        mpv->command({"loadfile", url.isLocalFile() ? url.toLocalFile() : url.toString(), i ? "append" : "replace"});
    }

    if (cli.isSet(shot)) {
        QTimer::singleShot(cli.value(after).toInt(), window, [&] {
            bool played = mpv->state().value("time-pos").toDouble() > 0;
            bool saved = window->grabWindow().save(cli.value(shot));
            qInfo("smoke: played=%d saved=%d", played, saved);
            QCoreApplication::exit(played && saved ? 0 : 2);
        });
    }
    return app.exec();
}
