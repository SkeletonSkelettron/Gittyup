//
//          Copyright (c) 2026, Gittyup contributors
//
// This software is licensed under the MIT License. The LICENSE.md file
// describes the conditions under which this software may be distributed.
//

#include "ReferenceItems.h"
#include "git/Branch.h"
#include "git/Commit.h"
#include "git/Repository.h"
#include "git/TagRef.h"

ReferenceItems::ReferenceItems(const git::Repository &repo, int kinds,
                               const QString &none) {
  if (!none.isEmpty()) {
    mRefs.append(git::Reference());
    mItems.append(QVariantMap{{"text", none}});
  }

  if (kinds & DetachedHead) {
    git::Reference head = repo.head();
    if (head.isValid() && head.isDetachedHead())
      add(head, "commit");
  }

  if (kinds & LocalBranches) {
    for (const git::Branch &branch : repo.branches(GIT_BRANCH_LOCAL))
      add(branch, "branch");
  }

  if (kinds & RemoteBranches) {
    for (const git::Branch &branch : repo.branches(GIT_BRANCH_REMOTE)) {
      // Skip symbolic references like origin/HEAD.
      if (!branch.name().endsWith("/HEAD"))
        add(branch, "cloud");
    }
  }

  if (kinds & Tags) {
    for (const git::TagRef &tag : repo.tags())
      add(tag, "tag");
  }
}

git::Reference ReferenceItems::reference(int index) const {
  return (index >= 0 && index < mRefs.size()) ? mRefs.at(index)
                                              : git::Reference();
}

int ReferenceItems::indexOf(const git::Reference &ref) const {
  if (!ref.isValid())
    return -1;

  QString name = ref.qualifiedName();
  for (int i = 0; i < mRefs.size(); ++i) {
    if (mRefs.at(i).isValid() && mRefs.at(i).qualifiedName() == name)
      return i;
  }

  return -1;
}

void ReferenceItems::add(const git::Reference &ref, const QString &icon) {
  git::Commit commit = ref.target();
  mRefs.append(ref);
  mItems.append(QVariantMap{
      {"text", ref.name()},
      {"icon", icon},
      {"detail", commit.isValid() ? commit.shortId() : QString()}});
}
