//
//          Copyright (c) 2026, Gittyup contributors
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//

#include "DiffModel.h"
#include "dialogs/ConfirmDialog.h"
#include "RepoView.h"
#include "app/Application.h"
#include "app/Theme.h"
#include "git/Blob.h"
#include "git/Commit.h"
#include "git/Index.h"
#include "git/Reference.h"
#include "git/Repository.h"
#include "git/Tree.h"
#include "git2/diff.h"
#include <QDir>
#include <QPushButton>
#include <QSaveFile>
#include <QTextStream>

namespace {

// Diffs with more lines are only loaded on request.
const int kMaxLines = 20000;
const int kTabWidth = 4;

// Append 'text' to 'html', escaped, with tabs expanded and spaces kept.
void appendText(QString &html, const QString &text, int &column) {
  for (QChar ch : text) {
    if (ch == '\t') {
      int spaces = kTabWidth - (column % kTabWidth);
      for (int i = 0; i < spaces; ++i)
        html += "&nbsp;";
      column += spaces;
      continue;
    }

    switch (ch.unicode()) {
      case ' ':
        html += "&nbsp;";
        break;
      case '<':
        html += "&lt;";
        break;
      case '>':
        html += "&gt;";
        break;
      case '&':
        html += "&amp;";
        break;
      default:
        html += ch;
        break;
    }

    ++column;
  }
}

QByteArray chomp(const QByteArray &line) {
  QByteArray result = line;
  while (result.endsWith('\n') || result.endsWith('\r'))
    result.chop(1);
  return result;
}

} // namespace

DiffModel::DiffModel(RepoView *view, QObject *parent)
    : QAbstractListModel(parent), mView(view) {
  git::RepositoryNotifier *notifier = view->repo().notifier();
  connect(notifier, &git::RepositoryNotifier::indexChanged, this,
          &DiffModel::updateIndex);
}

void DiffModel::setDiff(const git::Diff &diff, const QString &path) {
  // Keep the forced load of a large diff for the same file.
  if (path != mPath)
    mLoadAnyway = false;

  mDiff = diff;
  mPath = diff.isValid() && diff.indexOf(path) >= 0 ? path : QString();
  load();
}

QString DiffModel::oldPath() const {
  return mPatch.isValid() ? mPatch.name(git::Diff::OldFile) : QString();
}

QString DiffModel::status() const {
  if (!mPatch.isValid())
    return QString();

  if (mPatch.isConflicted())
    return "!";

  return QString(git::Diff::statusChar(mPatch.status()));
}

bool DiffModel::isEditable() const {
  return mDiff.isValid() && mDiff.isStatusDiff() && mPatch.isValid();
}

bool DiffModel::isConflicted() const {
  return mPatch.isValid() && mPatch.isConflicted();
}

int DiffModel::stageState() const {
  if (!isEditable())
    return git::Index::Disabled;

  return mDiff.index().isStaged(mPath);
}

int DiffModel::row(int hunk, int line) const {
  for (int i = 0; i < mRows.size(); ++i) {
    if (mRows.at(i).hunk == hunk && mRows.at(i).line == line)
      return i;
  }

  return -1;
}

void DiffModel::loadAnyway() {
  mLoadAnyway = true;
  load();
}

void DiffModel::load() {
  beginResetModel();

  mPatch = git::Patch();
  mStaged = git::Patch();
  mHunks.clear();
  mRows.clear();
  mStyles.clear();
  mNotice.clear();
  mCanLoadAnyway = false;
  mAdditions = 0;
  mDeletions = 0;
  mLineNumberWidth = 2;
  mMaxLineLength = 0;

  if (!mPath.isEmpty()) {
    mPatch = mDiff.patch(mDiff.indexOf(mPath));
    git::Patch::LineStats stats = mPatch.lineStats();
    mAdditions = stats.additions;
    mDeletions = stats.deletions;

    if (mPatch.isBinary()) {
      mNotice = tr("Binary file");
    } else if (mPatch.isLfsPointer()) {
      mNotice = tr("Git LFS object");
    } else if (!mLoadAnyway && stats.additions + stats.deletions > kMaxLines) {
      mNotice = tr("This diff has %1 changed lines and wasn't loaded.")
                    .arg(stats.additions + stats.deletions);
      mCanLoadAnyway = true;
    } else {
      loadStaged();

      int maxLine = 0;
      for (int h = 0; h < mPatch.count(); ++h) {
        QList<DiffLines::Line> lines = DiffLines::lines(mPatch, h, mStaged);
        mRows.append({HunkRow, h, -1});
        for (int l = 0; l < lines.size(); ++l) {
          mRows.append({LineRow, h, l});
          maxLine = qMax(maxLine, qMax(lines.at(l).oldLine, lines.at(l).newLine));

          // Approximate the width, tabs are expanded when drawn.
          const QByteArray &content = lines.at(l).content;
          int length = content.size() + content.count('\t') * (kTabWidth - 1);
          mMaxLineLength = qMax(mMaxLineLength, length);
        }

        mHunks.append(lines);
      }

      mLineNumberWidth = qMax(2, static_cast<int>(QString::number(maxLine).size()));
      highlight();

      if (mHunks.isEmpty())
        mNotice = mPatch.status() == GIT_DELTA_UNTRACKED && QFileInfo(
                      mView->repo().workdir().filePath(mPath)).isDir()
                      ? tr("Untracked directory")
                      : tr("No changes to show");
    }
  }

  endResetModel();
  emit diffChanged();
  emit stageStateChanged();
}

void DiffModel::loadStaged() {
  mStaged = git::Patch();
  if (!mDiff.isStatusDiff())
    return;

  // Generate a diff between the head tree and the index.
  git::Repository repo = mView->repo();
  git::Reference head = repo.head();
  git::Commit commit = head.isValid() ? head.target() : git::Commit();
  if (!commit.isValid())
    return;

  git::Diff staged = repo.diffTreeToIndex(commit.tree());
  int index = staged.indexOf(mPath);
  if (index >= 0)
    mStaged = staged.patch(index);
}

void DiffModel::updateIndex(const QStringList &paths) {
  if (mPath.isEmpty() || !mDiff.isStatusDiff() ||
      (!paths.isEmpty() && !paths.contains(mPath)))
    return;

  // Only the staged state of the lines changed.
  loadStaged();
  for (int h = 0; h < mHunks.size(); ++h) {
    QList<DiffLines::Line> lines = DiffLines::lines(mPatch, h, mStaged);
    for (int l = 0; l < lines.size() && l < mHunks[h].size(); ++l)
      mHunks[h][l].staged = lines.at(l).staged;
  }

  if (!mRows.isEmpty())
    emit dataChanged(index(0), index(mRows.size() - 1),
                     {StagedRole, HunkStateRole});
  emit stageStateChanged();
}

int DiffModel::hunkState(int hunk) const {
  int staged = 0;
  int changes = 0;
  for (const DiffLines::Line &line : mHunks.at(hunk)) {
    if (!line.isChange())
      continue;

    ++changes;
    if (line.staged)
      ++staged;
  }

  if (!staged)
    return git::Index::Unstaged;
  return staged == changes ? git::Index::Staged : git::Index::PartiallyStaged;
}

void DiffModel::highlight() {
  if (mHunks.isEmpty())
    return;

  // Style all lines of the file at once, in the order they appear.
  QByteArray text;
  QList<QList<QPair<int, int>>> spans;
  for (const QList<DiffLines::Line> &lines : mHunks) {
    QList<QPair<int, int>> lineSpans;
    for (const DiffLines::Line &line : lines) {
      QByteArray content = chomp(line.content);
      lineSpans.append({static_cast<int>(text.size()),
                        static_cast<int>(content.size())});
      text += content;
      text += '\n';
    }
    spans.append(lineSpans);
  }

  if (!mHighlighter)
    mHighlighter.reset(new SyntaxHighlighter);
  QByteArray styles = mHighlighter->style(mPath, text);

  for (const QList<QPair<int, int>> &lineSpans : spans) {
    QList<QByteArray> hunkStyles;
    for (const auto &span : lineSpans)
      hunkStyles.append(styles.mid(span.first, span.second));
    mStyles.append(hunkStyles);
  }
}

QString DiffModel::html(int hunk, int line) const {
  const QList<DiffLines::Line> &lines = mHunks.at(hunk);
  const DiffLines::Line &current = lines.at(line);
  git::Repository repo = mView->repo();
  QByteArray content = chomp(current.content);

  // Highlight the changed words of modified lines.
  DiffLines::Ranges ranges;
  if (current.isChange() && current.matchingLine >= 0) {
    const DiffLines::Line &match = lines.at(current.matchingLine);
    bool deletion = (current.origin == '-');
    auto changes = deletion ? DiffLines::changedRanges(current.content,
                                                       match.content)
                            : DiffLines::changedRanges(match.content,
                                                       current.content);
    ranges = deletion ? changes.first : changes.second;
  }

  Theme *theme = Application::theme();
  QString wordColor =
      theme->diff(current.origin == '-' ? Theme::Diff::WordDeletion
                                        : Theme::Diff::WordAddition)
          .name();

  // Whether each byte is in a changed word.
  QVector<bool> changed(content.size(), false);
  for (const auto &range : ranges) {
    int end = qMin(range.first + range.second, static_cast<int>(content.size()));
    for (int i = qMax(0, range.first); i < end; ++i)
      changed[i] = true;
  }

  QByteArray styles;
  if (mHighlighter && hunk < mStyles.size() && line < mStyles.at(hunk).size())
    styles = mStyles.at(hunk).at(line);
  auto styleAt = [&styles](int pos) {
    return pos < styles.size() ? static_cast<uchar>(styles.at(pos)) : 0;
  };

  // Emit runs of bytes with the same style and word change state.
  QString html;
  int column = 0;
  int pos = 0;
  while (pos < content.size()) {
    bool word = changed.at(pos);
    int style = styleAt(pos);
    int end = pos + 1;
    while (end < content.size() && changed.at(end) == word &&
           styleAt(end) == style)
      ++end;

    QString css;
    if (word)
      css += QString("background-color:%1;").arg(wordColor);
    if (style) {
      SyntaxHighlighter::Format format = mHighlighter->format(style);
      if (format.color.isValid())
        css += QString("color:%1;").arg(format.color.name());
      if (format.bold)
        css += "font-weight:bold;";
      if (format.italic)
        css += "font-style:italic;";
    }

    if (!css.isEmpty())
      html += QString("<span style='%1'>").arg(css);
    appendText(html, repo.decode(content.mid(pos, end - pos)), column);
    if (!css.isEmpty())
      html += "</span>";

    pos = end;
  }

  return html;
}

void DiffModel::toggleLine(int row) {
  if (row < 0 || row >= mRows.size() || mRows.at(row).kind != LineRow)
    return;

  const Row &r = mRows.at(row);
  DiffLines::Line &line = mHunks[r.hunk][r.line];
  if (!isEditable() || isConflicted() || !line.isChange())
    return;

  line.staged = !line.staged;
  stage(r.hunk);
}

void DiffModel::setLinesStaged(int first, int last, bool staged) {
  if (!isEditable() || isConflicted())
    return;

  QSet<int> hunks;
  for (int row = qMax(0, first); row <= last && row < mRows.size(); ++row) {
    const Row &r = mRows.at(row);
    if (r.kind != LineRow)
      continue;

    DiffLines::Line &line = mHunks[r.hunk][r.line];
    if (line.isChange()) {
      line.staged = staged;
      hunks.insert(r.hunk);
    }
  }

  if (!hunks.isEmpty())
    stage(*hunks.begin());
}

void DiffModel::setHunkStaged(int hunk, bool staged) {
  if (!isEditable() || isConflicted() || hunk < 0 || hunk >= mHunks.size())
    return;

  for (DiffLines::Line &line : mHunks[hunk]) {
    if (line.isChange())
      line.staged = staged;
  }

  stage(hunk);
}

void DiffModel::setFileStaged(bool staged) {
  if (isEditable())
    mDiff.index().setStaged({mPath}, staged);
}

void DiffModel::stage(int changedHunk) {
  Q_UNUSED(changedHunk)

  git::Index index = mDiff.index();
  if (!index.isValid())
    return;

  int staged = 0;
  int unstaged = 0;
  for (int h = 0; h < mHunks.size(); ++h) {
    int state = hunkState(h);
    if (state == git::Index::Staged)
      ++staged;
    else if (state == git::Index::Unstaged)
      ++unstaged;
  }

  if (staged == mHunks.size() && !mHunks.isEmpty()) {
    index.setStaged({mPath}, true);
    return;
  }

  if (unstaged == mHunks.size()) {
    index.setStaged({mPath}, false);
    return;
  }

  // Build the content of the index from the staged lines. When a line is
  // changed, the old and the new version are in the diff. Staging only the
  // new one keeps the old one in the file.
  git::Repository repo = mView->repo();
  git::Blob blob = repo.lookupBlob(repo.workdirId(mPath));

  QList<QList<QByteArray>> image;
  git::Patch::populatePreimage(image, blob.content());
  for (int h = 0; h < mHunks.size(); ++h) {
    QByteArray content = DiffLines::stagedContent(mHunks.at(h));
    mPatch.apply(image, h, content);
  }

  index.add(mPath, mPatch.generateResult(image));
}

void DiffModel::discardHunk(int hunk) {
  if (!isEditable() || hunk < 0 || hunk >= mHunks.size())
    return;

  QString text =
      mPatch.isUntracked()
          ? tr("Are you sure you want to remove '%1'?").arg(mPath)
          : tr("Are you sure you want to discard this hunk of '%1'?").arg(mPath);
  ConfirmDialog *dialog = new ConfirmDialog(mView);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setTitle(tr("Discard Hunk?"));
  dialog->setText(text);
  dialog->setInformativeText(tr("This action cannot be undone."));
  dialog->setAcceptText(tr("Discard Hunk"));
  dialog->setDanger(true);

  QList<bool> lines(mHunks.at(hunk).size(), true);
  connect(dialog, &QDialog::accepted, this,
          [this, hunk, lines] { this->discard(hunk, lines); });

  dialog->open();
}

void DiffModel::discardLines(int first, int last) {
  if (!isEditable() || first < 0 || first >= mRows.size())
    return;

  int hunk = mRows.at(first).hunk;
  QList<bool> lines(mHunks.at(hunk).size(), false);
  for (int row = first; row <= last && row < mRows.size(); ++row) {
    if (mRows.at(row).hunk == hunk && mRows.at(row).kind == LineRow)
      lines[mRows.at(row).line] = true;
  }

  ConfirmDialog *dialog = new ConfirmDialog(mView);
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  dialog->setTitle(tr("Discard Lines?"));
  dialog->setText(
      tr("Are you sure you want to discard the selected lines of '%1'?")
          .arg(mPath));
  dialog->setInformativeText(tr("This action cannot be undone."));
  dialog->setAcceptText(tr("Discard Lines"));
  dialog->setDanger(true);

  connect(dialog, &QDialog::accepted, this,
          [this, hunk, lines] { this->discard(hunk, lines); });

  dialog->open();
}

void DiffModel::discard(int hunk, const QList<bool> &lines) {
  git::Repository repo = mView->repo();
  if (mPatch.isUntracked()) {
    repo.workdir().remove(mPath);
    mView->refresh();
    return;
  }

  git::Blob blob = repo.lookupBlob(repo.workdirId(mPath));
  QByteArray content = DiffLines::discardContent(mHunks.at(hunk), lines);
  QByteArray buffer = mPatch.apply(hunk, content, blob.content());

  QSaveFile file(repo.workdir().filePath(mPath));
  if (!file.open(QFile::WriteOnly))
    return;

  file.write(buffer);
  if (!file.commit())
    return;

  mView->refresh();
}

void DiffModel::editHunk(int hunk) {
  if (hunk < 0 || hunk >= mHunks.size())
    return;

  int line = 1;
  for (const DiffLines::Line &l : mHunks.at(hunk)) {
    if (l.newLine > 0) {
      line = l.newLine;
      break;
    }
  }

  mView->edit(mPath, line);
}

void DiffModel::chooseConflict(int hunk, int resolution) {
  if (!isConflicted() || hunk < 0 || hunk >= mHunks.size())
    return;

  mPatch.setConflictResolution(
      hunk, static_cast<git::Patch::ConflictResolution>(resolution));

  for (int row = 0; row < mRows.size(); ++row) {
    if (mRows.at(row).hunk == hunk)
      emit dataChanged(index(row), index(row), {ResolutionRole, ChosenRole});
  }
}

void DiffModel::saveConflict(int hunk) {
  if (!isConflicted() || hunk < 0 || hunk >= mHunks.size())
    return;

  git::Patch::ConflictResolution resolution = mPatch.conflictResolution(hunk);
  if (resolution == git::Patch::Unresolved)
    return;

  git::Repository repo = mView->repo();
  QString path = repo.workdir().filePath(mPath);

  QStringList lines;
  {
    QFile file(path);
    if (!file.open(QFile::ReadOnly))
      return;

    // Keep the line endings.
    QString text = repo.decode(file.readAll());
    int pos = 0;
    while (pos < text.length()) {
      int end = text.indexOf('\n', pos);
      end = (end < 0) ? text.length() : end + 1;
      lines.append(text.mid(pos, end - pos));
      pos = end;
    }
  }

  // Remove the lines of the other side and the conflict markers.
  for (int i = mPatch.lineCount(hunk) - 1; i >= 0; --i) {
    char origin = mPatch.lineOrigin(hunk, i);
    if (origin == GIT_DIFF_LINE_CONTEXT ||
        (origin == 'O' && resolution == git::Patch::Ours) ||
        (origin == 'T' && resolution == git::Patch::Theirs))
      continue;

    int line = mPatch.lineNumber(hunk, i);
    if (line >= 0 && line < lines.size())
      lines.removeAt(line);
  }

  QSaveFile file(path);
  if (!file.open(QFile::WriteOnly))
    return;

  QTextStream out(&file);
  out.setEncoding(repo.encoding());
  out << lines.join(QString());
  out.flush();
  if (!file.commit())
    return;

  mPatch.setConflictResolution(hunk, git::Patch::Unresolved);
  mView->refresh();
}

int DiffModel::rowCount(const QModelIndex &parent) const {
  return parent.isValid() ? 0 : mRows.size();
}

QVariant DiffModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= mRows.size())
    return QVariant();

  const Row &row = mRows.at(index.row());
  switch (role) {
    case KindRole:
      return row.kind;
    case HunkRole:
      return row.hunk;
    case HeaderRole:
      return row.kind == HunkRow ? QString::fromUtf8(chomp(mPatch.header(row.hunk)))
                                 : QString();
    case HunkStateRole:
      return hunkState(row.hunk);
    case ResolutionRole:
      return isConflicted()
                 ? static_cast<int>(
                       const_cast<git::Patch &>(mPatch).conflictResolution(
                           row.hunk))
                 : 0;
  }

  if (row.kind != LineRow) {
    switch (role) {
      case OriginRole:
        return QString();
      case OldLineRole:
      case NewLineRole:
        return -1;
      case HtmlRole:
        return QString();
      case StagedRole:
      case StageableRole:
        return false;
      case ChosenRole:
        return true;
    }

    return QVariant();
  }

  const DiffLines::Line &line = mHunks.at(row.hunk).at(row.line);
  switch (role) {
    case OriginRole:
      return QString(QChar(line.origin));
    case OldLineRole:
      return line.oldLine;
    case NewLineRole:
      return line.newLine;
    case HtmlRole:
      return html(row.hunk, row.line);
    case StagedRole:
      return line.staged;
    case StageableRole:
      return isEditable() && !isConflicted() && line.isChange();
    case ChosenRole: {
      if (!isConflicted())
        return true;

      auto resolution =
          const_cast<git::Patch &>(mPatch).conflictResolution(row.hunk);
      if (resolution == git::Patch::Ours)
        return line.origin != 'T';
      if (resolution == git::Patch::Theirs)
        return line.origin != 'O';
      return true;
    }
  }

  return QVariant();
}

QHash<int, QByteArray> DiffModel::roleNames() const {
  return {{KindRole, "kind"},         {HunkRole, "hunk"},
          {OriginRole, "origin"},     {OldLineRole, "oldLine"},
          {NewLineRole, "newLine"},   {HtmlRole, "html"},
          {StagedRole, "staged"},     {StageableRole, "stageable"},
          {HeaderRole, "header"},     {HunkStateRole, "hunkState"},
          {ResolutionRole, "resolution"}, {ChosenRole, "chosen"}};
}
