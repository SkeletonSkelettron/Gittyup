//
//          Copyright (c) 2016, Scientific Toolworks, Inc.
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//
// Author: Jason Haslam
//

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QAction>
#include <QToolBar>

class MainWindow;
class RepoView;
class SearchField;
class QMenu;
class QQuickWidget;

// The main window tool bar. The buttons are drawn by qrc:/qml/ToolBar.qml,
// which reads the properties below and calls the invokable methods.
class ToolBar : public QToolBar {
  Q_OBJECT

  Q_PROPERTY(bool hasView READ hasView NOTIFY stateChanged)
  Q_PROPERTY(bool sidebarVisible READ sidebarVisible NOTIFY stateChanged)
  Q_PROPERTY(QString repoName READ repoName NOTIFY stateChanged)
  Q_PROPERTY(QString repoPath READ repoPath NOTIFY stateChanged)
  Q_PROPERTY(QString branchName READ branchName NOTIFY stateChanged)
  Q_PROPERTY(bool canPrev READ canPrev NOTIFY stateChanged)
  Q_PROPERTY(bool canNext READ canNext NOTIFY stateChanged)
  Q_PROPERTY(bool canPull READ canPull NOTIFY stateChanged)
  Q_PROPERTY(bool canCheckout READ canCheckout NOTIFY stateChanged)
  Q_PROPERTY(bool canStash READ canStash NOTIFY stateChanged)
  Q_PROPERTY(bool canPop READ canPop NOTIFY stateChanged)
  Q_PROPERTY(int ahead READ ahead NOTIFY stateChanged)
  Q_PROPERTY(int behind READ behind NOTIFY stateChanged)
  Q_PROPERTY(bool logVisible READ logVisible NOTIFY stateChanged)
  Q_PROPERTY(int viewMode READ viewMode NOTIFY stateChanged)
  Q_PROPERTY(bool starred READ starred NOTIFY stateChanged)
  Q_PROPERTY(bool pullRequestAvailable READ pullRequestAvailable CONSTANT)

public:
  ToolBar(MainWindow *parent);
  ~ToolBar() override;

  SearchField *searchField() const { return mSearchField; }

  bool hasView() const { return mState.hasView; }
  bool sidebarVisible() const { return mState.sidebarVisible; }
  QString repoName() const { return mState.repoName; }
  QString repoPath() const { return mState.repoPath; }
  QString branchName() const { return mState.branchName; }
  bool canPrev() const { return mState.canPrev; }
  bool canNext() const { return mState.canNext; }
  bool canPull() const { return mState.canPull; }
  bool canCheckout() const { return mState.canCheckout; }
  bool canStash() const { return mState.canStash; }
  bool canPop() const { return mState.canPop; }
  int ahead() const { return mState.ahead; }
  int behind() const { return mState.behind; }
  bool logVisible() const { return mState.logVisible; }
  int viewMode() const { return mState.viewMode; }
  bool starred() const { return mState.starred; }
  bool pullRequestAvailable() const { return mPullRequestAvailable; }

  Q_INVOKABLE void toggleSideBar();
  Q_INVOKABLE void prev();
  Q_INVOKABLE void next();
  Q_INVOKABLE void showHistoryMenu(bool next, qreal x, qreal y);
  Q_INVOKABLE void fetch();
  Q_INVOKABLE void pull();
  Q_INVOKABLE void showPullMenu(qreal x, qreal y);
  Q_INVOKABLE void push();
  Q_INVOKABLE void checkout();
  Q_INVOKABLE void stash();
  Q_INVOKABLE void popStash();
  Q_INVOKABLE void refresh();
  Q_INVOKABLE void createPullRequest();
  Q_INVOKABLE void openTerminal();
  Q_INVOKABLE void openFileManager();
  Q_INVOKABLE void toggleLog();
  Q_INVOKABLE void setViewMode(int mode);
  Q_INVOKABLE void setStarred(bool starred);
  Q_INVOKABLE void showSettingsMenu(qreal x, qreal y);

signals:
  void stateChanged();

protected:
  void resizeEvent(QResizeEvent *event) override;

private:
  struct State {
    bool hasView = false;
    bool sidebarVisible = false;
    QString repoName;
    QString repoPath;
    QString branchName;
    bool canPrev = false;
    bool canNext = false;
    bool canPull = false;
    bool canCheckout = false;
    bool canStash = false;
    bool canPop = false;
    int ahead = 0;
    int behind = 0;
    bool logVisible = false;
    int viewMode = 0;
    bool starred = false;
  };

  void updateButtons(int ahead, int behind);
  void updateRemote(int ahead, int behind);
  void updateHistory();
  void updateStash();
  void updateView();
  void updateSearch();

  RepoView *currentView() const;

  State mState;
  bool mPullRequestAvailable = false;

  QQuickWidget *mView;
  QMenu *mPrevMenu;
  QMenu *mNextMenu;
  QMenu *mPullMenu;
  QMenu *mSettingsMenu;
  QAction *mRepoConfigAction;

  SearchField *mSearchField;

  friend class MainWindow;
  friend class RepoView;
};

#endif
