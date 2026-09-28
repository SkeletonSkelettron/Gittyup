//
//          Copyright (c) 2016, Scientific Toolworks, Inc.
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//
// Author: Jason Haslam
//

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>

class QQuickWidget;
class WelcomePage;

// The repository views of a main window. The tabs are drawn by the TabStrip
// above the tool bar, so the tab bar is hidden. The welcome page covers the
// views when no repository is open or a new tab is requested.
class TabWidget : public QTabWidget {
  Q_OBJECT

public:
  TabWidget(QWidget *parent = nullptr);
  ~TabWidget() override;

  bool isWelcomeVisible() const { return mWelcomeVisible; }
  void setWelcomeVisible(bool visible);

signals:
  void tabAboutToBeInserted();
  void tabAboutToBeRemoved();
  void tabInserted();
  void tabRemoved();
  void welcomeChanged();

protected:
  void resizeEvent(QResizeEvent *event) override;
  void tabInserted(int index) override;
  void tabRemoved(int index) override;

private:
  void updateWelcome();

  QQuickWidget *mWelcome;
  WelcomePage *mWelcomePage;
  bool mWelcomeRequested = false;
  bool mWelcomeVisible = false;
};

#endif
