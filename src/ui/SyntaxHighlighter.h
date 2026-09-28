//
//          Copyright (c) 2026, Gittyup contributors
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//

#ifndef SYNTAXHIGHLIGHTER_H
#define SYNTAXHIGHLIGHTER_H

#include <QByteArray>
#include <QColor>
#include <QHash>
#include <QString>

class TextEditor;

// Styles source code with the lexers and the theme of the text editor, so
// views that aren't text editors can highlight code like it.
class SyntaxHighlighter {
public:
  struct Format {
    // Invalid for the default text color.
    QColor color;
    bool bold = false;
    bool italic = false;
  };

  SyntaxHighlighter();
  ~SyntaxHighlighter();

  // Get the style of each byte of 'text', which is the content of 'path'.
  QByteArray style(const QString &path, const QByteArray &text);

  // The format of a style returned by the last call to style().
  Format format(int style) const;

  // The hidden editor, for example to run plugins on text.
  TextEditor *editor() const { return mEditor; }

private:
  TextEditor *mEditor;
  mutable QHash<int, Format> mFormats;
};

#endif
