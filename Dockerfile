FROM ubuntu:22.04

# Evita prompts interativos
ENV DEBIAN_FRONTEND=noninteractive

# Atualiza e instala dependências do sistema, OpenCV 4, Qt5, Python e pip
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential cmake pkg-config \
    qtbase5-dev qttools5-dev qttools5-dev-tools \
    libopencv-dev \
    python3 python3-pip python3-venv python3-setuptools python3-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Instala bibliotecas Python necessárias para o seu script
RUN pip install --no-cache-dir \
    astropy astroquery matplotlib numpy \
    concurrent-log-handler

# Define o diretório de trabalho
WORKDIR /app

# Copia o código-fonte (C++ e Python)
COPY . .

# Compila o projeto C++ (ajuste se o seu Makefile usar outro alvo)
RUN make re CXXFLAGS="-fPIC -I/usr/include/opencv4 $(pkg-config --cflags Qt5Widgets)"

# Define o comando padrão (altere conforme desejar)
# Exemplo: executa o binário C++ principal
CMD ["./map_obj_viw"]

# (Opcional) — se quiser rodar o script Python em vez do binário:
# CMD ["python3", "seu_script.py"]

