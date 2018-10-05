#include <QDesktopWidget>

#include "threads.h"
#include "context.h"
#include "qquickimage.h"
#include "backgroundconnect.h"
#include "contextplugin.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    int h = 40;
    QDesktopWidget desktop;

    ContextPlugin plugins;

    Context *ctx = new Context();
    ctx->basepath = app.applicationDirPath();

    QQuickImage image;
    QQuickImg img;
    image.ctx = ctx;

    Threads *threads = new Threads();

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("ContextPlugin", &plugins);
    engine.rootContext()->setContextProperty("Context", ctx);
    engine.addImageProvider("pixmap", &image);
    engine.addImageProvider("grab", &img);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    QObject *main = engine.rootObjects().first();
    QWindow *window = qobject_cast<QWindow *>(main); // (QWindow *)main
    window->setGeometry(0, desktop.height() - h, desktop.width(), h);
    window->setProperty("mainId", window->winId());

    //QMainWindow *wind = (QMainWindow *)window;
    //wind->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    //wind->setAttribute(Qt::WA_X11NetWmWindowTypeDock);
    //window->xChanged(Qt::WA_X11NetWmWindowTypeDock);

    window->xChanged(Qt::FramelessWindowHint| Qt::WindowStaysOnTopHint| Qt::WA_X11NetWmWindowTypeDock);
    //app.setAttribute(Qt::AA_X11InitThreads);

    threads->main = window;
    threads->ctx = ctx;

    ctx->xreservedSpace(window->winId(), window->height());
    ctx->xchangeProperty(window->winId(), "Desktop", "_OB_APP_TYPE", XA_STRING);
    //4294967295
    ctx->xchangeProperty(window->winId(), window->winId(), "_NET_WM_DESKTOP", XA_CARDINAL);
    //hack to openbox
    ctx->openboxChange(window->winId(), ALL_DESKTOPS);
    //"_NET_WM_WINDOW_TYPE_DOCK" "_NET_WM_WINDOW_TYPE_DESKTOP"
    ctx->xchange(window->winId(), "_NET_WM_WINDOW_TYPE_DOCK");

    BackgroundConnect bg;
    bg.main = window;
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerObject("/", &bg, QDBusConnection::ExportAllSlots);
    connection.connect("emmi.interface.background", "/", "emmi.interface.background", "BgConnect", &bg, SLOT(BgConnect()));
    connection.registerService("emmi.interface.background");

    threads->start();

    return app.exec();
}
