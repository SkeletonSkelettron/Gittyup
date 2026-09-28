//
//          Copyright (c) 2016, Scientific Toolworks, Inc.
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//
// Author: Jason Haslam
//

#include "Test.h"
#include "editor/TextEditor.h"
#include "ui/BlameEditor.h"
#include "ui/EditorWindow.h"
#include "ui/FindWidget.h"
#include "ui/MenuBar.h"
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include "dialogs/ConfirmDialog.h"
#include <QPushButton>

using namespace QTest;

class TestEditor : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void insertText();
  void copyPaste();
  void find();
  void cleanupTestCase();

private:
  EditorWindow *mWindow = nullptr;
  BlameEditor *mBlameEditor = nullptr;
  TextEditor *mEditor = nullptr;
};

void TestEditor::initTestCase() {
  mWindow = new EditorWindow;
  mBlameEditor = mWindow->widget();
  mEditor = mBlameEditor->editor();
  mWindow->show();
  QVERIFY(qWaitForWindowActive(mWindow));
}

void TestEditor::insertText() {
  keyClicks(mEditor, "This is a test.");
  keyClick(mEditor, Qt::Key_Return);
}

void TestEditor::copyPaste() {
  keyClick(mEditor, 'A', Qt::ControlModifier);
  keyClick(mEditor, 'C', Qt::ControlModifier);
  keyClick(mEditor, Qt::Key_Down);
  keyClick(mEditor, 'V', Qt::ControlModifier);
}

void TestEditor::find() {
  // It's difficult to trigger the shortcut on macOS.
  MenuBar *menuBar = MenuBar::instance(mWindow);
  QAction *findAction = menuBar->findChild<QAction *>("Find");
  QVERIFY(findAction);
  findAction->trigger();

  FindWidget *find = mBlameEditor->findChild<FindWidget *>();
  QVERIFY(find);

  QLineEdit *field = find->findChild<QLineEdit *>();
  QVERIFY(field && field->hasFocus());

  keyClicks(field, "test");
  QLabel *label = find->findChild<QLabel *>();
  QCOMPARE(label->text(), QString("2 matches"));
}

void TestEditor::cleanupTestCase() {
  // Set up timer to dismiss the dialog.
  QTimer::singleShot(0, [] {
    ConfirmDialog *dialog =
        qobject_cast<ConfirmDialog *>(QApplication::activeModalWidget());
    QVERIFY(dialog && qWaitForWindowActive(dialog));

    // Don't save.
    QCOMPARE(dialog->buttons().size(), 1);
    dialog->clickButton(0);
    QVERIFY(!dialog->isVisible());
  });

  mWindow->close();
}

TEST_MAIN(TestEditor)

#include "editor.moc"
