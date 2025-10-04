FROM ubuntu:22.04

# Não interativo
ENV DEBIAN_FRONTEND=noninteractive

# Atualiza e instala dependências do sistema, OpenCV e compilador
RUN apt-get update && \
    apt install qt6-base-dev qt6-tools-dev qt6-tools-dev-tools && \
    apt-get install -y build-essential cmake pkg-config \
    libopencv-dev


# Cria diretório de trabalho
WORKDIR /app

# Copia o código-fonte para dentro da imagem
COPY . .

# Compila o projeto (ajuste conforme seu arquivo principal)
RUN make re

# Define o comando padrão para rodar o executável
CMD ["./map_obj_viw"]
