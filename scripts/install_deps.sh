#!/usr/bin/env bash
# scripts/install_deps.sh -- install the system packages GeoZui4D needs to
# build and run its tests, on Debian or Ubuntu. The lists below are the
# project's single list of build dependencies; the README points here.
#
# The script knows the platform it runs on (/etc/os-release): the packages
# are the common list, plus what that release adds, minus what it drops
# (RELEASE_ADD and RELEASE_DROP below). A release with no entry gets the
# common list and a note that the build has not been checked there.
#
# Usage: scripts/install_deps.sh [--list] [--dry-run] [--update] [--yes]
#
#   --list     print this platform's package names, one per line, and exit
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

# Needed on every release.
PACKAGES=(
    build-essential cmake
    libboost-all-dev
    tcl-dev tk-dev
    libgdal-dev
    libjpeg-dev libtiff-dev libpng-dev
    libnetcdf-dev libnetcdf-c++4-dev
    freeglut3-dev libglew-dev
    libx11-dev libxmu-dev
    portaudio19-dev libjack-dev
    libavformat-dev libavcodec-dev libavutil-dev
    libvorbis-dev
    libbluetooth-dev
    libspnav-dev
    libgtest-dev    # tests (-DBUILD_GTESTS=ON) offline; without it configure fetches GoogleTest
)

# Not listed, because the build works without it: libopenni-dev (OpenNI).
# When it is installed, the Kinect object is built (GZ4D_KINECT);
# -DGZ4D_WITH_KINECT=OFF leaves it out even then.

# Packages that stand in for a listed one when installed: JACK 2's headers
# conflict with libjack-dev (JACK 1) and serve the build just as well.
declare -A ALTERNATIVES=(
    [libjack-dev]=libjack-jackd2-dev
)

# Per-release requirements, keyed "<ID>:<VERSION_ID>" as /etc/os-release
# gives them. A release has an entry in RELEASE_ADD when the build is known
# to work there, even when it adds nothing.
#   RELEASE_ADD   packages that release needs on top of the common list
#   RELEASE_DROP  common packages that release does not have or need
# Both are space-separated names. Today the two known releases install the
# same packages: what differs between them (Boost 1.90, GDAL 3.12, GCC 15
# on 26.04) is handled in the CMake files and the source, not here.
declare -A RELEASE_ADD=(
    [ubuntu:24.04]=""
    [ubuntu:26.04]=""
)
declare -A RELEASE_DROP=(
    [ubuntu:24.04]=""
    [ubuntu:26.04]=""
)

usage() { sed -n '11,16p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'; }

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

OS_ID="" OS_LIKE="" OS_VERSION="" OS_NAME="this system"
if [ -r /etc/os-release ]; then
    # shellcheck disable=SC1091
    . /etc/os-release
    OS_ID="${ID:-}" OS_LIKE="${ID_LIKE:-}" OS_VERSION="${VERSION_ID:-}" OS_NAME="${PRETTY_NAME:-$OS_ID}"
fi

# This platform's list: common, minus what the release drops, plus what it
# adds. An unknown release keeps the common list; the note goes to stderr so
# that --list still prints package names only.
RELEASE="$OS_ID:$OS_VERSION"
if [[ -v RELEASE_ADD[$RELEASE] ]]; then
    read -r -a ADD <<< "${RELEASE_ADD[$RELEASE]}"
    read -r -a DROP <<< "${RELEASE_DROP[$RELEASE]:-}"
    KEPT=()
    for pkg in "${PACKAGES[@]}"; do
        case " ${DROP[*]:-} " in *" $pkg "*) ;; *) KEPT+=("$pkg") ;; esac
    done
    PACKAGES=("${KEPT[@]}" "${ADD[@]}")
else
    known="$(printf '%s\n' "${!RELEASE_ADD[@]}" | sort | tr '\n' ' ')"
    echo "install_deps.sh: note: the build has not been checked on $OS_NAME (known: ${known% }); using the common package list" >&2
fi

if [ "$LIST" -eq 1 ]; then
    printf '%s\n' "${PACKAGES[@]}"
    exit 0
fi

# Debian and its derivatives only: the names above are Debian package names.
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
