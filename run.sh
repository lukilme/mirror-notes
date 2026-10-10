#!/usr/bin/env bash

set -Eeuo pipefail

OF_ROOT="/opt/openframeworks"
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"
EXECUTABLE="${BUILD_DIR}/GuitarVisualizer"

log() {
    echo
    echo "==> $1"
}

error() {
    echo
    echo "ERRO: $1" >&2
    exit 1
}

log "Verificando dependências"

command -v cmake >/dev/null 2>&1 \
    || error "CMake não está instalado."

command -v make >/dev/null 2>&1 \
    || error "Make não está instalado."

command -v nproc >/dev/null 2>&1 \
    || error "nproc não está disponível."

[[ -d "$OF_ROOT" ]] \
    || error "OpenFrameworks não encontrado em: $OF_ROOT"

[[ -x "$OF_ROOT/scripts/linux/install_dependencies.sh" ]] \
    || error "Script de dependências do OpenFrameworks não encontrado."

log "Instalando dependências do OpenFrameworks"

cd "$OF_ROOT"

sudo ./scripts/linux/install_dependencies.sh

log "Configurando projeto"

mkdir -p "$BUILD_DIR"

cd "$BUILD_DIR"

cmake .. \
    -DOF_ROOT="$OF_ROOT"

log "Compilando projeto"

cmake --build . --parallel "$(nproc)"

[[ -x "$EXECUTABLE" ]] \
    || error "Executável não encontrado: $EXECUTABLE"

log "Configurando câmera para formato compatível com V4L2"

VIDEO_DEVICE="${VIDEO_DEVICE:-}"
CANDIDATES=()
if [[ -n "$VIDEO_DEVICE" ]]; then
    CANDIDATES+=("$VIDEO_DEVICE")
fi
for dev in /dev/video0 /dev/video1 /dev/video2; do
    [[ -e "$dev" ]] || continue
    CANDIDATES+=("$dev")
done

ACTIVE_VIDEO_DEVICE=""
for dev in "${CANDIDATES[@]}"; do
    [[ -n "$dev" ]] || continue
    if v4l2-ctl --device="$dev" --list-formats-ext >/dev/null 2>&1; then
        if v4l2-ctl --device="$dev" --set-fmt-video=width=640,height=480,pixelformat=YUYV >/dev/null 2>&1 || \
           v4l2-ctl --device="$dev" --set-fmt-video=width=640,height=480,pixelformat=MJPG >/dev/null 2>&1; then
            ACTIVE_VIDEO_DEVICE="$dev"
            echo "Câmera ajustada em $ACTIVE_VIDEO_DEVICE"
            break
        fi
    fi
done

if [[ -n "$ACTIVE_VIDEO_DEVICE" ]]; then
    export VIDEO_DEVICE="$ACTIVE_VIDEO_DEVICE"
else
    echo "Aviso: nenhuma câmera compatível foi encontrada; o app continuará sem webcam." >&2
fi

log "Executando GuitarVisualizer"

exec "$EXECUTABLE"