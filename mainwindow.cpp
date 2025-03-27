#include "mainwindow.hpp"
#include <QMessageBox>
#include <QTextCharFormat>
#include <QStatusBar>
#include <QProcess>
#include <QTemporaryFile>
#include <QTextStream>
#include <QTextBlock>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), textEdit(new QTextEdit)
{
  setCentralWidget(textEdit);
  textEdit->setAcceptRichText(true);
  textEdit->document()->setModified(false);
  createActions();
  createMenus();
  createToolBars();
  setWindowTitle("RTF Editor");
  resize(800, 600);
}

void MainWindow::createActions()
{
  newAct = new QAction(QIcon::fromTheme("document-new"), "&New", this);
  connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

  openAct = new QAction(QIcon::fromTheme("document-open"), "&Open...", this);
  connect(openAct, &QAction::triggered, this, &MainWindow::openFile);

  saveAct = new QAction(QIcon::fromTheme("document-save"), "&Save", this);
  connect(saveAct, &QAction::triggered, this, &MainWindow::saveFile);

  saveAsAct = new QAction("Save &As...", this);
  connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveAsFile);

  exitAct = new QAction("E&xit", this);
  connect(exitAct, &QAction::triggered, this, &QWidget::close);

  boldAct = new QAction(QIcon::fromTheme("format-text-bold"), "&Bold", this);
  boldAct->setCheckable(true);
  connect(boldAct, &QAction::triggered, this, &MainWindow::toggleBold);

  italicAct = new QAction(QIcon::fromTheme("format-text-italic"), "&Italic", this);
  italicAct->setCheckable(true);
  connect(italicAct, &QAction::triggered, this, &MainWindow::toggleItalic);

  underlineAct = new QAction(QIcon::fromTheme("format-text-underline"), "&Underline", this);
  underlineAct->setCheckable(true);
  connect(underlineAct, &QAction::triggered, this, &MainWindow::toggleUnderline);
}

void MainWindow::createMenus()
{
  fileMenu = menuBar()->addMenu("&File");
  fileMenu->addAction(newAct);
  fileMenu->addAction(openAct);
  fileMenu->addAction(saveAct);
  fileMenu->addAction(saveAsAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  formatMenu = menuBar()->addMenu("&Format");
  formatMenu->addAction(boldAct);
  formatMenu->addAction(italicAct);
  formatMenu->addAction(underlineAct);
}

void MainWindow::createToolBars()
{
  fileToolBar = addToolBar("File");
  fileToolBar->addAction(newAct);
  fileToolBar->addAction(openAct);
  fileToolBar->addAction(saveAct);

  formatToolBar = addToolBar("Format");
  formatToolBar->addAction(boldAct);
  formatToolBar->addAction(italicAct);
  formatToolBar->addAction(underlineAct);
}

void MainWindow::newFile()
{
  if (maybeSave())
  {
    textEdit->clear();
    currentFile.clear();
  }
}

void MainWindow::openFile()
{
    if (maybeSave())
    {
        QString fileName = QFileDialog::getOpenFileName(this, "Open File", "",
                                                      "Rich Text Files (*.rtf)");
        if (!fileName.isEmpty())
        {
            QFile file(fileName);
            if (file.open(QIODevice::ReadOnly))
            {
                // Read the file content
                QByteArray data = file.readAll();
                file.close();

                if (data.startsWith("{\\rtf"))
                {
                    // Create a new document
                    QTextDocument *doc = new QTextDocument;
                    QTextCursor cursor(doc);
                    
                    // Process the RTF content
                    QString rtfContent = QString::fromLocal8Bit(data);
                    
                    // Remove RTF header and color table
                    int contentStart = rtfContent.indexOf("\\colortbl");
                    contentStart = rtfContent.indexOf("}", contentStart);
                    if (contentStart != -1) {
                        contentStart++;
                        rtfContent = rtfContent.mid(contentStart);
                    }
                    
                    // Remove the final closing brace
                    if (rtfContent.endsWith("}")) {
                        rtfContent.chop(1);
                    }
                    
                    // Split into lines and process
                    QStringList lines = rtfContent.split("\\par");
                    bool firstLine = true;
                    
                    for (const QString &line : lines) 
                    {
                        QString cleanLine = line;
                        cleanLine = cleanLine.trimmed();
                        
                        if (!firstLine) {
                            cursor.insertBlock();
                        }
                        
                        if (!cleanLine.isEmpty()) {
                            cursor.insertText(cleanLine);
                        }
                        
                        firstLine = false;
                    }
                    
                    // Set the document in the text edit
                    textEdit->setDocument(doc);
                    textEdit->document()->setModified(false);
                    
                    currentFile = fileName;
                }
                else
                {
                    QMessageBox::warning(this, "Error", "Invalid RTF file format");
                }
            }
            else
            {
                QMessageBox::warning(this, "Error", "Could not open file");
            }
        }
    }
}

bool MainWindow::save(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly))
    {
        QTextDocument *doc = textEdit->document();
        QTextCursor cursor(doc);
        QString rtf = "{\\rtf1\\ansi\\deff0{\\fonttbl{\\f0\\fswiss\\fcharset0 Arial;}}\n";
        rtf += "{\\colortbl;\\red0\\green0\\blue0;}\n";

        bool currentBold = false;
        bool currentItalic = false;
        bool currentUnderline = false;
        bool needsSpace = false;

        // Iterate through text blocks (paragraphs)
        QTextBlock block = doc->begin();
        while (block.isValid())
        {
            QString blockText = block.text();
            
            // Process each character in the block
            for (int i = 0; i < blockText.length(); ++i)
            {
                cursor.setPosition(block.position() + i);
                cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);

                QTextCharFormat fmt = cursor.charFormat();
                QString ch = cursor.selectedText();

                bool newBold = fmt.fontWeight() > QFont::Normal;
                bool newItalic = fmt.fontItalic();
                bool newUnderline = fmt.fontUnderline();

                // Update formatting commands
                auto updateFormat = [&](bool &current, bool newState, const QString &tag)
                {
                    if (newState != current)
                    {
                        rtf += QString("%1\\%2")
                                   .arg(needsSpace ? " " : "")
                                   .arg(newState ? tag : tag + "0");
                        current = newState;
                        needsSpace = true;
                    }
                };

                updateFormat(currentBold, newBold, "b");
                updateFormat(currentItalic, newItalic, "i");
                updateFormat(currentUnderline, newUnderline, "ul");

                // Process character
                if (!ch.isEmpty())
                {
                    QString escaped = ch.toHtmlEscaped()
                                        .replace("\\", "\\\\")
                                        .replace("{", "\\{")
                                        .replace("}", "\\}");

                    rtf += QString("%1%2").arg(needsSpace ? " " : "").arg(escaped);
                    needsSpace = false;
                }
            }

            // Add paragraph break after each block
            block = block.next();
            if (block.isValid())
            {
                rtf += "\\par\n"; // Removed \pard
                needsSpace = false;
                currentBold = currentItalic = currentUnderline = false;
            }
        }

        // Close the RTF document
        rtf += "}";

        // Write using UTF-8 encoding
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << rtf;
        file.close();
        
        textEdit->document()->setModified(false);
        return true;
    }
    return false;
}

void MainWindow::saveFile()
{
  if (currentFile.isEmpty())
  {
    saveAsFile();
  }
  else
  {
    save(currentFile);
  }
}

bool MainWindow::saveAsFile()
{ // Changed return type
  QString fileName = QFileDialog::getSaveFileName(this, "Save As", QString(),
                                                  "Rich Text Files (*.rtf)");
  if (!fileName.isEmpty())
  {
    return save(fileName);
  }
  return false; // Return false if canceled
}

void MainWindow::toggleBold()
{
  QTextCharFormat fmt;
  fmt.setFontWeight(boldAct->isChecked() ? QFont::Bold : QFont::Normal);
  textEdit->mergeCurrentCharFormat(fmt);
}

void MainWindow::toggleItalic()
{
  QTextCharFormat fmt;
  fmt.setFontItalic(italicAct->isChecked());
  textEdit->mergeCurrentCharFormat(fmt);
}

void MainWindow::toggleUnderline()
{
  QTextCharFormat fmt;
  fmt.setFontUnderline(underlineAct->isChecked());
  textEdit->mergeCurrentCharFormat(fmt);
}

bool MainWindow::maybeSave()
{
  if (!textEdit->document()->isModified())
    return true;

  QMessageBox::StandardButton ret = QMessageBox::warning(
      this, "Application",
      "The document has been modified.\nDo you want to save your changes?",
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

  if (ret == QMessageBox::Save)
  {
    if (currentFile.isEmpty())
    {
      return saveAsFile(); // Now returns bool
    }
    else
    {
      return save(currentFile);
    }
  }
  else if (ret == QMessageBox::Cancel)
  {
    return false;
  }
  return true;
}