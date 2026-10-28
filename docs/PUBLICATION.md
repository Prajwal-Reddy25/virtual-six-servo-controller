# v1.0.0 publication record

The project author explicitly approved public publication to:

- Repository: https://github.com/Prajwal-Reddy25/virtual-six-servo-controller
- Release: https://github.com/Prajwal-Reddy25/virtual-six-servo-controller/releases/tag/v1.0.0
- Branch: `master`
- Annotated tag: `v1.0.0`

The repository was created empty, without a generated README, license, or
.gitignore. Authentication matched `Prajwal-Reddy25`; inspection found no
existing remote refs before publication. No remote history is overwritten.

The original verified source commit is
`b8e01ccf686b1cc371cc347cb69215a2bf9d6190`, preserved unchanged in history.
The publication commit updates documentation only. The author explicitly chose
to retain the existing Git author/committer email after being informed that it
would become public. No credentials, local filesystem paths, or generated build
artifacts were found in the tracked file review. The existing author identity
is intentional; Codex implementation attribution remains explicit.

Publication uses normal pushes and the prepared notes:

```sh
git remote add origin https://github.com/Prajwal-Reddy25/virtual-six-servo-controller.git
git ls-remote origin
git push -u origin master
git tag -a v1.0.0 -m "Virtual Six-Servo Controller v1.0.0"
git push origin refs/tags/v1.0.0
gh release create v1.0.0 --verify-tag \
  --repo Prajwal-Reddy25/virtual-six-servo-controller \
  --title "Virtual Six-Servo Controller v1.0.0" \
  --notes-file docs/RELEASE_v1.0.0.md
```

Verification checks the public repository metadata, matching master/tag commit,
annotated tag object, non-draft release, README, documentation, and clean local
working tree. Public access is checked without an authorization header, in
addition to authenticated GitHub inspection. The release commit can be read
with `git rev-parse 'v1.0.0^{}'`.

No force-push, history replacement, extra functionality, or v1.1.0 development
is part of publication. No reuse license has been selected. This software-only
project makes no physical performance or real-time claims.
