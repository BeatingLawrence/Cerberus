#!/usr/bin/env bash
set -euo pipefail

usage() {
    cat <<'EOF'
Usage:
  scripts/sync_remote_sysroot.sh <user@host> [sysroot-dir] [extra-remote-path ...]

Behavior:
  - Synchronizes the standard runtime and development paths from a remote ARM64 Linux target
  - Writes the sysroot locally without deleting existing files
  - Defaults to ./sysroot inside the framework root
  - Additional remote paths can be appended as extra arguments

Examples:
  scripts/sync_remote_sysroot.sh pi@target-host
  scripts/sync_remote_sysroot.sh pi@target-host ./sysroot
  scripts/sync_remote_sysroot.sh root@192.168.1.50 ./sysroot /usr/local /opt
EOF
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    usage
    exit 0
fi

if [[ $# -lt 1 ]]; then
    echo "error: expected <user@host>" >&2
    usage >&2
    exit 1
fi

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TARGET_SSH="$1"

if [[ $# -eq 1 ]]; then
    SYSROOT_DIR="${PROJECT_ROOT}/sysroot"
    shift 1
else
    if [[ "$2" == /* ]]; then
        SYSROOT_DIR="$2"
    else
        SYSROOT_DIR="${PROJECT_ROOT}/${2#./}"
    fi
    shift 2
fi

mkdir -p "$SYSROOT_DIR"

SSH_CONTROL_PATH="${TMPDIR:-/tmp}/cerberus-sysroot-%r@%h:%p"
SSH_OPTIONS=(
    -o BatchMode=yes
    -o PreferredAuthentications=publickey
    -o PubkeyAuthentication=yes
    -o ControlMaster=auto
    -o ControlPersist=600
    -o "ControlPath=${SSH_CONTROL_PATH}"
)

ssh_remote() {
    ssh "${SSH_OPTIONS[@]}" "$TARGET_SSH" "$@"
}

sync_dir() {
    local remote_dir="$1"
    local local_dir="${SYSROOT_DIR}${remote_dir}"
    mkdir -p "$local_dir"
    rsync -e "ssh ${SSH_OPTIONS[*]}" -aHAX --numeric-ids "${TARGET_SSH}:${remote_dir}/" "${local_dir}/"
}

sync_file() {
    local remote_file="$1"
    local local_file="${SYSROOT_DIR}${remote_file}"
    mkdir -p "$(dirname "$local_file")"
    rsync -e "ssh ${SSH_OPTIONS[*]}" -aHAX --numeric-ids "${TARGET_SSH}:${remote_file}" "${local_file}"
}

remote_dir_exists() {
    local remote_dir="$1"
    ssh_remote "test -d '$remote_dir'"
}

remote_file_exists() {
    local remote_file="$1"
    ssh_remote "test -e '$remote_file'"
}

sync_dir_if_exists() {
    local remote_dir="$1"
    if remote_dir_exists "$remote_dir"; then
        sync_dir "$remote_dir"
    fi
}

sync_file_if_exists() {
    local remote_file="$1"
    if remote_file_exists "$remote_file"; then
        sync_file "$remote_file"
    fi
}

sync_dir_if_exists /lib/aarch64-linux-gnu
sync_file_if_exists /lib/ld-linux-aarch64.so.1
sync_dir /usr/include
sync_dir_if_exists /usr/lib/linux/uapi
sync_dir_if_exists /usr/lib/aarch64-linux-gnu
sync_dir_if_exists /usr/lib/pkgconfig
sync_dir_if_exists /usr/share/pkgconfig
sync_file_if_exists /etc/ld.so.conf
sync_dir_if_exists /etc/ld.so.conf.d

for extra_path in "$@"; do
    if ssh_remote "test -d '$extra_path'"; then
        sync_dir "$extra_path"
    else
        sync_file "$extra_path"
    fi
done
