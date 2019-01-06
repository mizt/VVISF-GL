#include "MainWindow.h"
#include <QApplication>
#include <QDebug>

#include "OutputWindow.h"
#include "AutoUpdater.h"




#if defined(Q_OS_WIN)
extern "C"
{
	__declspec(dllexport) uint32_t NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif


int main(int argc, char *argv[])
{
	//	we want all the widgets to share contexts, and we need to make a widget to get that shard context (there's no way to tell a widget to use a given context)
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
	QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL, true);
	//QCoreApplication::setAttribute(Qt::AA_DontCheckOpenGLContextThreadAffinity, true);
	
	//	let's set up some basic info about the app so QSettings will be easier to use
	QCoreApplication::setOrganizationName("yourcompanyname");
	QCoreApplication::setOrganizationDomain("com.yourcompanyname");
	QCoreApplication::setApplicationName("ISFEditor");
	
	//QSurfaceFormat::setDefaultFormat(VVGL::CreateCompatibilityGLSurfaceFormat());
	QSurfaceFormat::setDefaultFormat(VVGL::CreateGL4SurfaceFormat());
	//QSurfaceFormat::setDefaultFormat(VVGL::CreateGL3SurfaceFormat());
	
	//			from sample code for QtAutoUpdater:
	//	"Since there is no mainwindow, the various dialogs should not quit the app"
	QApplication::setQuitOnLastWindowClosed(false);
	
	
	//	make the qApp
	QApplication a(argc, argv);
	
	//	make the auto updater (by default, its parent will be qApp)
	//GetGlobalAutoUpdater();
	AutoUpdater		*aa = new AutoUpdater(&a);
	
	//	make the main window, which has a GL view in it and will create the GL backend, and then finish launching.
	MainWindow		w;
	
	//	open the window after a slight delay, we want to give the app a chance to start all the other stuff
	QTimer::singleShot(500, [&]()	{
		w.show();
	});
	
	return a.exec();
}
