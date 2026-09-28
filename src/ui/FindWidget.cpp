//
//          Copyright (c) 2016, Scientific Toolworks, Inc.
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//
// Author: Jason Haslam
//

#include "FindWidget.h"
#include "MenuBar.h"
#include "editor/TextEditor.h"
#include "qml/QmlSupport.h"
#include <QHideEvent>
#include <QQuickWidget>
#include <QShowEvent>
#include <QVBoxLayout>

namespace {

const int kHeight = 44;

} // namespace

QString FindWidget::sText;

FindWidget::FindWidget(EditorProvider *provider, QWidget *parent)
    : QWidget(parent), mEditorProvider(provider) {
  setFixedHeight(kHeight);

  mView = QmlSupport::createView(
      "FindBar", {{"findBar", QVariant::fromValue<QObject *>(this)}}, this);
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(mView);
}

FindWidget::~FindWidget() {
  // The QML view references this object, so it has to go first.
  delete mView;
}

void FindWidget::search(const QString &text) {
  if (text == sText)
    return;

  sText = text;
  emit searchTextChanged();
  highlightAll();

  if (MenuBar *menuBar = MenuBar::instance(this))
    menuBar->updateFind();
}

void FindWidget::reset() { mEditorIndex = 0; }

void FindWidget::clearHighlights() {
  for (TextEditor *editor : mEditorProvider->editors())
    editor->clearHighlights();
}

void FindWidget::highlightAll() {
  int matches = 0;
  for (TextEditor *editor : mEditorProvider->editors())
    matches += editor->highlightAll(sText);

  QString text;
  switch (matches) {
    case 0:
      text = tr("Not found");
      break;

    case 1:
      text = tr("%1 match").arg(matches);
      break;

    default:
      text = tr("%1 matches").arg(matches);
      break;
  }

  mHits = sText.isEmpty() ? QString() : text;
  mMatches = matches;
  emit hitsChanged();

  // Go to the first match.
  if (matches)
    find(Forward);
}

void FindWidget::find(Direction direction) {
  bool forward = (direction != Backward);

  // Search through all editors until a match is found.
  // Then search the initial editor again from the beginning.
  QList<TextEditor *> editors = mEditorProvider->editors();
  for (int i = 0; i < editors.size() + 1; ++i) {
    TextEditor *editor = editors.at(mEditorIndex);

    // Advance to end of selection.
    if (direction == Advance) {
      int sel = editor->selectionEnd();
      editor->setSelection(sel, sel);
    }

    // Search without wrapping.
    int pos = editor->find(sText, forward, isVisible());
    if (pos >= 0) {
      // Scroll the match into view.
      mEditorProvider->ensureVisible(editor, pos);
      return;
    }

    // Choose next index.
    if (forward) {
      ++mEditorIndex;
      if (mEditorIndex > editors.size() - 1)
        mEditorIndex = 0;
    } else {
      --mEditorIndex;
      if (mEditorIndex < 0)
        mEditorIndex = editors.size() - 1;
    }

    // Reset current editor selection.
    editor->setSelection(0, 0);

    // Reset next editor selection.
    TextEditor *next = editors.at(mEditorIndex);
    int extreme = forward ? 0 : next->length();
    next->setSelection(extreme, extreme);
  }
}

void FindWidget::showAndSetFocus() {
  show();
  emit searchTextChanged();
  mView->setFocus();
  emit focusRequested();
}

void FindWidget::hideEvent(QHideEvent *event) {
  QWidget::hideEvent(event);

  if (!event->spontaneous())
    clearHighlights();
}

void FindWidget::showEvent(QShowEvent *event) {
  if (!event->spontaneous() && !sText.isEmpty())
    highlightAll();

  QWidget::showEvent(event);
}
