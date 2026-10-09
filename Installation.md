# MacoraOne Installation on Linux

This guide installs the MacoraOne Linux x86_64 release as a system-wide command so it can be run from any working directory.

## Requirements

- A 64-bit x86 Linux system
- `curl` or `wget`
- `unzip`
- `sudo` access to install into `/usr/local/bin`

On Debian or Ubuntu, install the required tools if needed:

```bash
sudo apt-get update
sudo apt-get install -y curl unzip
```

## Download and install

Run this complete command in a terminal:

```bash
set -eu

URL="https://github.com/MacoraOne/MacoraOne/releases/download/one-v1.0.0/macoraone-linux-x86_64.zip"
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

curl -fL "$URL" -o "$TMP_DIR/macoraone-linux-x86_64.zip"
unzip -q "$TMP_DIR/macoraone-linux-x86_64.zip" -d "$TMP_DIR/extracted"

# Locate the executable inside the archive.
BIN="$(find "$TMP_DIR/extracted" -type f \( -name 'macoraone' -o -name 'macoraone-linux-x86_64' \) -print -quit)"

if [ -z "$BIN" ]; then
    echo "Error: could not find the MacoraOne executable in the release archive." >&2
    echo "Archive contents:" >&2
    find "$TMP_DIR/extracted" -maxdepth 3 -type f -print >&2
    exit 1
fi

chmod +x "$BIN"
sudo install -m 0755 "$BIN" /usr/local/bin/macoraone

echo "MacoraOne installed at /usr/local/bin/macoraone"
macoraone --help
```

If the program does not support `--help`, the installation can still have succeeded; check the command's documented usage or run `macoraone` with the appropriate arguments.

## Verify installation

```bash
command -v macoraone
macoraone --help
```

The first command should print:

```text
/usr/local/bin/macoraone
```

Because `/usr/local/bin` is normally included in the system `PATH`, `macoraone` can be called from any directory. If the shell cannot find it, open a new terminal and check that `/usr/local/bin` is in `PATH`.

## Uninstall

```bash
sudo rm -f /usr/local/bin/macoraone
```

## Release

Download: [MacoraOne v1.0.0 Linux x86_64](https://github.com/MacoraOne/MacoraOne/releases/download/one-v1.0.0/macoraone-linux-x86_64.zip)

Project: [MacoraOne on GitHub](https://github.com/MacoraOne/MacoraOne)
