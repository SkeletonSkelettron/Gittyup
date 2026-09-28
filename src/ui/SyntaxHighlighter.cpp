//
//          Copyright (c) 2026, Gittyup contributors
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//

#include "SyntaxHighlighter.h"
#include "editor/TextEditor.h"

namespace {

QColor fromScintilla(sptr_t colour) {
  return QColor(colour & 0xff, (colour >> 8) & 0xff, (colour >> 16) & 0xff);
}

} // namespace

// The editor is never shown. It only runs the lexer.
SyntaxHighlighter::SyntaxHighlighter() : mEditor(new TextEditor) {}

SyntaxHighlighter::~SyntaxHighlighter() { delete mEditor; }

QByteArray SyntaxHighlighter::style(const QString &path,
                                    const QByteArray &text) {
  mFormats.clear();
  mEditor->setLexer(path);
  mEditor->setText(text.constData());
  mEditor->colourise(0, -1);

  int length = qMin(static_cast<int>(mEditor->length()),
                    static_cast<int>(text.size()));
  QByteArray styles(text.size(), 0);
  for (int i = 0; i < length; ++i)
    styles[i] = static_cast<char>(mEditor->styleAt(i));

  // Free the memory of the text.
  mEditor->setText("");
  return styles;
}

SyntaxHighlighter::Format SyntaxHighlighter::format(int style) const {
  auto it = mFormats.constFind(style);
  if (it != mFormats.constEnd())
    return it.value();

  Format format;
  if (style != 0) {
    sptr_t fore = mEditor->styleFore(style);
    if (fore != mEditor->styleFore(STYLE_DEFAULT))
      format.color = fromScintilla(fore);
    format.bold = mEditor->styleBold(style);
    format.italic = mEditor->styleItalic(style);
  }

  mFormats.insert(style, format);
  return format;
}
