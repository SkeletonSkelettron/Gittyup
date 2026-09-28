//
//          Copyright (c) 2026, Gittyup contributors
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//

#ifndef QMLPOPUP_H
#define QMLPOPUP_H

#include <QVariantMap>
#include <QWidget>

class QQuickItem;
class QQuickWidget;

// A window drawn by qrc:/qml/<name>.qml, which reads 'popup' and the
// objects of the context, shown below a field. QML views can't draw outside
// of their widget, so lists and panels that drop down from a field in the
// tool bar are separate windows. The height follows the implicit height of
// the root item.
class QmlPopup : public QWidget {
  Q_OBJECT

public:
  // A popup that takes the keyboard closes on a click outside of it, like a
  // menu. Otherwise it's shown without taking the focus from the field, like
  // a completion list.
  QmlPopup(const QString &name, const QVariantMap &context, bool keyboard,
           QWidget *parent);
  ~QmlPopup() override;

  QQuickItem *rootItem() const;

  // Show below 'field', a rectangle on the screen, at least as wide as it.
  void popup(const QRect &field, int width);

signals:
  void hidden();

protected:
  void showEvent(QShowEvent *event) override;
  void hideEvent(QHideEvent *event) override;

private:
  void updateSize();
  void syncView();

  QQuickWidget *mView = nullptr;
  QRect mField;
};

#endif
