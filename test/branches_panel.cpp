//
//          Copyright (c) 2016, Scientific Toolworks, Inc.
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//
// Author: Shane Gramlich
//

#include "Test.h"
#include "dialogs/NewBranchDialog.h"
#include <QQuickWidget>
#include "dialogs/ConfigDialog.h"
#include "ui/Footer.h"
#include "ui/MainWindow.h"
#include "ui/RepoView.h"
#include <QComboBox>
#include <QDialog>
#include <QMenu>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableView>
#include <QToolButton>

using namespace Test;
using namespace QTest;

class TestBranchesPanel : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void createBranch();
  void cleanupTestCase();

private:
  int inputDelay = 0;
  int closeDelay = 0;

  ScratchRepository mRepo;
  MainWindow *mWindow = nullptr;
  ConfigDialog *mConfigDialog = nullptr;
};

void TestBranchesPanel::initTestCase() {
  mWindow = new MainWindow(mRepo);
  RepoView *view = mWindow->currentView();

  git::Remote remote = mRepo->addRemote(
      "origin", "https://github.com/Murmele/GittyupTestRepo.git");
  fetch(view, remote);

  git::Branch upstream =
      mRepo->lookupBranch("origin/master", GIT_BRANCH_REMOTE);
  QVERIFY(upstream.isValid());

  git::Branch branch =
      view->createBranch("master", upstream.target(), upstream, true);
  QVERIFY(branch.isValid());

  mWindow->show();
  mConfigDialog = view->configureSettings(ConfigDialog::Branches);
  QVERIFY(qWaitForWindowExposed(mConfigDialog));
}

void TestBranchesPanel::createBranch() {
  QStackedWidget *stack = mConfigDialog->findChild<QStackedWidget *>();
  QVERIFY(stack);

  // Click add branch icon
  QWidget *panel = stack->currentWidget();
  Footer *remotesFooter = panel->findChild<Footer *>();
  QToolButton *addRemote = remotesFooter->findChild<QToolButton *>();
  QVERIFY(addRemote);
  mouseClick(addRemote, Qt::LeftButton, Qt::KeyboardModifiers(), QPoint(),
             inputDelay);

  // The new branch dialog opens with the name field focused.
  NewBranchDialog *dialog = panel->findChild<NewBranchDialog *>();
  QVERIFY(dialog);
  QVERIFY(qWaitForWindowExposed(dialog));
  QQuickWidget *view = dialog->findChild<QQuickWidget *>();
  QVERIFY(view);
  keyClicks(view, "feature");
  QCOMPARE(dialog->name(), QString("feature"));

  // Select upstream origin/master.
  QVariantList upstreams = dialog->upstreams();
  int upstream = -1;
  for (int i = 0; i < upstreams.size(); ++i) {
    if (upstreams.at(i).toMap().value("text") == "origin/master")
      upstream = i;
  }
  QVERIFY(upstream > 0);
  dialog->setUpstreamIndex(upstream);

  // Accept.
  QVERIFY(dialog->isAcceptable());
  dialog->accept();

  git::Branch branch = mRepo->lookupBranch("feature", GIT_BRANCH_LOCAL);
  QVERIFY(branch.isValid());
  QCOMPARE(branch.upstream().name(), QString("origin/master"));

  // Verify branch created
  QTableView *branchTable = panel->findChild<QTableView *>();
  QVERIFY(branchTable);
  QVERIFY(branchTable->rowAt(0) != -1);
}

void TestBranchesPanel::cleanupTestCase() {
  qWait(closeDelay);
  mWindow->close();
}

TEST_MAIN(TestBranchesPanel)

#include "branches_panel.moc"
