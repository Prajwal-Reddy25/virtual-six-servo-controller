# Publication preparation

No GitHub remote was configured when inspected. No external push or GitHub
release has been performed. Publication needs explicit author approval. Source
version is 1.0.0; release notes are a local draft.

After approval, create an **empty** GitHub repository named
`virtual-six-servo-controller` in the intended account. Do not initialize it with
a README, license, or .gitignore, because local history already exists. Use the
web interface or an authenticated GitHub CLI with the intended account selected.

Replace `<account>` and set the remote:

```sh
git remote add origin https://github.com/<account>/virtual-six-servo-controller.git
git ls-remote origin
```

The local branch is `master`. For the expected empty remote, publication commands
are:

```sh
git push -u origin master
git tag -a v1.0.0 -m "Virtual Six-Servo Controller v1.0.0"
git push origin v1.0.0
gh release create v1.0.0 --verify-tag \
  --title "Virtual Six-Servo Controller v1.0.0" \
  --notes-file docs/RELEASE_v1.0.0.md
```

These are prepared instructions, not executed actions. If the remote contains
history or the name is unavailable, inspect it before proceeding; never overwrite
or force-push. Use normal GitHub authentication; do not put credentials in files
or remote URLs. No reuse license has been selected; publication does not itself
select one.
