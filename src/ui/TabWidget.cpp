//
//          Copyright (c) 2016, Scientific Toolworks, Inc.
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//
// Author: Jason Haslam
//

#include "TabWidget.h"
#include "MenuBar.h"
#include "WelcomePage.h"
#include "qml/QmlSupport.h"
#include <QQuickWidget>
#include <QResizeEvent>
#include <QTabBar>

TabWidget::TabWidget(QWidget *parent) : QTabWidget(parent) {
  // The tab bar only keeps the tab names.
  QTabBar *bar = new QTabBar(this);
  bar->setMovable(true);
  bar->setTabsClosable(true);
  setTabBar(bar);
  setDocumentMode(true);
  bar->hide();

  // Create the welcome page.
  mWelcomePage = new WelcomePage(this);
  mWelcome = QmlSupport::createView(
      "WelcomePage", {{"welcome", QVariant::fromValue<QObject *>(mWelcomePage)}},
      this);
  connect(mWelcomePage, &WelcomePage::closeRequested, this,
          [this] { setWelcomeVisible(false); });
  updateWelcome();

  // Handle tab close.
  connect(this, &TabWidget::tabCloseRequested, [this](int index) {
    emit tabAboutToBeRemoved();
    widget(index)->close();
  });

  // Switching to a tab hides the welcome page.
  connect(this, &TabWidget::currentChanged, this,
          [this] { setWelcomeVisible(false); });
}

TabWidget::~TabWidget() {
  // The QML view references the welcome page, so it has to go first.
  delete mWelcome;
}

void TabWidget::setWelcomeVisible(bool visible) {
  if (visible == mWelcomeRequested)
    return;

  mWelcomeRequested = visible;
  updateWelcome();
}

void TabWidget::resizeEvent(QResizeEvent *event) {
  QTabWidget::resizeEvent(event);
  mWelcome->setGeometry(rect());
}

void TabWidget::tabInserted(int index) {
  QTabWidget::tabInserted(index);
  MenuBar::instance(this)->updateWindow();
  emit tabInserted();

  mWelcomeRequested = false;
  updateWelcome();
}

void TabWidget::tabRemoved(int index) {
  QTabWidget::tabRemoved(index);
  MenuBar::instance(this)->updateWindow();
  emit tabRemoved();

  updateWelcome();
}

void TabWidget::updateWelcome() {
  bool visible = !count() || mWelcomeRequested;
  mWelcomePage->setClosable(count() > 0);
  mWelcome->setGeometry(rect());
  mWelcome->setVisible(visible);
  if (visible) {
    mWelcome->raise();
    mWelcome->setFocus();
  }

  if (visible != mWelcomeVisible) {
    mWelcomeVisible = visible;
    emit welcomeChanged();
  }
}
