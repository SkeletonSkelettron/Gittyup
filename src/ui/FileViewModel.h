//
//          Copyright (c) 2026, Gittyup contributors
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//

#ifndef FILEVIEWMODEL_H
#define FILEVIEWMODEL_H

#include "FindController.h"
#include "git/Blame.h"
#include "git/Commit.h"
#include <QAbstractListModel>
#include <QDateTime>
#include <QFutureWatcher>
#include <QScopedPointer>

class RepoView;
class SyntaxHighlighter;

// The lines of a file at a commit or in the working copy, highlighted and
// annotated with the commit that last changed them. qrc:/qml/FileView.qml
// draws it as 'detailView.file'.
class FileViewModel : public QAbstractListModel, public FindTarget {
  Q_OBJECT

  Q_PROPERTY(QString path READ path NOTIFY fileChanged)
  Q_PROPERTY(QString revision READ revision NOTIFY fileChanged)
  Q_PROPERTY(QString notice READ notice NOTIFY fileChanged)
  Q_PROPERTY(int lineNumberWidth READ lineNumberWidth NOTIFY fileChanged)
  Q_PROPERTY(int maxLineLength READ maxLineLength NOTIFY fileChanged)
  Q_PROPERTY(bool blameLoading READ isBlameLoading NOTIFY blameChanged)
  Q_PROPERTY(bool hasBlame READ hasBlame NOTIFY blameChanged)
  Q_PROPERTY(QString selectedCommit READ selectedCommit WRITE
                 setSelectedCommit NOTIFY selectedCommitChanged)

public:
  enum Role {
    NumberRole = Qt::UserRole + 1,
    HtmlRole,
    // The commit that last changed the line, empty for uncommitted lines.
    BlameIdRole,
    // The first and the last line of the lines of the same commit.
    BlameFirstRole,
    BlameLastRole,
    // The position of the line in the lines of the same commit.
    BlameOffsetRole,
    BlameCommittedRole,
    BlameSummaryRole,
    BlameAuthorRole,
    BlameDateRole,
    // The age of the commit in the heat map, empty if it's disabled.
    BlameColorRole,
    BlameTipRole,
    // The matches of the find bar in the line.
    MatchesRole
  };

  FileViewModel(RepoView *view, QObject *parent = nullptr);
  ~FileViewModel() override;

  // Load 'path' at 'commit', or from the working copy if it's invalid.
  void load(const QString &path, const git::Commit &commit);
  void clear();

  QString path() const { return mPath; }
  QString revision() const;
  QString notice() const { return mNotice; }
  int lineNumberWidth() const;
  int maxLineLength() const { return mMaxLineLength; }
  bool isBlameLoading() const { return mBlameLoading; }
  bool hasBlame() const { return mBlame.isValid(); }

  QString selectedCommit() const { return mSelectedCommit; }
  void setSelectedCommit(const QString &id);

  // Show the commit in the history.
  Q_INVOKABLE void showCommit(const QString &id);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QHash<int, QByteArray> roleNames() const override;

  int findRowCount() const override { return mLines.size(); }
  QString findRowText(int row) const override;
  void setFindState(const QString &text, int row, int start) override;

signals:
  void fileChanged();
  void blameChanged();
  void selectedCommitChanged();

private:
  // A range of lines of the same commit.
  struct Block {
    QString id;
    bool committed = false;
    QString summary;
    QString author;
    QString tip;
    QDateTime date;
    QString color;
  };

  void cancelBlame();
  void setBlame(const git::Blame &blame);

  RepoView *mView;
  QScopedPointer<SyntaxHighlighter> mHighlighter;

  QString mPath;
  git::Commit mCommit;
  QString mNotice;
  int mMaxLineLength = 0;

  QList<QByteArray> mLines;
  QList<QByteArray> mStyles;

  // The block of each line.
  git::Blame mBlame;
  QList<Block> mBlocks;
  QList<int> mLineBlocks;
  QList<int> mLineOffsets;
  QString mSelectedCommit;

  QString mFindText;
  int mFindRow = -1;
  int mFindStart = -1;

  QByteArray mContent;
  bool mBlameLoading = false;
  QFutureWatcher<git::Blame> mBlameWatcher;
  QScopedPointer<class BlameCanceler> mCanceler;
};

#endif
