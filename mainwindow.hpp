#include <QMainWindow>
#include <QTextEdit>
#include <QAction>
#include <QFileDialog>
#include <QTextDocumentFragment>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QTextDocument>
#include <QTextDocumentWriter>
#include <QMessageBox>
#include <QTextCharFormat>
#include <QStatusBar>

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);

private slots:
  void newFile();
  void openFile();
  void saveFile();
  // void saveAsFile();
  bool saveAsFile();
  void toggleBold();
  void toggleItalic();
  void toggleUnderline();

private:
  void createActions();
  void createMenus();
  void createToolBars();
  bool maybeSave();
  bool save(const QString &fileName);

  QTextEdit *textEdit;
  QString currentFile;

  QMenu *fileMenu;
  QMenu *formatMenu;

  QToolBar *fileToolBar;
  QToolBar *formatToolBar;

  QAction *newAct;
  QAction *openAct;
  QAction *saveAct;
  QAction *saveAsAct;
  QAction *exitAct;
  QAction *boldAct;
  QAction *italicAct;
  QAction *underlineAct;
};