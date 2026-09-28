//
//          Copyright (c) 2026, Gittyup contributors
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//

#include "ConfirmDialog.h"

ConfirmDialog::ConfirmDialog(QWidget *parent)
    : QmlDialog(parent), mAcceptText(tr("OK")) {
  setContent("ConfirmDialog");
}

void ConfirmDialog::setTitle(const QString &title) {
  mTitle = title;
  setWindowTitle(title);
  emit changed();
}

void ConfirmDialog::setText(const QString &text) {
  mText = text;
  emit changed();
}

void ConfirmDialog::setInformativeText(const QString &text) {
  mInformativeText = text;
  emit changed();
}

void ConfirmDialog::setDetailedText(const QString &text) {
  mDetailedText = text;
  emit changed();
}

void ConfirmDialog::setAcceptText(const QString &text) {
  mAcceptText = text;
  emit changed();
}

void ConfirmDialog::setCheckText(const QString &text) {
  mCheckText = text;
  emit changed();
}

void ConfirmDialog::setChecked(bool checked) {
  if (checked == mChecked)
    return;

  mChecked = checked;
  emit changed();
}

void ConfirmDialog::setDanger(bool danger) {
  mDanger = danger;
  emit changed();
}
