//
//          Copyright (c) 2026, Gittyup contributors
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//

#include "QmlTheme.h"
#include "app/Application.h"
#include "app/Theme.h"
#include <QPalette>

namespace {

QColor mix(const QColor &a, const QColor &b, qreal t) {
  return QColor::fromRgbF(a.redF() + (b.redF() - a.redF()) * t,
                          a.greenF() + (b.greenF() - a.greenF()) * t,
                          a.blueF() + (b.blueF() - a.blueF()) * t);
}

} // namespace

QmlTheme *QmlTheme::instance() {
  static QmlTheme *instance = new QmlTheme;
  return instance;
}

QmlTheme::QmlTheme() {
  QPalette palette;
  QColor window = palette.color(QPalette::Window);
  QColor base = palette.color(QPalette::Base);
  QColor text = palette.color(QPalette::WindowText);
  QColor highlight = palette.color(QPalette::Highlight);
  mDark = (text.lightnessF() > window.lightnessF());

  // Derive every color from the widget palette so that themes without a
  // theme['ui'] section still get a consistent QML interface.
  mColors = {
      {"toolbar", window},
      {"sidebar", base},
      {"field", base},
      {"border", mix(window, text, 0.15)},
      {"text", text},
      {"text_muted", mix(text, window, 0.4)},
      // Many themes don't define disabled colors, so blend instead.
      {"text_disabled", mix(text, window, 0.65)},
      {"hover", mix(window, text, 0.08)},
      {"pressed", mix(window, text, 0.16)},
      {"selected", highlight},
      {"selected_text", palette.color(QPalette::HighlightedText)},
      {"accent", highlight},
      {"accent_text", palette.color(QPalette::HighlightedText)},
      {"badge", highlight},
      {"badge_text", palette.color(QPalette::HighlightedText)},
      {"ahead", highlight},
      {"behind", highlight},
      {"star", QColor("#FFCE6D")},
      {"tooltip", palette.color(QPalette::ToolTipBase)},
      {"tooltip_text", palette.color(QPalette::ToolTipText)},
  };

  if (Theme *theme = Application::theme()) {
    QColor notification = theme->badge(Theme::BadgeRole::Background,
                                       Theme::BadgeState::Notification);
    if (notification.isValid()) {
      mColors.insert("badge", notification);
      mColors.insert("ahead", notification);
      mColors.insert("behind", notification);
      mColors.insert("badge_text",
                     theme->badge(Theme::BadgeRole::Foreground,
                                  Theme::BadgeState::Notification));
    }

    QColor star = theme->star();
    if (star.isValid())
      mColors.insert("star", star);

    // Explicit theme['ui'] entries win.
    QVariantMap ui = theme->ui();
    for (auto it = ui.cbegin(); it != ui.cend(); ++it) {
      QColor color(it.value().toString());
      if (color.isValid())
        mColors.insert(it.key(), color);
    }

    if (ui.contains("dark"))
      mDark = ui.value("dark").toBool();
  }
}

QColor QmlTheme::color(const QString &key) const {
  return mColors.value(key).value<QColor>();
}
