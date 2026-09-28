//
//          Copyright (c) 2026, Gittyup contributors
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//

#include "QmlPopup.h"
#include "qml/QmlSupport.h"
#include <QGuiApplication>
#include <QQuickItem>
#include <QQuickWidget>
#include <QQuickWindow>
#include <QScreen>
#include <QTimer>
#include <QtMath>

QmlPopup::QmlPopup(const QString &name, const QVariantMap &context,
                   bool keyboard, QWidget *parent)
    : QWidget(parent, (keyboard ? Qt::Popup : Qt::ToolTip) |
                          Qt::FramelessWindowHint) {
  if (!keyboard)
    setAttribute(Qt::WA_ShowWithoutActivating);

  QVariantMap map = context;
  map.insert("popup", QVariant::fromValue<QObject *>(this));
  mView = QmlSupport::createView(name, map, this);

  if (QQuickItem *root = rootItem())
    connect(root, &QQuickItem::implicitHeightChanged, this,
            &QmlPopup::updateSize);
}

QmlPopup::~QmlPopup() {
  // The QML view references the context objects, so it has to go first.
  delete mView;
}

QQuickItem *QmlPopup::rootItem() const { return mView->rootObject(); }

void QmlPopup::popup(const QRect &field, int width) {
  mField = field;
  resize(qMax(width, field.width()), height());
  updateSize();
  show();
  raise();
  if (testAttribute(Qt::WA_ShowWithoutActivating))
    return;

  activateWindow();
  mView->setFocus();
}

void QmlPopup::showEvent(QShowEvent *event) {
  QWidget::showEvent(event);
  QTimer::singleShot(0, this, &QmlPopup::syncView);
}

void QmlPopup::hideEvent(QHideEvent *event) {
  QWidget::hideEvent(event);
  emit hidden();
}

void QmlPopup::updateSize() {
  QQuickItem *root = rootItem();
  if (!root)
    return;

  QSize size(width(), qMax(1, qCeil(root->implicitHeight())));

  // Align the right edges of a wider popup and the field, and keep it on
  // the screen.
  QPoint pos(mField.right() + 1 - size.width(), mField.bottom() + 5);
  if (QScreen *screen = QGuiApplication::screenAt(mField.center())) {
    QRect available = screen->availableGeometry();
    pos.setX(qBound(available.left(), pos.x(),
                    available.right() + 1 - size.width()));
    size.setHeight(qMin(size.height(), available.bottom() + 1 - pos.y()));
  }

  setGeometry(QRect(pos, size));
  mView->setGeometry(rect());
  QTimer::singleShot(0, this, &QmlPopup::syncView);
}

void QmlPopup::syncView() {
  if (mView->geometry() != rect())
    mView->setGeometry(rect());

  // A QQuickWidget that is resized before its window is exposed keeps
  // drawing at its old size, so resize it again after that.
  if (mView->quickWindow()->size() == mView->size())
    return;

  QSize size = mView->size();
  mView->resize(size + QSize(0, 1));
  mView->resize(size);
}
