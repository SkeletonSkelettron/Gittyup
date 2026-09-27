//
//          Copyright (c) 2016, Scientific Toolworks, Inc.
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//
// Author: Jason Haslam
//

#include "ToolBar.h"
#include "History.h"
#include "MainWindow.h"
#include "RepoView.h"
#include "SearchField.h"
#include "dialogs/PullRequestDialog.h"
#include "dialogs/SettingsDialog.h"
#include "git/Branch.h"
#include "qml/QmlSupport.h"
#include "qml/QmlTheme.h"
#include "ui/HotkeyManager.h"
#include <QMenu>
#include <QQuickWidget>
#include <QRegularExpression>
#include <QShortcut>

namespace {

const int kToolBarHeight = 52;
const int kSearchFieldWidth = 220;
const int kNarrowSearchFieldWidth = 150;
const int kNarrowWidth = 1100;
const int kSearchFieldHeight = 28;
const QString kStarredQuery = "is:starred";

static Hotkey terminalHotkey = HotkeyManager::registerHotkey(
    nullptr, "tools/terminal", "Tools/Open Terminal");

static Hotkey fileManagerHotkey = HotkeyManager::registerHotkey(
    nullptr, "tools/fileManager", "Tools/Open File Manager");

} // namespace

ToolBar::ToolBar(MainWindow *parent) : QToolBar(parent) {
  Q_ASSERT(parent);

  setMovable(false);
  setObjectName("toolbar");
  setFixedHeight(kToolBarHeight);
  setContentsMargins(0, 0, 0, 0);

  // Match the QML background so the search field blends in.
  QmlTheme *theme = QmlTheme::instance();
  setStyleSheet(QString("ToolBar {"
                        "  background: %1;"
                        "  border: none;"
                        "  border-bottom: 1px solid %2;"
                        "  padding: 0px;"
                        "  spacing: 0px"
                        "}"
                        "SearchField {"
                        "  background: %3;"
                        "  color: %4;"
                        "  border: 1px solid %2;"
                        "  border-radius: 6px"
                        "}"
                        "SearchField:focus {"
                        "  border: 1px solid %5"
                        "}")
                    .arg(theme->toolbar().name(), theme->border().name(),
                         theme->field().name(), theme->text().name(),
                         theme->accent().name()));

  // Disable the built-in context menu.
  setContextMenuPolicy(Qt::PreventContextMenu);

  mPullRequestAvailable = !qgetenv("GITTYUP_OAUTH").isEmpty();

  // Menus are native so they aren't clipped to the QML view.
  mPrevMenu = new QMenu(this);
  connect(mPrevMenu, &QMenu::triggered, [this](QAction *action) {
    currentView()->history()->setIndex(action->data().toInt());
  });

  mNextMenu = new QMenu(this);
  connect(mNextMenu, &QMenu::triggered, [this](QAction *action) {
    currentView()->history()->setIndex(action->data().toInt());
  });

  mPullMenu = new QMenu(this);
  QAction *mergeAction = mPullMenu->addAction(tr("Merge"));
  connect(mergeAction, &QAction::triggered,
          [this] { currentView()->pull(RepoView::Merge); });

  QAction *rebaseAction = mPullMenu->addAction(tr("Rebase"));
  connect(rebaseAction, &QAction::triggered,
          [this] { currentView()->pull(RepoView::Rebase); });

  mSettingsMenu = new QMenu(this);
  mRepoConfigAction = mSettingsMenu->addAction(tr("Repository settings"));
  connect(mRepoConfigAction, &QAction::triggered,
          [this] { currentView()->configureSettings(); });

  QAction *appConfigAction =
      mSettingsMenu->addAction(tr("Application settings"));
  connect(appConfigAction, &QAction::triggered,
          [] { SettingsDialog::openSharedInstance(); });

  // Create the QML buttons.
  mView = QmlSupport::createView(
      "ToolBar", {{"toolbar", QVariant::fromValue<QObject *>(this)}}, this);
  mView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  mView->setMinimumHeight(kToolBarHeight - 1);
  addWidget(mView);

  // The search field stays a widget for its completer and advanced search.
  mSearchField = new SearchField(this);
  mSearchField->setFixedSize(kSearchFieldWidth, kSearchFieldHeight);
  addWidget(mSearchField);

  QWidget *spacer = new QWidget(this);
  spacer->setFixedWidth(8);
  spacer->setAttribute(Qt::WA_TransparentForMouseEvents);
  addWidget(spacer);

  connect(mSearchField, &SearchField::textChanged, [this](const QString &text) {
    QStringList terms = text.split(QRegularExpression("\\s+"));
    bool starred = terms.contains(kStarredQuery);
    if (starred != mState.starred) {
      mState.starred = starred;
      emit stateChanged();
    }
  });

  QShortcut *shortcut = new QShortcut(this);
  terminalHotkey.use(shortcut);
  connect(shortcut, &QShortcut::activated, this, &ToolBar::openTerminal);

  shortcut = new QShortcut(this);
  fileManagerHotkey.use(shortcut);
  connect(shortcut, &QShortcut::activated, this, &ToolBar::openFileManager);
}

ToolBar::~ToolBar() {
  // The QML view references this object, so it has to go first.
  delete mView;
}

void ToolBar::resizeEvent(QResizeEvent *event) {
  QToolBar::resizeEvent(event);
  bool narrow = (width() < kNarrowWidth);
  mSearchField->setFixedWidth(narrow ? kNarrowSearchFieldWidth
                                     : kSearchFieldWidth);
}

void ToolBar::toggleSideBar() {
  MainWindow *window = static_cast<MainWindow *>(parent());
  window->setSideBarVisible(!window->isSideBarVisible());
}

void ToolBar::prev() {
  if (RepoView *view = currentView())
    view->history()->prev();
}

void ToolBar::next() {
  if (RepoView *view = currentView())
    view->history()->next();
}

void ToolBar::showHistoryMenu(bool next, qreal x, qreal y) {
  RepoView *view = currentView();
  if (!view)
    return;

  QMenu *menu = next ? mNextMenu : mPrevMenu;
  if (next) {
    view->history()->updateNextMenu(menu);
  } else {
    view->history()->updatePrevMenu(menu);
  }

  if (!menu->isEmpty())
    QmlSupport::host(mView)->popup(menu, x, y);
}

void ToolBar::fetch() {
  if (RepoView *view = currentView())
    view->fetch();
}

void ToolBar::pull() {
  if (RepoView *view = currentView())
    view->pull();
}

void ToolBar::showPullMenu(qreal x, qreal y) {
  if (mState.canPull)
    QmlSupport::host(mView)->popup(mPullMenu, x, y);
}

void ToolBar::push() {
  if (RepoView *view = currentView())
    view->push();
}

void ToolBar::checkout() {
  if (RepoView *view = currentView())
    view->promptToCheckout();
}

void ToolBar::stash() {
  if (RepoView *view = currentView())
    view->promptToStash();
}

void ToolBar::popStash() {
  if (RepoView *view = currentView())
    view->popStash();
}

void ToolBar::refresh() {
  if (RepoView *view = currentView())
    view->refresh();
}

void ToolBar::createPullRequest() {
  if (RepoView *view = currentView()) {
    PullRequestDialog *dialog = new PullRequestDialog(view);
    dialog->open();
  }
}

void ToolBar::openTerminal() {
  if (RepoView *view = currentView())
    view->openTerminal();
}

void ToolBar::openFileManager() {
  if (RepoView *view = currentView())
    view->openFileManager();
}

void ToolBar::toggleLog() {
  if (RepoView *view = currentView())
    view->setLogVisible(!view->isLogVisible());
}

void ToolBar::setViewMode(int mode) {
  if (RepoView *view = currentView())
    view->setViewMode(static_cast<RepoView::ViewMode>(mode));
}

void ToolBar::setStarred(bool starred) {
  QStringList terms = mSearchField->text().split(QRegularExpression("\\s+"),
                                                 Qt::SkipEmptyParts);
  if (starred) {
    if (!terms.contains(kStarredQuery))
      terms.append(kStarredQuery);
  } else {
    terms.removeAll(kStarredQuery);
  }

  mSearchField->setText(terms.join(' '));
}

void ToolBar::showSettingsMenu(qreal x, qreal y) {
  QmlSupport::host(mView)->popup(mSettingsMenu, x, y);
}

void ToolBar::updateButtons(int ahead, int behind) {
  RepoView *view = currentView();
  mState.hasView = view;
  mState.canCheckout = view && !view->repo().isBare();

  mState.repoName.clear();
  mState.repoPath.clear();
  mState.branchName.clear();
  if (view) {
    git::Repository repo = view->repo();
    QDir dir = repo.dir(false);
    mState.repoName = dir.dirName();
    mState.repoPath = dir.path();

    git::Reference head = repo.head();
    mState.branchName = head.isValid() ? head.name() : repo.unbornHeadName();
  }

  // Each of these emits stateChanged.
  updateRemote(ahead, behind);
  updateHistory();
  updateStash();
  updateView();
  updateSearch();
}

void ToolBar::updateRemote(int ahead, int behind) {
  RepoView *view = currentView();
  mState.ahead = ahead;
  mState.behind = behind;
  mState.canPull = view && !view->repo().isBare();
  emit stateChanged();
}

void ToolBar::updateHistory() {
  RepoView *view = currentView();
  History *history = view ? view->history() : nullptr;
  mState.canPrev = history && history->hasPrev();
  mState.canNext = history && history->hasNext();
  emit stateChanged();
}

void ToolBar::updateStash() {
  RepoView *view = currentView();
  mState.canStash = view && view->isWorkingDirectoryDirty();
  mState.canPop = view && view->repo().stashRef().isValid();
  emit stateChanged();
}

void ToolBar::updateView() {
  RepoView *view = currentView();
  MainWindow *window = static_cast<MainWindow *>(parent());
  mState.sidebarVisible = window->isSideBarVisible();
  mState.logVisible = view && view->isLogVisible();
  if (view)
    mState.viewMode = view->viewMode();
  mRepoConfigAction->setEnabled(view);
  emit stateChanged();
}

void ToolBar::updateSearch() { mSearchField->setEnabled(currentView()); }

RepoView *ToolBar::currentView() const {
  return static_cast<MainWindow *>(parent())->currentView();
}
