//
//          Copyright (c) 2026, Gittyup contributors
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//

#ifndef QMLTHEME_H
#define QMLTHEME_H

#include <QColor>
#include <QObject>
#include <QVariantMap>

// Exposes the current theme's colors to QML as the "Theme" singleton of the
// "Gittyup" module. Themes are only applied at startup, so every property is
// constant.
class QmlTheme : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool dark READ dark CONSTANT)

  Q_PROPERTY(QColor toolbar READ toolbar CONSTANT)
  Q_PROPERTY(QColor sidebar READ sidebar CONSTANT)
  Q_PROPERTY(QColor field READ field CONSTANT)
  Q_PROPERTY(QColor border READ border CONSTANT)

  Q_PROPERTY(QColor text READ text CONSTANT)
  Q_PROPERTY(QColor textMuted READ textMuted CONSTANT)
  Q_PROPERTY(QColor textDisabled READ textDisabled CONSTANT)

  Q_PROPERTY(QColor hover READ hover CONSTANT)
  Q_PROPERTY(QColor pressed READ pressed CONSTANT)
  Q_PROPERTY(QColor selected READ selected CONSTANT)
  Q_PROPERTY(QColor selectedText READ selectedText CONSTANT)

  Q_PROPERTY(QColor accent READ accent CONSTANT)
  Q_PROPERTY(QColor accentText READ accentText CONSTANT)

  Q_PROPERTY(QColor badge READ badge CONSTANT)
  Q_PROPERTY(QColor badgeText READ badgeText CONSTANT)
  Q_PROPERTY(QColor ahead READ ahead CONSTANT)
  Q_PROPERTY(QColor behind READ behind CONSTANT)
  Q_PROPERTY(QColor star READ star CONSTANT)

  Q_PROPERTY(QColor tooltip READ tooltip CONSTANT)
  Q_PROPERTY(QColor tooltipText READ tooltipText CONSTANT)

public:
  static QmlTheme *instance();

  bool dark() const { return mDark; }

  QColor toolbar() const { return color("toolbar"); }
  QColor sidebar() const { return color("sidebar"); }
  QColor field() const { return color("field"); }
  QColor border() const { return color("border"); }

  QColor text() const { return color("text"); }
  QColor textMuted() const { return color("text_muted"); }
  QColor textDisabled() const { return color("text_disabled"); }

  QColor hover() const { return color("hover"); }
  QColor pressed() const { return color("pressed"); }
  QColor selected() const { return color("selected"); }
  QColor selectedText() const { return color("selected_text"); }

  QColor accent() const { return color("accent"); }
  QColor accentText() const { return color("accent_text"); }

  QColor badge() const { return color("badge"); }
  QColor badgeText() const { return color("badge_text"); }
  QColor ahead() const { return color("ahead"); }
  QColor behind() const { return color("behind"); }
  QColor star() const { return color("star"); }

  QColor tooltip() const { return color("tooltip"); }
  QColor tooltipText() const { return color("tooltip_text"); }

  // Look up a color by its theme['ui'] key.
  QColor color(const QString &key) const;

private:
  QmlTheme();

  bool mDark = false;
  QVariantMap mColors;
};

#endif
