# gh

gh is a minimal alternative to GitHub CLI. It allows you to rapidly open the repository page, list PRs waiting for review, open new PRs, etc.

## Summary
```bash
# Opens the repository page
$ gh

# Opens the pull request page of the current repository
$ gh pulls

# Opens the pull request page, but only your closed PRs are listed
$ gh pulls --closed \
           --author=@me

# Opens the pull request page, but only PRs awaiting review from you are listed
$ gh pulls --to-review

# Opens the pull request form filling some fields
$ gh newpr --dest-src=main..my-branch \
           --template=feature.md \
           --title=I implemented a very nice feature \
           --assignees=gustavothecoder \
           --labels=feature,minor
```

## Installation

### Dependencies

You need to be able to compile C code. Installing `build-essential` for Debian-based distros or `base-devel` for Arch-based distros should be sufficient.

Right now FireFox is a dependency too.

### Steps

Just run:
```bash
$ make install
```

To uninstall:
```bash
$ make uninstall
```

## Usage

Run `gh help` or read the manpage [right here](./docs/text_man).

## Contributing

Bug reports and pull requests are welcome :)
