# MacoraOne Installation on Linux

This guide installs the MacoraOne Linux x86_64 release as the system-wide command `one`, so it can be run from any working directory.

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

## Quick installation using install.sh

You can install MacoraOne with this one-line command. The installer downloads the Linux x86_64 release, extracts the executable, and installs it system-wide as `/usr/local/bin/one`.

```bash
curl -fsSL https://github.com/MacoraOne/MacoraOne/releases/download/installer-v1.0.2/install.sh | tr -d '\r' | bash
```

This installer requires `sudo` access. If you prefer to inspect and run the complete installation command yourself, use the manual method below.

## Download and install manually

Run this complete command in a terminal:

```bash
set -eu

URL="https://github.com/MacoraOne/MacoraOne/releases/download/one-v1.0.2/macoraone-linux-x86_64.zip"
TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

curl -fL "$URL" -o "$TMP_DIR/macoraone-linux-x86_64.zip"
unzip -q "$TMP_DIR/macoraone-linux-x86_64.zip" -d "$TMP_DIR/extracted"

# Locate the executable inside the archive.
BIN="$(find "$TMP_DIR/extracted" -type f \( -name 'macoraone' -o -name 'macoraone-linux-x86_64' -o -name 'one' \) -print -quit)"

if [ -z "$BIN" ]; then
    echo "Error: could not find the MacoraOne executable in the release archive." >&2
    echo "Archive contents:" >&2
    find "$TMP_DIR/extracted" -maxdepth 3 -type f -print >&2
    exit 1
fi

chmod +x "$BIN"
sudo install -m 0755 "$BIN" /usr/local/bin/one

echo "MacoraOne installed as the 'one' command at /usr/local/bin/one"
one --help
```

If the program does not support `--help`, the installation can still have succeeded; use the program's documented arguments.

## Verify installation

```bash
command -v one
one --help
```

The first command should print:

```text
/usr/local/bin/one
```

Because `/usr/local/bin` is normally included in the system `PATH`, `one` can be called from any directory. If the shell cannot find it, open a new terminal and check that `/usr/local/bin` is in `PATH`.

## Uninstall

```bash
sudo rm -f /usr/local/bin/one
```


## Install the greenServeFE VS Code extension

The greenServeFE VS Code extension adds syntax highlighting and language support for `.gsve` files.

Make sure the Visual Studio Code command-line interface (`code`) is available in your terminal. Then run:

```bash
# Check whether the VS Code CLI is installed
if ! command -v code >/dev/null 2>&1; then
    echo "VS Code CLI is not installed."
    echo "Install Visual Studio Code first: https://code.visualstudio.com/"
    exit 1
fi

# Download the greenServeFE VSIX extension v1.0.1
curl -fL \
  "https://github.com/dominexmacedon-docs/greenServeFE-/releases/download/gs-extension-v1.0.1/greenServeFE-syntax-1.0.1.vsix" \
  -o greenServeFE-syntax-1.0.1.vsix

# Install the extension
code --install-extension greenServeFE-syntax-1.0.1.vsix

# Confirm installation
code --list-extensions | grep -i greenserve
```

If the extension is installed successfully, the final command should list its extension identifier. You can then open or create a `.gsve` file in VS Code.

## Release

Download: [MacoraOne v1.0.2 Linux x86_64](https://github.com/MacoraOne/MacoraOne/releases/download/one-v1.0.2/macoraone-linux-x86_64.zip)

Installer: [install.sh v1.0.2](https://github.com/MacoraOne/MacoraOne/releases/download/installer-v1.0.2/install.sh)

Project: [MacoraOne on GitHub](https://github.com/MacoraOne/MacoraOne)
