#!/usr/bin/env bash
# scripts/install_deps.sh -- install the system packages GeoZui4D needs to
# build and run its tests, on Debian or Ubuntu. The package list below is
# the project's single list of build dependencies; the README points here.
#
# Usage: scripts/install_deps.sh [--list] [--dry-run] [--update] [--yes]
#
#   --list     print the package names, one per line, and exit
#   --dry-run  say which packages are missing or unavailable; install nothing
#   --update   run `apt-get update` first (a fresh container has no lists)
#   --yes      install without asking (for CI and other unattended use)
#
# Packages already installed are left alone. When a package has no install
# candidate, nothing is installed and the script exits 1, naming it.
# Runs apt-get through sudo unless it is already running as root.
#
# Exit: 0 everything is installed (or would be), 1 a package is unavailable
# or the install failed, 2 usage error or not a Debian/Ubuntu system.
set -euo pipefail

PACKAGES=(
    build-essential cmake
    libboost-all-dev
    tcl-dev tk-dev
    libgdal-dev
    libjpeg-dev libtiff-dev libpng-dev
    libnetcdf-dev libnetcdf-c++4-dev
    freeglut3-dev libglew-dev
    libx11-dev
    portaudio19-dev libjack-dev
    libavformat-dev libavcodec-dev libavutil-dev
    libvorbis-dev
    libbluetooth-dev
    libspnav-dev
    libgtest-dev    # tests only: scripts/build.sh sets -DBUILD_GTESTS=ON
)

# Packages that stand in for a listed one when installed: JACK 2's headers
# conflict with libjack-dev (JACK 1) and serve the build just as well.
declare -A ALTERNATIVES=(
    [libjack-dev]=libjack-jackd2-dev
)

usage() { sed -n '6,11p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; }

LIST=0 DRY_RUN=0 UPDATE=0 YES=0
for arg in "$@"; do
    case "$arg" in
        --list) LIST=1 ;;
        --dry-run) DRY_RUN=1 ;;
        --update) UPDATE=1 ;;
        --yes) YES=1 ;;
        -h|--help) usage; exit 0 ;;
        *) echo "install_deps.sh: unknown argument '$arg'" >&2; usage >&2; exit 2 ;;
    esac
done

if [ "$LIST" -eq 1 ]; then
    printf '%s\n' "${PACKAGES[@]}"
    exit 0
fi

# Debian and its derivatives only: the names above are Debian package names.
OS_ID="" OS_LIKE="" OS_NAME="this system"
if [ -r /etc/os-release ]; then
    # shellcheck disable=SC1091
    . /etc/os-release
    OS_ID="${ID:-}" OS_LIKE="${ID_LIKE:-}" OS_NAME="${PRETTY_NAME:-$OS_ID}"
fi
case " $OS_ID $OS_LIKE " in
    *" debian "*|*" ubuntu "*) ;;
    *)
        echo "install_deps.sh: $OS_NAME is not Debian or Ubuntu; install the equivalents of these packages by hand:" >&2
        printf '  %s\n' "${PACKAGES[@]}" >&2
        exit 2
        ;;
esac

SUDO=()
if [ "$(id -u)" -ne 0 ]; then
    command -v sudo >/dev/null 2>&1 || { echo "install_deps.sh: not root and no sudo" >&2; exit 2; }
    SUDO=(sudo)
fi

if [ "$UPDATE" -eq 1 ]; then
    if [ "$DRY_RUN" -eq 1 ]; then
        echo "would run: ${SUDO[*]:+${SUDO[*]} }apt-get update"
    else
        "${SUDO[@]}" apt-get update
    fi
fi

installed() {
    [[ "$(dpkg-query -W -f='${Status}' "$1" 2>/dev/null)" == *"ok installed"* ]]
}
available() {
    local candidate
    candidate="$(apt-cache policy "$1" 2>/dev/null | awk '/Candidate:/ {print $2}')"
    [ -n "$candidate" ] && [ "$candidate" != "(none)" ]
}

MISSING=() UNAVAILABLE=()
for pkg in "${PACKAGES[@]}"; do
    alt="${ALTERNATIVES[$pkg]:-}"
    if installed "$pkg" || { [ -n "$alt" ] && installed "$alt"; }; then
        continue
    fi
    if available "$pkg"; then
        MISSING+=("$pkg")
    else
        UNAVAILABLE+=("$pkg")
    fi
done

echo "$OS_NAME: ${#PACKAGES[@]} packages, ${#MISSING[@]} to install, ${#UNAVAILABLE[@]} unavailable"

if [ "${#UNAVAILABLE[@]}" -gt 0 ]; then
    echo "No install candidate for:" >&2
    printf '  %s\n' "${UNAVAILABLE[@]}" >&2
    if [ "$UPDATE" -eq 0 ]; then
        echo "If the package lists are stale or empty, run again with --update." >&2
    else
        echo "These names may differ on this release; please report it." >&2
    fi
    exit 1
fi

if [ "${#MISSING[@]}" -eq 0 ]; then
    echo "Nothing to install."
    exit 0
fi

APT=(apt-get install)
[ "$YES" -eq 1 ] && APT+=(-y)

if [ "$DRY_RUN" -eq 1 ]; then
    echo "would run: ${SUDO[*]:+${SUDO[*]} }${APT[*]} ${MISSING[*]}"
    exit 0
fi

if [ "$YES" -eq 1 ]; then
    "${SUDO[@]}" env DEBIAN_FRONTEND=noninteractive "${APT[@]}" "${MISSING[@]}"
else
    "${SUDO[@]}" "${APT[@]}" "${MISSING[@]}"
fi
