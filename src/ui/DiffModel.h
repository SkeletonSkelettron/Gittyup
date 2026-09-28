//
//          Copyright (c) 2026, Gittyup contributors
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//

#ifndef DIFFMODEL_H
#define DIFFMODEL_H

#include "DiffLines.h"
#include "git/Diff.h"
#include "git/Patch.h"
#include <QAbstractListModel>

class RepoView;

// The diff of a single file as rows of hunk headers and lines. For the
// uncommitted changes, single lines, hunks and the whole file can be staged,
// unstaged and discarded. Used by qrc:/qml/DiffPanel.qml.
class DiffModel : public QAbstractListModel {
  Q_OBJECT

  Q_PROPERTY(QString path READ path NOTIFY diffChanged)
  Q_PROPERTY(QString oldPath READ oldPath NOTIFY diffChanged)
  Q_PROPERTY(QString status READ status NOTIFY diffChanged)
  Q_PROPERTY(bool empty READ isEmpty NOTIFY diffChanged)
  Q_PROPERTY(bool editable READ isEditable NOTIFY diffChanged)
  Q_PROPERTY(bool conflicted READ isConflicted NOTIFY diffChanged)
  Q_PROPERTY(QString notice READ notice NOTIFY diffChanged)
  Q_PROPERTY(bool canLoadAnyway READ canLoadAnyway NOTIFY diffChanged)
  Q_PROPERTY(int additions READ additions NOTIFY diffChanged)
  Q_PROPERTY(int deletions READ deletions NOTIFY diffChanged)
  Q_PROPERTY(int stageState READ stageState NOTIFY stageStateChanged)
  Q_PROPERTY(int lineNumberWidth READ lineNumberWidth NOTIFY diffChanged)
  Q_PROPERTY(int maxLineLength READ maxLineLength NOTIFY diffChanged)

public:
  enum Kind { HunkRow, LineRow };

  enum Role {
    KindRole = Qt::UserRole,
    HunkRole,
    OriginRole,
    OldLineRole,
    NewLineRole,
    HtmlRole,
    StagedRole,
    StageableRole,
    HeaderRole,
    HunkStateRole,
    ResolutionRole,
    ChosenRole
  };

  DiffModel(RepoView *view, QObject *parent = nullptr);

  // Show the file 'path' of 'diff', or nothing.
  void setDiff(const git::Diff &diff, const QString &path);
  git::Diff diff() const { return mDiff; }

  QString path() const { return mPath; }
  QString oldPath() const;
  QString status() const;
  bool isEmpty() const { return mPath.isEmpty(); }
  bool isEditable() const;
  bool isConflicted() const;
  QString notice() const { return mNotice; }
  bool canLoadAnyway() const { return mCanLoadAnyway; }
  int additions() const { return mAdditions; }
  int deletions() const { return mDeletions; }
  int stageState() const;
  int lineNumberWidth() const { return mLineNumberWidth; }
  int maxLineLength() const { return mMaxLineLength; }

  // The lines of the hunks, and the row of a line in the model.
  int hunkCount() const { return mHunks.size(); }
  const QList<DiffLines::Line> &lines(int hunk) const { return mHunks.at(hunk); }
  int row(int hunk, int line) const;

  // Discard the changes of the lines for which 'lines' is true without
  // asking for confirmation.
  void discard(int hunk, const QList<bool> &lines);

  Q_INVOKABLE void loadAnyway();
  Q_INVOKABLE void toggleLine(int row);
  Q_INVOKABLE void setLinesStaged(int first, int last, bool staged);
  Q_INVOKABLE void setHunkStaged(int hunk, bool staged);
  Q_INVOKABLE void setFileStaged(bool staged);
  Q_INVOKABLE void discardHunk(int hunk);
  Q_INVOKABLE void discardLines(int first, int last);
  Q_INVOKABLE void editHunk(int hunk);
  Q_INVOKABLE void chooseConflict(int hunk, int resolution);
  Q_INVOKABLE void saveConflict(int hunk);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QHash<int, QByteArray> roleNames() const override;

signals:
  void diffChanged();
  void stageStateChanged();

private:
  struct Row {
    Kind kind;
    int hunk;
    int line;
  };

  void load();
  void loadStaged();
  void updateIndex(const QStringList &paths);
  QString html(int hunk, int line) const;
  int hunkState(int hunk) const;
  void stage(int changedHunk = -1);

  RepoView *mView;
  git::Diff mDiff;
  QString mPath;
  git::Patch mPatch;
  git::Patch mStaged;

  QList<QList<DiffLines::Line>> mHunks;
  QList<Row> mRows;

  QString mNotice;
  bool mCanLoadAnyway = false;
  bool mLoadAnyway = false;
  int mAdditions = 0;
  int mDeletions = 0;
  int mLineNumberWidth = 2;
  int mMaxLineLength = 0;
};

#endif
