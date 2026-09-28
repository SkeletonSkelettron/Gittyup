//
//          Copyright (c) 2026, Gittyup contributors
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//

#ifndef QMLSUPPORT_H
#define QMLSUPPORT_H

#include <QObject>
#include <QPoint>
#include <QString>
#include <QVariantMap>

class QAction;
class QImage;
class QMenu;
class QQuickWidget;
class QWidget;

// Published to every QML view as "host". Tool tips are native widgets, and
// menus are drawn in the view if it allows it, see execMenu().
class QmlHost : public QObject {
  Q_OBJECT

public:
  QmlHost(QQuickWidget *view);

  // Coordinates are in the scene coordinates of the view's root item.
  Q_INVOKABLE void showToolTip(const QString &text, qreal x, qreal y,
                               qreal width, qreal height);
  Q_INVOKABLE void hideToolTip();

  QPoint mapToGlobal(qreal x, qreal y) const;
  // Show 'menu' at a point of the scene and wait until it closes.
  void popup(QMenu *menu, qreal x, qreal y) const;

private:
  QQuickWidget *mView;
};

namespace QmlSupport {

// Create a view showing qrc:/qml/<name>.qml. Each entry of 'context' is
// published to QML as a context property, along with "host". The view
// uses the current theme and can load tinted icons from
// "image://icons/<name>/<rrggbb>".
QQuickWidget *createView(const QString &name, const QVariantMap &context,
                         QWidget *parent);

// Get the host object of a view created with createView().
QmlHost *host(QQuickWidget *view);

// Views that are big enough for menus, like the view of the main window,
// draw them with qrc:/qml/ContextMenu.qml instead of showing native menus.
void setDrawsMenus(QQuickWidget *view, bool draws);

// Show the actions of 'menu' at 'pos' on the screen and wait until the
// menu closes, like QMenu::exec(). The menu is drawn by the view at 'pos'
// if it draws menus. Returns the triggered action.
QAction *execMenu(QMenu *menu, const QPoint &pos);

// Images for QML at "image://images/<key>". Remove them when they're no
// longer shown.
QString addImage(const QImage &image);
void removeImage(const QString &url);

} // namespace QmlSupport

#endif
