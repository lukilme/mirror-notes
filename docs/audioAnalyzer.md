# Explicação Matemática da Classe AudioAnalyzer

Este documento apresenta a fundamentação matemática detalhada para cada um dos módulos e algoritmos implementados na classe `AudioAnalyzer`, utilizada para processamento digital de sinais (DSP) e extração de características de áudio em tempo real.

---

## 1. Janelamento (Hann Window)

No método `setup`, é inicializada a janela de Hann. No processamento digital de áudio, os sinais são modelados como sequências contínuas ou infinitas. Para processá-los em tempo real, o sinal é segmentado em blocos finitos de comprimento $N$ (onde $N = \text{fftSize}$).

A truncação abrupta de um sinal gera descontinuidades artificiais nas bordas do bloco, um fenômeno que introduz altas frequências inexistentes no sinal original, conhecido como **vazamento espectral** (*spectral leakage*). Para mitigar esse efeito, o sinal amostrado é multiplicado por uma função de janela que atenua suavemente as extremidades até zero.

A equação matemática da janela de Hann implementada é:

$$
w[i] = 0.5 \cdot \left(1 - \cos\left(\frac{2\pi i}{N - 1}\right)\right)
$$

Para $i \in [0, N-1]$.

---

## 2. Raiz Quadrada Média (Root Mean Square - RMS)

O método `computeRMS` determina a amplitude média do sinal no domínio do tempo. O valor RMS correlaciona-se diretamente com a energia física do sinal e com a percepção psicofísica de volume (*loudness*) da audição humana.

A formulação matemática para um bloco de $n$ amostras é dada por:

$$
X_{\text{RMS}} = \sqrt{\frac{1}{n} \sum_{i=0}^{n-1} s[i]^2}
$$

Onde $s[i]$ representa a $i$-ésima amostra escalar do vetor de áudio.

---

## 3. Transformada Rápida de Fourier (FFT) e Cálculo de Magnitude

A Transformada Discreta de Fourier (DFT) mapeia o sinal do domínio do tempo discreto para o domínio da frequência complexa. A equação analítica da DFT é:

$$
X[k] = \sum_{n=0}^{N-1} x[n] \cdot e^{-j \frac{2\pi}{N}kn}
$$

Onde $j = \sqrt{-1}$ é a unidade imaginária e $k \in [0, N-1]$ representa o índice da componente de frequência (bin).

O algoritmo de **Cooley-Tukey** implementado na função estática `fft` otimiza esse cálculo de uma complexidade computacional de $O(N^2)$ para $O(N \log_2 N)$ através de uma abordagem de divisão e conquista *in-place*, utilizando permutações por inversão de bits (*bit-reversal*) e estruturas de recombinação conhecidas como **borboletas** (*butterflies*).

Como o resultado $X[k]$ é um vetor de números complexos com parte real $R[k]$ e parte imaginária $I[k]$:

$$
X[k] = R[k] + j \cdot I[k]
$$

A magnitude física absoluta (amplitude) de cada componente espectral é calculada no método `computeFFT` através da norma euclidiana, multiplicada por um fator de normalização:

$$
M[k] = \sqrt{R[k]^2 + I[k]^2} \cdot \frac{2}{N}
$$

*Nota: A normalização por $\frac{2}{N}$ compensa a perda de amplitude decorrente do fato de a energia de um sinal real ser distribuída simetricamente entre as frequências positivas e negativas. Devido a esta simetria (Teorema de Nyquist-Shannon), apenas a primeira metade do vetor ($N/2$) contém informação física única e é armazenada no vetor `magnitude`.*

---

## 4. Mapeamento de Frequência Linear (`binToHz`)

A FFT divide o espectro discreto que vai de $0\text{ Hz}$ até a frequência de amostragem $f_s$ (`sampleRate`) em $N$ sub-bandas lineares de largura uniforme. A resolução espectral (a largura de cada caixa ou *bin*) em Hertz é dada pela razão:

$$
\Delta f = \frac{f_s}{N}
$$

A frequência central $f$ em Hertz correspondente a um índice espectral $k$ (`bin`) é calculada linearmente através de:

$$
f = k \cdot \frac{f_s}{N}
$$

---

## 5. Energia por Bandas Frequenciais (`bandEnergy`)

Para analisar o comportamento de diferentes regiões do espectro acústico (como os graves, médios e agudos de uma guitarra), o método `bandEnergy` integra a energia quadrática das magnitudes contidas em um intervalo fechado de bins $[k_{\text{low}}, k_{\text{high}}]$:

$$
E_{\text{banda}} = \sqrt{\frac{1}{k_{\text{high}} - k_{\text{low}} + 1} \sum_{k=k_{\text{low}}}^{k_{\text{high}}} M[k]^2}
$$

No loop principal do método `process`, estes valores escalares de energia são ponderados por coeficientes empíricos multiplicativos (`8.0`, `4.0` e `10.0`) para calibrar a sensibilidade dinâmica do analisador às faixas de frequência do instrumento, sendo posteriormente saturados no teto unitário através da função:

$$
\min(1.0, E_{\text{ponderada}})
$$

---

## 6. Estimação de Pitch e Conversão para Escala Musical (`hzToNote`)

O algoritmo realiza uma estimação elementar da frequência fundamental ($f_0$) ao identificar o índice do bin de maior magnitude espectral dentro de uma janela de busca anatômica (restringida entre as sub-bandas `bassLow` e `midHigh`).

A conversão da frequência contínua $f$ (em Hz) para o domínio discreto das notas musicais baseia-se no padrão de afinação de temperamento igual de 12 tons, onde a nota de referência Lá 4 (A4) é fixada estritamente em $440\text{ Hz}$ e mapeada para o índice numérico MIDI 69.

A relação geométrica que rege a frequência em função do número de semitons $d$ de distância da referência é:

$$
f = 440 \cdot 2^{\frac{d}{12}}
$$

Aplicando o logaritmo na base 2 para isolar a variável linear $d$ e transladando para a escala MIDI, obtém-se a fórmula implementada em `hzToNote`:

$$
\text{midi} = 69 + 12 \cdot \log_2\left(\frac{f}{440}\right)
$$

O índice cromático da nota dentro da oitava padrão ($0 = \text{Dó}$, $1 = \text{Dó\#}$, ..., $9 = \text{Lá}$, ..., $11 = \text{Si}$) é extraído através do operador módulo aritmético:

$$
\text{noteIdx} = \lfloor \text{midi} + 0.5 \rfloor \pmod{12}
$$

---

## 7. Detecção de Transientes Dinâmicos (Onset Detection via Leaky Integrator)

A detecção de ataques ou inícios de notas (*onsets*) baseia-se na variação temporal instantânea da potência do sinal.

### 7.1 Cálculo da Potência Atual

É o quadrado do valor RMS instantâneo:

$$
E_t = X_{\text{RMS}}^2
$$

### 7.2 Diferenciação Temporal

Mede o gradiente de crescimento de energia:

$$
\Delta = E_t - E_{\text{passada}}
$$

### 7.3 Critério de Ativação

Um evento de ataque é validado logicamente se o gradiente violar o limiar estático:

$$
\Delta > \text{onsetThresh}
$$

e o sinal contiver uma amplitude mínima de segurança:

$$
X_{\text{RMS}} > 0.05
$$

### 7.4 Filtro Recursivo Passa-Baixas (Integrador Vazado)

A variável de referência histórica $E_{\text{passada}}$ é atualizada a cada bloco através de um filtro de resposta ao impulso infinita (IIR) de primeira ordem:

$$
E_{\text{passada}}^{(t)} = 0.8 \cdot E_t + 0.2 \cdot E_{\text{passada}}^{(t-1)}
$$

Esta equação faz com que o limiar histórico acompanhe a evolução dinâmica do sinal com uma taxa de decaimento exponencial, prevenindo múltiplos disparos falsos causados por oscilações secundárias do transiente.