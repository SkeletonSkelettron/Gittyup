//
//          Copyright (c) 2026, Gittyup contributors
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//

#include "QmlSupport.h"
#include "CommitGraphItem.h"
#include "QmlTheme.h"
#include "host/Account.h"
#include <QBuffer>
#include <QFile>
#include <QHash>
#include <QImage>
#include <QImageReader>
#include <QMenu>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickImageProvider>
#include <QQuickWidget>
#include <QToolTip>

namespace {

const QSize kDefaultIconSize(48, 48);

// Serves "image://icons/<name>/<rrggbb>". Monochrome SVG icons from
// qrc:/qml/icons/<name>.svg are drawn with 'currentColor', which is replaced
// by the requested color. "account-<kind>" serves the remote account icons.
class IconProvider : public QQuickImageProvider {
public:
  IconProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

  QImage requestImage(const QString &id, QSize *size,
                      const QSize &requestedSize) override {
    QSize target = requestedSize.isValid() && !requestedSize.isEmpty()
                       ? requestedSize
                       : kDefaultIconSize;

    QString name = id.section('/', 0, 0);
    QString color = id.section('/', 1, 1);

    QImage image;
    if (name.startsWith("account-")) {
      int kind = name.mid(8).toInt();
      QIcon icon = Account::icon(static_cast<Account::Kind>(kind));
      image = icon.pixmap(target).toImage();
    } else {
      QFile file(QString(":/qml/icons/%1.svg").arg(name));
      if (file.open(QIODevice::ReadOnly)) {
        QByteArray svg = file.readAll();
        QColor tint(color.length() == 8 ? "#" + color.right(6) : "#" + color);
        if (tint.isValid())
          svg.replace("currentColor", tint.name(QColor::HexRgb).toUtf8());

        QBuffer buffer(&svg);
        QImageReader reader(&buffer, "svg");
        reader.setScaledSize(target);
        image = reader.read();
      }
    }

    if (size)
      *size = image.size();
    return image;
  }
};

} // namespace

QmlHost::QmlHost(QQuickWidget *view) : QObject(view), mView(view) {}

void QmlHost::showToolTip(const QString &text, qreal x, qreal y, qreal width,
                          qreal height) {
  QRect rect = QRectF(x, y, width, height).toAlignedRect();
  QPoint pos = mView->mapToGlobal(QPoint(rect.left(), rect.bottom() + 4));
  QToolTip::showText(pos, text, mView, rect);
}

void QmlHost::hideToolTip() { QToolTip::hideText(); }

QPoint QmlHost::mapToGlobal(qreal x, qreal y) const {
  return mView->mapToGlobal(QPointF(x, y).toPoint());
}

void QmlHost::popup(QMenu *menu, qreal x, qreal y) const {
  QToolTip::hideText();
  menu->popup(mapToGlobal(x, y));
}

namespace {

// Serves "image://images/<key>" from the images added to the store.
QHash<QString, QImage> sImages;
int sNextImage = 0;

class ImageProvider : public QQuickImageProvider {
public:
  ImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

  QImage requestImage(const QString &id, QSize *size,
                      const QSize &requestedSize) override {
    QImage image = sImages.value(id);
    if (size)
      *size = image.size();
    if (!image.isNull() && requestedSize.isValid() && !requestedSize.isEmpty())
      image = image.scaled(requestedSize, Qt::KeepAspectRatio,
                           Qt::SmoothTransformation);
    return image;
  }
};

} // namespace

namespace QmlSupport {

QQuickWidget *createView(const QString &name, const QVariantMap &context,
                         QWidget *parent) {
  static bool registered = false;
  if (!registered) {
    registered = true;
    qmlRegisterSingletonType<QmlTheme>(
        "Gittyup", 1, 0, "Theme", [](QQmlEngine *, QJSEngine *) -> QObject * {
          QmlTheme *theme = QmlTheme::instance();
          QQmlEngine::setObjectOwnership(theme, QQmlEngine::CppOwnership);
          return theme;
        });
    qmlRegisterType<CommitGraphItem>("Gittyup", 1, 0, "CommitGraph");
  }

  QQuickWidget *view = new QQuickWidget(parent);
  view->setResizeMode(QQuickWidget::SizeRootObjectToView);
  view->setClearColor(QmlTheme::instance()->toolbar());
  view->engine()->addImageProvider("icons", new IconProvider);
  view->engine()->addImageProvider("images", new ImageProvider);

  QQmlContext *root = view->rootContext();
  root->setContextProperty("host", new QmlHost(view));
  for (auto it = context.cbegin(); it != context.cend(); ++it)
    root->setContextProperty(it.key(), it.value());

  view->setSource(QUrl(QString("qrc:/qml/%1.qml").arg(name)));
  if (view->status() == QQuickWidget::Error) {
    for (const QQmlError &error : view->errors())
      qWarning("%s", qPrintable(error.toString()));
  }

  return view;
}

QmlHost *host(QQuickWidget *view) { return view->findChild<QmlHost *>(); }

QString addImage(const QImage &image) {
  QString key = QString::number(sNextImage++);
  sImages.insert(key, image);
  return QString("image://images/%1").arg(key);
}

void removeImage(const QString &url) {
  sImages.remove(url.section('/', -1));
}

} // namespace QmlSupport
