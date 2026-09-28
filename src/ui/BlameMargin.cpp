//
//          Copyright (c) 2016, Scientific Toolworks, Inc.
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//
// Author: Jason Haslam
//

#include "BlameMargin.h"
#include "ProgressIndicator.h"
#include "app/Application.h"
#include "qml/QmlTheme.h"
#include "conf/Settings.h"
#include "editor/TextEditor.h"
#include "git/Commit.h"
#include "git/Repository.h"
#include "git/RevWalk.h"
#include "git/Signature.h"
#include <QDateTime>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QStyleOption>
#include <QTextLayout>
#include <QToolTip>
#include <QUrl>
#include <QUrlQuery>

namespace {

const QString kNowrapFmt = "<span style='white-space: nowrap'>%1</span><br>";

// Width of the bar that shows the age of the lines.
const qreal kAgeBarWidth = 3;

// A short, relative date like "3 days ago".
QString relativeDate(const QDateTime &dateTime) {
  qint64 secs = dateTime.secsTo(QDateTime::currentDateTime());
  auto format = [](qint64 n, const QString &one, const QString &many) {
    return (n == 1) ? one : many.arg(n);
  };

  if (secs < 60)
    return BlameMargin::tr("just now");
  if (secs < 3600)
    return format(secs / 60, BlameMargin::tr("1 minute ago"),
                  BlameMargin::tr("%1 minutes ago"));
  if (secs < 86400)
    return format(secs / 3600, BlameMargin::tr("1 hour ago"),
                  BlameMargin::tr("%1 hours ago"));
  if (secs < 86400 * 30)
    return format(secs / 86400, BlameMargin::tr("1 day ago"),
                  BlameMargin::tr("%1 days ago"));
  if (secs < 86400 * 365)
    return format(secs / (86400 * 30), BlameMargin::tr("1 month ago"),
                  BlameMargin::tr("%1 months ago"));
  return format(secs / (86400 * 365), BlameMargin::tr("1 year ago"),
                BlameMargin::tr("%1 years ago"));
}

} // namespace

BlameMargin::BlameMargin(TextEditor *editor, QWidget *parent)
    : QWidget(parent), mEditor(editor), mIndex(-1), mProgress(0) {
  setAttribute(Qt::WA_OpaquePaintEvent);
  setMouseTracking(true);

  // Can't connect directly because of different parameter types.
  QScrollBar *sb = editor->verticalScrollBar();
  connect(sb, &QScrollBar::valueChanged, [this] { update(); });

  // Update blame when lines are added or removed.
  connect(mEditor, &TextEditor::linesAdded, this, &BlameMargin::updateBlame);

  // Connect progress timer.
  connect(&mTimer, &QTimer::timeout, [this] {
    ++mProgress;
    update();
  });
}

void BlameMargin::startBlame(const QString &name) {
  mName = name;
  mProgress = 0;
  mTimer.start(50);
}

void BlameMargin::setBlame(const git::Repository &repo,
                           const git::Blame &blame) {
  if (Settings::instance()
          ->value(Setting::Id::ShowHeatmapInBlameMargin)
          .toBool()) {
    git::Commit first = repo.walker(GIT_SORT_TIME | GIT_SORT_REVERSE).next();
    git::Commit last = repo.walker(GIT_SORT_TIME).next();
    mMinTime = first ? first.committer().date().toSecsSinceEpoch() : -1;
    mMaxTime = last ? last.committer().date().toSecsSinceEpoch() : -1;
  }

  mTimer.stop();
  mSource = blame;
  updateBlame();
}

void BlameMargin::clear() {
  mName = QString();
  mBlame = git::Blame();
  mSource = git::Blame();

  mIndex = -1;
  mSelection = git::Id();

  mMinTime = -1;
  mMaxTime = -1;

  // Repaint.
  update();
}

QSize BlameMargin::minimumSizeHint() const { return QSize(160, 0); }

bool BlameMargin::event(QEvent *event) {
  if (event->type() != QEvent::ToolTip || !mBlame.isValid())
    return QWidget::event(event);

  QHelpEvent *help = static_cast<QHelpEvent *>(event);
  int index = this->index(help->y());
  if (index >= 0) {
    QStringList lines;

    QString name = this->name(index);
    if (!name.isEmpty())
      name = QString("<b>%1</b>").arg(name);

    QString email, date;
    git::Signature signature = mBlame.signature(index);
    if (signature.isValid()) {
      email = QString("&lt;%1&gt;").arg(signature.email());
      date = QLocale().toString(signature.date(), QLocale::LongFormat);
    }

    if (!name.isEmpty())
      lines << kNowrapFmt.arg(QString("%1 %2").arg(name, email));

    if (!date.isEmpty())
      lines << kNowrapFmt.arg(date);

    lines << mBlame.id(index).toString();

    QString msg = mBlame.message(index);
    if (!msg.isEmpty())
      lines << QString("<p>%1</p>").arg(msg);

    QToolTip::showText(help->globalPos(), lines.join('\n'), this);

  } else {
    QToolTip::hideText();
    event->ignore();
  }

  return true;
}

void BlameMargin::mousePressEvent(QMouseEvent *event) {
  mIndex = mBlame.isValid() ? index(event->position().y()) : -1;
}

void BlameMargin::mouseReleaseEvent(QMouseEvent *event) {
  if (mBlame.isValid() && mIndex >= 0 &&
      mIndex == index(event->position().y())) {
    // Update selection.
    git::Id id = mBlame.id(mIndex);
    mSelection = (mSelection != id) ? id : git::Id();
    update();
  }

  mIndex = -1;
}

void BlameMargin::mouseDoubleClickEvent(QMouseEvent *event) {
  if (!mBlame.isValid())
    return;

  int index = this->index(event->position().y());
  if (index < 0)
    return;

  QUrlQuery query;
  query.addQueryItem("file", mName);

  QUrl url;
  url.setScheme("id");
  url.setPath(mBlame.id(index).toString());
  url.setQuery(query);

  emit linkActivated(url.toString());
}

void BlameMargin::paintEvent(QPaintEvent *event) {
  QmlTheme *theme = QmlTheme::instance();
  QPainter painter(this);
  painter.fillRect(rect(), theme->panel());

  // Separate the margin from the text.
  painter.fillRect(QRectF(width() - 1, 0, 1, height()), theme->border());

  // Draw busy indicator.
  if (!mBlame.isValid()) {
    QRect rect(0, 10, width(), ProgressIndicator::size().height());
    ProgressIndicator::paint(&painter, rect, theme->textMuted(), mProgress);
    return;
  }

  // Draw items.
  painter.setRenderHints(QPainter::Antialiasing);

  int size = mEditor->styleSize(STYLE_DEFAULT) - 1;
  QFont regular = font();
  regular.setPointSize(size);
  QFontMetricsF regularMetrics(regular);

  QFont small = regular;
  small.setPointSize(qMax(6, size - 1));
  QFontMetricsF smallMetrics(small);

  int lh = mEditor->textHeight(0);
  int lc = mEditor->lineCount() + 1;
  int first = mEditor->firstVisibleLine() + 1;
  int last = first + mEditor->linesOnScreen();

  int count = mBlame.count();
  int index = mBlame.index(first);
  while (index < count && mBlame.line(index) < last) {
    // Combine adjacent lines with the same id.
    int line = mBlame.line(index);
    git::Id id = mBlame.id(index);
    if (id.isNull()) {
      // This can happen when the commit is for some reason not valid, for
      // example if the email address is not specified
      index++;
      continue;
    }

    bool invalid = false;
    while (index + 1 < count) {
      const auto idNext = mBlame.id(index + 1);
      if (idNext.isNull()) {
        invalid = true;
        break;
      } else if (idNext == id) {
        index++;
      } else {
        break;
      }
    }
    if (invalid) {
      index++;
      continue;
    }

    // Calculate outer rectangle.
    int next = (index + 1 < count) ? mBlame.line(index + 1) : lc;
    QRectF rect(0, (line - first) * lh, width() - 1, (next - line) * lh);

    git::Signature signature = mBlame.signature(index);
    int time = signature.isValid() ? signature.date().toSecsSinceEpoch() : -1;

    // Highlight the selected commit.
    if (id == mSelection) {
      painter.fillRect(rect, theme->selected());
    } else if (id == mHover) {
      painter.fillRect(rect, theme->hover());
    }

    // Draw the age of the lines as a bar from cold to hot.
    QColor age = theme->textDisabled();
    if (time >= 0 && mMinTime >= 0 && mMaxTime >= 0 && mMinTime != mMaxTime &&
        Settings::instance()
            ->value(Setting::Id::ShowHeatmapInBlameMargin)
            .toBool()) {
      qreal val = qreal(time - mMinTime) / (mMaxTime - mMinTime);
      QColor cold = Application::theme()->heatMap(Theme::HeatMap::Cold);
      QColor hot = Application::theme()->heatMap(Theme::HeatMap::Hot);
      cold.setAlphaF(1);
      hot.setAlphaF(1);
      age = QColor::fromRgbF(
          cold.redF() + (hot.redF() - cold.redF()) * val,
          cold.greenF() + (hot.greenF() - cold.greenF()) * val,
          cold.blueF() + (hot.blueF() - cold.blueF()) * val);
    }

    QRectF bar(rect.x() + 2, rect.y() + 2, kAgeBarWidth, rect.height() - 4);
    painter.setPen(Qt::NoPen);
    painter.setBrush(age);
    painter.drawRoundedRect(bar, 1.5, 1.5);

    // Draw separator line wholly within the current cell.
    painter.fillRect(QRectF(rect.left(), rect.bottom() - 1, rect.width(), 1),
                     theme->border());

    // Calculate inner rectangle.
    rect.setY(qMax(0.0, rect.y()));
    rect.adjust(kAgeBarWidth + 10, 0, -8, 0);

    // Draw the date on the right of the summary.
    QString date = signature.isValid() ? relativeDate(signature.date()) : "";
    qreal dateWidth = date.isEmpty() ? 0 : smallMetrics.horizontalAdvance(date);
    QRectF firstLine(rect.x(), rect.y(), rect.width(), lh);
    painter.setFont(small);
    painter.setPen(theme->textMuted());
    painter.drawText(firstLine, Qt::AlignRight | Qt::AlignVCenter, date);

    // Draw the summary of the commit.
    QString summary = mBlame.isCommitted(index)
                          ? mBlame.message(index).section('\n', 0, 0)
                          : tr("Not Committed");
    painter.setFont(regular);
    painter.setPen(mBlame.isCommitted(index) ? theme->text()
                                             : theme->textMuted());
    QRectF summaryRect = firstLine.adjusted(0, 0, -(dateWidth + 8), 0);
    painter.drawText(summaryRect, Qt::AlignLeft | Qt::AlignVCenter,
                     regularMetrics.elidedText(summary, Qt::ElideRight,
                                               summaryRect.width()));

    // Draw the author below if there is room.
    if (next - qMax(line, first) > 1 && mBlame.isCommitted(index)) {
      QRectF secondLine(rect.x(), rect.y() + lh, rect.width(), lh);
      QString author = name(index);
      painter.setFont(small);
      painter.setPen(theme->textMuted());
      painter.drawText(secondLine, Qt::AlignLeft | Qt::AlignVCenter,
                       smallMetrics.elidedText(author, Qt::ElideRight,
                                               secondLine.width()));
    }

    ++index;
  }
}

void BlameMargin::mouseMoveEvent(QMouseEvent *event) {
  int index = mBlame.isValid() ? this->index(event->position().y()) : -1;
  git::Id id = (index >= 0) ? mBlame.id(index) : git::Id();
  if (id != mHover) {
    mHover = id;
    update();
  }

  QWidget::mouseMoveEvent(event);
}

void BlameMargin::leaveEvent(QEvent *event) {
  mHover = git::Id();
  update();
  QWidget::leaveEvent(event);
}

void BlameMargin::wheelEvent(QWheelEvent *event) {
  // Forward to the editor.
  mEditor->wheelEvent(event);
}

void BlameMargin::updateBlame() {
  if (!mSource.isValid())
    return;

#if 0
  char *data = reinterpret_cast<char *>(mEditor->characterPointer());
  mBlame = mSource.updated(QByteArray::fromRawData(data, mEditor->length()));
#else
  mBlame = mSource;
#endif

  update();
}

int BlameMargin::index(int y) const {
  int line = mEditor->firstVisibleLine() + (y / mEditor->textHeight(0));
  return (line < mEditor->lineCount()) ? mBlame.index(line + 1) : -1;
}

QString BlameMargin::name(int index) const {
  if (!mBlame.isCommitted(index))
    return tr("Not Committed");

  git::Signature signature = mBlame.signature(index);
  return signature.isValid() ? signature.name() : tr("Invalid Signature");
}
