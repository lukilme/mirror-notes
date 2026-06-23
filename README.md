# Guitar Visualizer — C++ / openFrameworks

Visualizador gráfico em tempo real que transforma o sinal de guitarra em imagens
reativas usando áudio → análise → mapeamento → OpenGL.

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
---

## Controlos em runtime

| Tecla | Acção |
|-------|-------|
| `F1` | Mostrar/ocultar HUD |
| `F2` | Debug (FPS, buffer size) |
| `+` / `-` | Aumentar/diminuir sensibilidade RMS |
| `ESC` | Sair |

---