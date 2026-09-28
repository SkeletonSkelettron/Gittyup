//
//          Copyright (c) 2026, Gittyup contributors
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//

#include "Test.h"
#include "dialogs/AboutDialog.h"
#include "dialogs/AccountDialog.h"
#include "dialogs/AddRemoteDialog.h"
#include "dialogs/AmendDialog.h"
#include "dialogs/CheckoutDialog.h"
#include "dialogs/CloneDialog.h"
#include "dialogs/CommitDialog.h"
#include "dialogs/ConfigDialog.h"
#include "dialogs/ConfirmDialog.h"
#include "dialogs/ExternalToolsDialog.h"
#include "dialogs/InputDialog.h"
#include "dialogs/MergeDialog.h"
#include "dialogs/NewBranchDialog.h"
#include "dialogs/PluginsDialog.h"
#include "dialogs/RemoteDialog.h"
#include "dialogs/RenameBranchDialog.h"
#include "dialogs/SettingsDialog.h"
#include "dialogs/TagDialog.h"
#include "dialogs/ThemeDialog.h"
#include "dialogs/UpdateSubmodulesDialog.h"
#include "git/Branch.h"
#include "git/Commit.h"
#include "git/Index.h"
#include "log/LogEntry.h"
#include "ui/IgnoreDialog.h"
#include "ui/DetailView.h"
#include "ui/FindController.h"
#include "ui/MainWindow.h"
#include "ui/RepoView.h"
#include "ui/TabWidget.h"
#include "ui/TemplateDialog.h"
#include "ui/MenuBar.h"
#include "ui/SearchField.h"
#include "ui/ToolBar.h"
#include "ui/qml/QmlSupport.h"
#include "update/UpdateDialog.h"
#include <QClipboard>
#include <QMenu>
#include <QTimer>
#include <QFile>
#include <QQuickItem>
#include <QQuickWidget>

using namespace Test;
using namespace QTest;

namespace {

// Warnings and errors of the QML files.
QStringList sMessages;
QtMessageHandler sPrevious = nullptr;

void handleMessage(QtMsgType type, const QMessageLogContext &context,
                   const QString &message) {
  if (message.contains("qrc:/qml/"))
    sMessages.append(message);
  if (sPrevious)
    sPrevious(type, context, message);
}

} // namespace

// Opens the main window and every QML dialog and checks that their QML
// loads without warnings.
class TestQmlViews : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void mainWindow();
  void dialogs();
  void settings();
  void search();
  void menu();
  void dragTab();
  void cleanupTestCase();

private:
  // Show the dialog and check its QML.
  void check(QDialog *dialog, const QString &name);

  ScratchRepository mRepo;
  MainWindow *mWindow = nullptr;
};

void TestQmlViews::initTestCase() {
  sPrevious = qInstallMessageHandler(handleMessage);

  // A commit and a change.
  QFile file(mRepo->workdir().filePath("file.txt"));
  QVERIFY(file.open(QFile::WriteOnly));
  file.write("content\n");
  file.close();
  mRepo->index().setStaged({"file.txt"}, true);
  QVERIFY(mRepo->commit("initial"));

  QVERIFY(file.open(QFile::WriteOnly | QFile::Append));
  file.write("change\n");
  file.close();

  mWindow = new MainWindow(mRepo);
  mWindow->show();
  QVERIFY(qWaitForWindowExposed(mWindow));
}

void TestQmlViews::mainWindow() {
  RepoView *view = mWindow->currentView();
  refresh(view);

  // The commit, the log and the welcome page.
  view->selectFirstCommit();
  view->addLogEntry("text", "Title")->addEntry(LogEntry::Error, "error");
  view->setLogVisible(true);
  mWindow->tabWidget()->setWelcomeVisible(true);
  qWait(200);
  mWindow->tabWidget()->setWelcomeVisible(false);
  qWait(100);

  // The content of a file with its blame in tree mode.
  view->setViewMode(RepoView::Tree);
  DetailView *details = view->findChild<DetailView *>();
  details->selectPath("file.txt");
  QCOMPARE(details->file(), QString("file.txt"));
  QAbstractItemModel *content =
      qobject_cast<QAbstractItemModel *>(details->contentModel());
  QVERIFY(content->rowCount() > 0);
  QTRY_VERIFY(!content->property("blameLoading").toBool());
  QVERIFY(content->property("hasBlame").toBool());
  QCOMPARE(content->index(0, 0).data(Qt::UserRole + 3).toString().isEmpty(),
           false);

  // Find in the file.
  FindController *finder = qobject_cast<FindController *>(details->finder());
  finder->show();
  finder->search("change");
  QVERIFY(finder->hasMatches());
  QCOMPARE(finder->hitsText(), QString("1 of 1"));
  qWait(100);
  details->closeFile();
  QVERIFY(!finder->isVisible());
  view->setViewMode(RepoView::DoubleTree);
  qWait(100);

  QVERIFY2(sMessages.isEmpty(), qPrintable(sMessages.join('\n')));
}

void TestQmlViews::dialogs() {
  RepoView *view = mWindow->currentView();
  git::Repository repo = view->repo();
  git::Commit head = repo.head().target();

  check(new NewBranchDialog(repo, git::Commit(), view), "NewBranchDialog");
  check(new TagDialog(repo, head.shortId(), git::Remote(), view), "TagDialog");
  check(new CheckoutDialog(repo, repo.head(), view), "CheckoutDialog");
  check(new RenameBranchDialog(repo, repo.head(), view), "RenameBranchDialog");
  check(new AddRemoteDialog("origin", view), "AddRemoteDialog");
  check(new MergeDialog(RepoView::Merge, repo, view), "MergeDialog");
  check(new CloneDialog(CloneDialog::Clone, view), "CloneDialog");
  check(new CloneDialog(CloneDialog::Init, view), "CloneDialog (init)");
  check(new AmendDialog(head.author(), head.committer(), head.message(), view),
        "AmendDialog");
  check(new CommitDialog("message", Prompt::Kind::Stash, view),
        "CommitDialog");
  check(new AccountDialog(nullptr, view), "AccountDialog");
  check(new RemoteDialog(RemoteDialog::Push, view), "RemoteDialog");
  check(new UpdateSubmodulesDialog(repo, view), "UpdateSubmodulesDialog");
  check(new IgnoreDialog("*.log", view), "IgnoreDialog");
  check(new ThemeDialog(view), "ThemeDialog");
  check(new AboutDialog(view), "AboutDialog");
  check(new PluginsDialog(repo, view), "PluginsDialog");
  check(new ExternalToolsDialog("diff", view), "ExternalToolsDialog");
  check(new UpdateDialog("linux", "99.0.0", "<p>Notes</p>", "", view),
        "UpdateDialog");
  check(new InputDialog("Title", "Text",
                        {{"Username", "name"}, {"Password", "", true}}, view),
        "InputDialog");

  ConfirmDialog *confirm = new ConfirmDialog(view);
  confirm->setTitle("Title");
  confirm->setText("Text");
  confirm->setDetailedText("Details");
  confirm->setCheckText("Check");
  confirm->addButton("Other");
  check(confirm, "ConfirmDialog");

  QList<CommitTemplates::Template> templates = {{"Name", "Value"}};
  TemplateDialog *templateDialog = new TemplateDialog(templates, view);
  check(templateDialog, "TemplateDialog");
}

void TestQmlViews::settings() {
  RepoView *view = mWindow->currentView();

  // Every section of the application settings.
  SettingsDialog *settings = new SettingsDialog(SettingsDialog::General);
  for (int i = SettingsDialog::General; i <= SettingsDialog::Terminal; ++i)
    settings->setSection(i);
  check(settings, "SettingsDialog");

  // Every section of the repository settings.
  ConfigDialog *config = new ConfigDialog(view);
  for (int i = ConfigDialog::General; i <= ConfigDialog::Lfs; ++i)
    config->setSection(i);
  check(config, "ConfigDialog");
}

void TestQmlViews::search() {
  SearchField *search = mWindow->toolBar()->searchField();
  QVERIFY(search->isEnabled());

  // The advanced search fills its fields from the query.
  search->setText("author:someone words");
  search->showAdvanced();
  QVERIFY(search->advancedVisible());
  qWait(200);
  search->setAdvancedValue(0, "other");
  search->acceptAdvanced();
  QCOMPARE(search->text(), QString("other author:someone"));
  QVERIFY(!search->advancedVisible());

  // The edit menu works on the focused QML field.
  search->setText("hello");
  QQuickWidget *view = mWindow->quickView();
  QQuickItem *input =
      view->rootObject()->findChild<QQuickItem *>("searchInput");
  QVERIFY(input);
  mWindow->activateWindow();
  QVERIFY(qWaitForWindowActive(mWindow));
  view->setFocus();
  qWait(50);
  input->forceActiveFocus();
  qWait(50);
  QApplication::clipboard()->clear();

  MenuBar *menuBar = MenuBar::instance(mWindow);
  QMenu *edit = nullptr;
  for (QMenu *menu : menuBar->findChildren<QMenu *>()) {
    if (menu->title() == "Edit")
      edit = menu;
  }
  QVERIFY(edit);

  QAction *selectAll = nullptr;
  QAction *copy = nullptr;
  for (QAction *action : edit->actions()) {
    if (action->text() == "Select All")
      selectAll = action;
    else if (action->text() == "Copy")
      copy = action;
  }
  QVERIFY(selectAll && copy);

  // Opening the menu updates the actions.
  emit edit->aboutToShow();
  QVERIFY(selectAll->isEnabled());
  QVERIFY(!copy->isEnabled());
  selectAll->trigger();
  emit edit->aboutToShow();
  QVERIFY(copy->isEnabled());
  copy->trigger();
  QCOMPARE(QApplication::clipboard()->text(), QString("hello"));

  search->edit("", 0);
  QVERIFY2(sMessages.isEmpty(), qPrintable(sMessages.join('\n')));
}

void TestQmlViews::menu() {
  QMenu menu;
  QAction *first = menu.addAction("First");
  menu.addSeparator();
  QMenu *more = menu.addMenu("More");
  more->addAction("Second");

  bool triggered = false;
  connect(first, &QAction::triggered, [&triggered] { triggered = true; });

  // The view of the window draws the menu.
  QQuickWidget *view = mWindow->quickView();
  QPoint pos = view->mapToGlobal(QPoint(300, 300));
  QCOMPARE(QApplication::widgetAt(pos), view);

  QTimer::singleShot(200, [view] {
    keyClick(view, Qt::Key_Down);
    keyClick(view, Qt::Key_Return);
  });

  QCOMPARE(QmlSupport::execMenu(&menu, pos), first);
  QVERIFY(triggered);
  QVERIFY2(sMessages.isEmpty(), qPrintable(sMessages.join('\n')));
}

void TestQmlViews::dragTab() {
  // A second tab.
  ScratchRepository other;
  QVERIFY(mWindow->addTab(other));
  QCOMPARE(mWindow->count(), 2);
  mWindow->resize(1000, 700);
  qWait(100);

  QString first = mWindow->tabWidget()->tabText(0);
  QString second = mWindow->tabWidget()->tabText(1);

  // The tabs are at the top of the view of the window.
  QQuickWidget *view = mWindow->quickView();
  QVERIFY(view);

  // Drag the first tab past the second one. The tabs are below the menu
  // bar when the view draws it.
  QPoint start(40, (mWindow->isMenuBarVisible() ? 28 : 0) + 20);
  mousePress(view, Qt::LeftButton, Qt::NoModifier, start);
  for (int x = 10; x <= 240; x += 10)
    mouseMove(view, start + QPoint(x, 0));
  mouseRelease(view, Qt::LeftButton, Qt::NoModifier, start + QPoint(240, 0));
  qWait(100);

  QCOMPARE(mWindow->tabWidget()->tabText(0), second);
  QCOMPARE(mWindow->tabWidget()->tabText(1), first);

  mWindow->tabWidget()->widget(1)->close();
  qWait(100);
}

void TestQmlViews::cleanupTestCase() {
  qInstallMessageHandler(sPrevious);
  mWindow->close();
}

void TestQmlViews::check(QDialog *dialog, const QString &name) {
  sMessages.clear();
  dialog->setAttribute(Qt::WA_DeleteOnClose, false);
  dialog->show();
  QVERIFY2(qWaitForWindowExposed(dialog), qPrintable(name));
  qWait(50);

  QQuickWidget *view = dialog->findChild<QQuickWidget *>();
  QVERIFY2(view, qPrintable(name));
  QVERIFY2(view->status() == QQuickWidget::Ready, qPrintable(name));
  QVERIFY2(view->rootObject(), qPrintable(name));

  // Switching sections creates their content.
  if (SettingsDialog *settings = qobject_cast<SettingsDialog *>(dialog)) {
    for (int i = SettingsDialog::General; i <= SettingsDialog::Terminal; ++i) {
      settings->setSection(i);
      qWait(20);
    }
  } else if (ConfigDialog *config = qobject_cast<ConfigDialog *>(dialog)) {
    for (int i = ConfigDialog::General; i <= ConfigDialog::Lfs; ++i) {
      config->setSection(i);
      qWait(20);
    }
  }

  dialog->hide();
  delete dialog;

  QVERIFY2(sMessages.isEmpty(),
           qPrintable(name + ":\n" + sMessages.join('\n')));
}

TEST_MAIN(TestQmlViews)

#include "qml_views.moc"
