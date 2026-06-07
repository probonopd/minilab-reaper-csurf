#!/usr/bin/env bash

set -euo pipefail

REAPER_DIR="/opt/REAPER"
REAPER_CFG="${HOME}/.config/REAPER"
BACKUP_DIR="${REAPER_CFG}/midi-cache-backup-$(date +%Y%m%d-%H%M%S)"

mkdir -p "${REAPER_CFG}" "${BACKUP_DIR}"

for file in reaper-midihw-alsa.ini reaper-midihw-linux.ini; do
  if [[ -f "${REAPER_CFG}/${file}" ]]; then
    cp "${REAPER_CFG}/${file}" "${BACKUP_DIR}/${file}"
    rm -f "${REAPER_CFG}/${file}"
  fi
done

cd "${REAPER_DIR}"
exec ./reaper "$@"
