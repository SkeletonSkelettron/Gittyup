//
//          Copyright (c) 2016, Scientific Toolworks, Inc.
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//
// Author: Jason Haslam
//

#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QWidget>

class TextEditor;
class QQuickWidget;

class EditorProvider {
public:
  virtual QList<TextEditor *> editors() = 0;
  virtual void ensureVisible(TextEditor *editor, int pos) = 0;
};

// The bar to find text in editors. qrc:/qml/FindBar.qml draws it.
class FindWidget : public QWidget {
  Q_OBJECT

  Q_PROPERTY(QString searchText READ text NOTIFY searchTextChanged)
  Q_PROPERTY(QString hitsText READ hitsText NOTIFY hitsChanged)
  Q_PROPERTY(bool hasMatches READ hasMatches NOTIFY hitsChanged)

public:
  // The difference between Forward and Advance is that Forward doesn't
  // advance if the current selection already matches the search term.
  enum Direction { Backward, Forward, Advance };

  FindWidget(EditorProvider *provider, QWidget *parent = nullptr);
  ~FindWidget() override;

  void reset();

  static QString text() { return sText; }
  static void setText(const QString &text) { sText = text; }

  QString hitsText() const { return mHits; }
  bool hasMatches() const { return mMatches > 0; }

  void clearHighlights();
  void highlightAll();
  void find(Direction direction = Advance);

  void showAndSetFocus();

  Q_INVOKABLE void search(const QString &text);
  Q_INVOKABLE void next() { find(); }
  Q_INVOKABLE void previous() { find(Backward); }

signals:
  void searchTextChanged();
  void hitsChanged();
  // Focus and select the search field.
  void focusRequested();

protected:
  void hideEvent(QHideEvent *event) override;
  void showEvent(QShowEvent *event) override;

private:
  int mEditorIndex = 0;
  EditorProvider *mEditorProvider;

  QQuickWidget *mView;
  QString mHits;
  int mMatches = 0;

  static QString sText;
};

#endif
