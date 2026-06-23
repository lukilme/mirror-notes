# 🎸 Guitar Visualizer — C++ / openFrameworks

Visualizador gráfico em tempo real que transforma o sinal de guitarra em imagens
reativas usando áudio → análise → mapeamento → OpenGL.

---

## Arquitetura do pipeline

```
┌─────────────┐    ┌───────────────────┐    ┌───────────────┐    ┌──────────────┐
│  Interface  │───▶│  AudioAnalyzer    │───▶│  VisualMapper │───▶│  ofApp::draw │
│  de áudio   │    │  (thread de áudio)│    │  (thread gráf)│    │  (OpenGL 60fps)│
└─────────────┘    └───────────────────┘    └───────────────┘    └──────────────┘
                         │ mutex                    │
                         ▼                          ▼
                   AudioFeatures              VisualParams
                   (RMS, FFT, pitch,         (cor, raio, ondas,
                    onset, bandas)            partículas, barras)
```

---

## Estrutura de ficheiros

```
guitar_visualizer/
├── src/
│   ├── main.cpp           – janela OpenGL 3.2 + loop principal
│   ├── ofApp.h / .cpp     – coordena áudio ↔ gráficos, lida com input
│   ├── AudioAnalyzer.h/.cpp – FFT, RMS, bandas, onset, pitch
│   ├── VisualMapper.h/.cpp  – traduz AudioFeatures → VisualParams
│   └── ParticleSystem.h/.cpp – sistema de partículas com blend aditivo
└── CMakeLists.txt
```

---

## Módulos em detalhe

### 1. `AudioAnalyzer`
- **Entrada**: buffer de amostras de float (mono, 44100 Hz, 1024 samples)
- **FFT interna** (Cooley–Tukey, power-of-two, janela de Hann)
- **Saída** (`AudioFeatures`):
  | Campo | Descrição |
  |-------|-----------|
  | `rms` | Intensidade global [0..1] |
  | `bass/mid/treble` | Energia por banda [0..1] |
  | `pitch` | Frequência dominante em Hz |
  | `noteIndex` | 0=C … 11=B (-1 = silêncio) |
  | `onsetFlag` | `true` por 1 frame quando há ataque |
  | `spectrum[]` | Bins FFT normalizados |

### 2. `VisualMapper`
Converte `AudioFeatures` suavizadas em `VisualParams`:

| Parâmetro de áudio | → Visual |
|--------------------|----------|
| RMS | Raio do círculo central |
| Bass | Amplitude da onda de fundo + cor quente/fria |
| Treble | Brilho/glow superior + sparkles |
| Nota detectada | Cor primária (paleta de 12 notas) |
| Onset | Explosão de partículas + flash branco |
| Spectrum[] | 128 barras de espectro (log-scale) |

### 3. `ParticleSystem`
- Pool de 3000 partículas (ring buffer, sem alocação por frame)
- **`burst()`** – onset dispara 60–80 partículas em todas as direções
- **`sparkle()`** – treble spawna faíscas que sobem pelo topo
- Blending aditivo → efeito de brilho

### 4. `ofApp`
- `audioIn()` corre na thread de áudio (mutex protege `latestFeatures`)
- `update()` faz smoothing com lerp (α ≈ 8 Hz)
- `draw()` compõe as camadas:
  1. Fade de fundo (trail)
  2. Onda de bass (polyline animada)
  3. Barras de espectro (log-spaced)
  4. Círculo central pulsante + glow
  5. Sparkles de treble
  6. Burst de onset
  7. Sistema de partículas
  8. HUD / UI

---

## Dependências

- **openFrameworks** ≥ 0.12 — https://openframeworks.cc/download/
- **CMake** ≥ 3.16
- Addons recomendados (opcionais, para extensão futura):
  - `ofxFft` — FFT mais rápida (FFTW)
  - `ofxAudioAnalyzer` — onset/beat tracking avançado
  - `ofxGui` — sliders em runtime para sensibilidade

---

## Build (Linux)

```bash
# 1. Instalar openFrameworks (descompactar em /opt/openframeworks ou outro path)
# 2. Configurar dependências
cd /opt/openframeworks && ./scripts/linux/install_dependencies.sh

# 3. Compilar o projecto
cd guitar_visualizer
mkdir build && cd build
cmake .. -DOF_ROOT=/opt/openframeworks
make -j$(nproc)

# 4. Executar
./GuitarVisualizer
```

## Build (macOS – Xcode)
Use o **Project Generator** do openFrameworks para gerar o `.xcodeproj`,
adicione os `.cpp/.h` ao target e desactive ARC.

## Build (Windows – VS2022)
Use o **Project Generator** para gerar a solução `.sln`; CMake também
funciona com MSVC — ajuste os paths das libs em `CMakeLists.txt`.

---

## Controlos em runtime

| Tecla | Acção |
|-------|-------|
| `F1` | Mostrar/ocultar HUD |
| `F2` | Debug (FPS, buffer size) |
| `+` / `-` | Aumentar/diminuir sensibilidade RMS |
| `ESC` | Sair |

---

## Extensões sugeridas

- **Beat tracking** – somar energia de bass em janelas de 500 ms, detectar BPM
- **Shader GLSL** – substituir `ofDrawCircle` por fragment shaders para efeitos mais ricos
- **ofxGui** – painel de controlo em runtime para todos os parâmetros do mapper
- **ofxMidi** – receber CC MIDI para controlar cenas manualmente
- **Gravação** – capturar frames com `ofImage::grabScreen()` e exportar vídeo
