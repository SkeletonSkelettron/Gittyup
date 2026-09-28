//
//          Copyright (c) 2026, Gittyup contributors
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//

#include "QmlDialog.h"
#include "ui/qml/QmlSupport.h"
#include <QQuickItem>
#include <QQuickWidget>
#include <QVBoxLayout>
#include <QtMath>

QmlDialog::QmlDialog(QWidget *parent) : QDialog(parent) {
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
}

QmlDialog::~QmlDialog() {
  // The QML view references this object, so it has to go first.
  delete mView;
}

void QmlDialog::setContent(const QString &name, const QVariantMap &context) {
  QVariantMap map = context;
  map.insert("dialog", QVariant::fromValue<QObject *>(this));
  mView = QmlSupport::createView(name, map, this);
  layout()->addWidget(mView);

  // Follow the size of the content.
  if (QQuickItem *root = rootItem()) {
    connect(root, &QQuickItem::implicitWidthChanged, this,
            &QmlDialog::updateSize);
    connect(root, &QQuickItem::implicitHeightChanged, this,
            &QmlDialog::updateSize);
    root->forceActiveFocus();
  }

  updateSize();
  mView->setFocus();
}

QQuickItem *QmlDialog::rootItem() const {
  return mView ? mView->rootObject() : nullptr;
}

void QmlDialog::updateSize() {
  QQuickItem *root = rootItem();
  if (!root)
    return;

  QSize size(qCeil(root->implicitWidth()), qCeil(root->implicitHeight()));
  setMinimumSize(size);
  resize(size.expandedTo(isVisible() ? QSize(width(), 0) : QSize()));
}
