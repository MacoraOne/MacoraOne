# 2. Installation

## Public distribution

Release packages are published at:
https://github.com/dominexmacedon-docs/greenServeFE-

The documented v1.0.0 Linux x86_64 release contains:
- greenServeFE-v1.0.0
- gs_table-v1.0.0
- gsnum-v1.0.0
- server-v1.0.0

## Debian/Ubuntu installation

~~~bash
set -e
sudo apt-get update
sudo apt-get install -y curl unzip

sudo rm -f /usr/local/bin/greenServeFE
sudo rm -f /usr/local/lib/modules/gs_table.so
sudo rm -f /usr/local/lib/modules/gsnum.so
sudo rm -f /usr/local/lib/modules/server.so
sudo rm -f /etc/profile.d/greenServeFE.sh
unset GREENSERVE_MODULE_PATH

tmp_dir="$(mktemp -d)"
trap 'rm -rf "$tmp_dir"' EXIT
cd "$tmp_dir"

curl -fL --show-error --retry 3 "https://github.com/dominexmacedon-docs/greenServeFE-/releases/download/greenServeFE-v1.0.0/greenServeFE-linux-x86_64.zip" -o greenServeFE-linux-x86_64.zip
curl -fL --show-error --retry 3 "https://github.com/dominexmacedon-docs/greenServeFE-/releases/download/gs_table-v1.0.0/gs_table-linux-x86_64.zip" -o gs_table-linux-x86_64.zip
curl -fL --show-error --retry 3 "https://github.com/dominexmacedon-docs/greenServeFE-/releases/download/gsnum-v1.0.0/gsnum-linux-x86_64.zip" -o gsnum-linux-x86_64.zip
curl -fL --show-error --retry 3 "https://github.com/dominexmacedon-docs/greenServeFE-/releases/download/server-v1.0.0/server-linux-x86_64.zip" -o server-linux-x86_64.zip

mkdir -p greenServeFE gs_table gsnum server
unzip -q -o greenServeFE-linux-x86_64.zip -d greenServeFE
unzip -q -o gs_table-linux-x86_64.zip -d gs_table
unzip -q -o gsnum-linux-x86_64.zip -d gsnum
unzip -q -o server-linux-x86_64.zip -d server

test -f greenServeFE/greenServeFE
test -f gs_table/gs_table.so
test -f gsnum/gsnum.so
test -f server/server.so

sudo install -d -m 755 /usr/local/bin
sudo install -m 755 greenServeFE/greenServeFE /usr/local/bin/greenServeFE
sudo install -d -m 755 /usr/local/lib/modules
sudo install -m 755 gs_table/gs_table.so /usr/local/lib/modules/gs_table.so
sudo install -m 755 gsnum/gsnum.so /usr/local/lib/modules/gsnum.so
sudo install -m 755 server/server.so /usr/local/lib/modules/server.so

echo 'export GREENSERVE_MODULE_PATH=/usr/local/lib/modules' | sudo tee /etc/profile.d/greenServeFE.sh >/dev/null
sudo chmod 644 /etc/profile.d/greenServeFE.sh
export GREENSERVE_MODULE_PATH=/usr/local/lib/modules

greenServeFE --version
test -x /usr/local/bin/greenServeFE
test -f /usr/local/lib/modules/gs_table.so
test -f /usr/local/lib/modules/gsnum.so
test -f /usr/local/lib/modules/server.so
test -f /etc/profile.d/greenServeFE.sh
~~~

The public release README is the authoritative installation source for future release changes.
