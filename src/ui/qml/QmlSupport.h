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

class QMenu;
class QQuickWidget;
class QWidget;

// Published to every QML view as "host". Popups created in QML are clipped
// to the bounds of their QQuickWidget, so tooltips and menus are shown as
// native widgets instead.
class QmlHost : public QObject {
  Q_OBJECT

public:
  QmlHost(QQuickWidget *view);

  // Coordinates are in the scene coordinates of the view's root item.
  Q_INVOKABLE void showToolTip(const QString &text, qreal x, qreal y,
                               qreal width, qreal height);
  Q_INVOKABLE void hideToolTip();

  QPoint mapToGlobal(qreal x, qreal y) const;
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

} // namespace QmlSupport

#endif
