
#include "pch.h"
#include "MainWindow.h"
#include "mdichild.h"

cMainWindow::cMainWindow()
   : QMainWindow()
   , m_ui(new Ui::CopperSpiceUIClass)
   , m_mdiArea(new QMdiArea)
{
   m_ui->setupUi(this);
   //m_mdiArea->setViewMode(QMdiArea::TabbedView);
   setCentralWidget(m_mdiArea);
}

void cMainWindow::on_actionOpen_triggered()
{
}

bool cMainWindow::openFile(const QString &fileName)
{
   if (QMdiSubWindow *existing = findMdiChild(fileName)) {
      m_mdiArea->setActiveSubWindow(existing);
      return true;
   }
   const bool succeeded = loadFile(fileName);
   if (succeeded) {
      statusBar()->showMessage(tr("File loaded"), 2000);
   }
   return succeeded;
}

bool cMainWindow::loadFile(const QString &fileName)
{
   cMdiChild *child = createMdiChild();
   const bool succeeded = child->loadFile(fileName);
   if (succeeded) {
      child->showMaximized();
   }
   else {
      child->close();
   }
   prependToRecentFiles(fileName);
   return succeeded;
}

cMdiChild *cMainWindow::createMdiChild()
{
   auto child = new cMdiChild;
   m_mdiArea->addSubWindow(child);

   return child;
}

QMdiSubWindow *cMainWindow::findMdiChild(const QString &fileName) const
{
   QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

   const QList<QMdiSubWindow *> subWindows = m_mdiArea->subWindowList();
   for (QMdiSubWindow *window: subWindows) {
      auto mdiChild = qobject_cast<cMdiChild *>(window->widget());
      if (mdiChild->currentFile() == canonicalFilePath) {
         return window;
      }
   }
   return nullptr;
}

void cMainWindow::prependToRecentFiles(const QString &fileName)
{
   //QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());

   //const QStringList oldRecentFiles = readRecentFiles(settings);
   //QStringList recentFiles = oldRecentFiles;
   //recentFiles.removeAll(fileName);
   //recentFiles.prepend(fileName);
   //if (oldRecentFiles != recentFiles)
   //   writeRecentFiles(recentFiles, settings);

   //setRecentFilesVisible(!recentFiles.isEmpty());
}
