#!/bin/sh
# Install for the current desktop, or stage files for a distribution package.
set -eu
source_dir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)/org.floitsch.shiftlayout.status
version=
destination=
while [ "$#" -gt 0 ]; do
    case "$1" in
        --plasma-version) version=$2; shift 2 ;;
        --destdir) destination=$2; shift 2 ;;
        *) echo "Usage: $0 [--plasma-version 5|6] [--destdir plasmoids-directory]" >&2; exit 2 ;;
    esac
done
if [ -z "$version" ]; then
    if command -v kpackagetool6 >/dev/null 2>&1; then version=6; else version=5; fi
fi
case "$version" in 5|6) ;; *) echo "Plasma version must be 5 or 6" >&2; exit 2 ;; esac
stage=$(mktemp -d)
trap 'rm -rf "$stage"' EXIT HUP INT TERM
cp -R "$source_dir" "$stage/org.floitsch.shiftlayout.status"
package=$stage/org.floitsch.shiftlayout.status
if [ "$version" = 5 ]; then
    cp "$package/metadata5.json" "$package/metadata.json"
    cp "$package/contents/ui/main5.qml" "$package/contents/ui/main.qml"
fi
rm "$package/metadata5.json" "$package/contents/ui/main5.qml"
if [ -n "$destination" ]; then
    mkdir -p "$destination"
    cp -R "$package" "$destination/"
else
    tool=kpackagetool$version
    # --show also finds system packages, but --upgrade only replaces a local
    # installation. Check the actual user directory rather than search results.
    package_root=${XDG_DATA_HOME:-"$HOME/.local/share"}/plasma/plasmoids
    mkdir -p "$package_root"
    if [ -d "$package_root/org.floitsch.shiftlayout.status" ]; then
        "$tool" --type Plasma/Applet --packageroot "$package_root" --upgrade "$package"
    else
        "$tool" --type Plasma/Applet --packageroot "$package_root" --install "$package"
    fi
fi
