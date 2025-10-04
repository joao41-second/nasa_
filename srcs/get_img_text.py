
import os
from astropy.coordinates import SkyCoord
from astropy.time import Time
from astroquery.skyview import SkyView
from astropy.visualization import ImageNormalize, MinMaxInterval, SqrtStretch
import astropy.units as u
import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime
from concurrent.futures import ThreadPoolExecutor, as_completed


# ================== FUNÇÃO PRINCIPAL ==================
def buscar_imagem_colorida(ra_graus, dec_graus, data_hora=None, survey_list=None,
                           raio=0.5, pixels=1000, nome_arquivo='imagem.png', pasta_destino='img'):
    """
    Busca imagem colorida combinando filtros RGB do DSS2 e ajusta o balanço de cores.
    """
    if survey_list is None:
        survey_list = ['DSS2 Red', 'DSS2 Blue', 'DSS2 IR']

    # Processar data/hora
    if data_hora is None:
        tempo = Time.now()
        data_hora_str = datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')
    elif isinstance(data_hora, str):
        tempo = Time(data_hora)
        data_hora_str = data_hora
    else:
        tempo = Time(data_hora)
        data_hora_str = data_hora.strftime('%Y-%m-%d %H:%M:%S')

    coords = SkyCoord(ra=ra_graus * u.degree, dec=dec_graus * u.degree,
                      frame='icrs', obstime=tempo)

    rgb_data = []
    for survey in survey_list:
        try:
            images = SkyView.get_images(position=coords, survey=survey,
                                        radius=raio * u.degree, pixels=pixels)
            if images and len(images) > 0:
                data = images[0][0].data
                norm = ImageNormalize(data, interval=MinMaxInterval(), stretch=SqrtStretch())
                rgb_data.append(norm(data))
                print(f"✅ {survey} baixado para RA={ra_graus:.4f}, Dec={dec_graus:.4f}")
            else:
                raise ValueError(f"Nenhuma imagem disponível para {survey}")
        except Exception as e:
            print(f"❌ Erro ao baixar {survey} para RA={ra_graus}, Dec={dec_graus}: {e}")
            rgb_data.append(np.zeros((pixels, pixels)))

    # Garantir 3 canais
    while len(rgb_data) < 3:
        rgb_data.append(np.zeros((pixels, pixels)))

    # Combinação RGB (balanceada)
    rgb_image = np.zeros((pixels, pixels, 3))
    rgb_image[:, :, 0] = rgb_data[0]                      # Red
    rgb_image[:, :, 1] = (rgb_data[1] + rgb_data[2]) / 2  # Green
    rgb_image[:, :, 2] = rgb_data[1]                      # Blue

    # Normalização
    for i in range(3):
        channel = rgb_image[:, :, i]
        channel = (channel - np.min(channel)) / (np.max(channel) - np.min(channel) + 1e-8)
        rgb_image[:, :, i] = channel

    # Salvar imagem
    os.makedirs(pasta_destino, exist_ok=True)
    caminho_arquivo = os.path.join(pasta_destino, nome_arquivo)
    plt.imsave(caminho_arquivo, rgb_image)
    print(f"💾 Arquivo salvo: {caminho_arquivo}")
    return caminho_arquivo


# ================== GRADE DE IMAGENS ==================
def gerar_grade_imagens_threaded(ra_centro, dec_centro, data_hora=None,
                                 survey_list=None, raio=0.02, pixels=2000,
                                 sobreposicao=0.0):
    """
    Gera 9 imagens (3x3) ao redor da coordenada central.

    Numeração padrão:
        1 2 3
        4 5 6
        7 8 9
    O parâmetro 'sobreposicao' permite pequenas sobreposições entre imagens (0.0–0.2).
    """
    delta = 2 * raio * (1 - sobreposicao)  # separação entre centros
    offsets = [-delta, 0, delta]
    index_map = [
        [1, 2, 3],
        [4, 5, 6],
        [7, 8, 9]
    ]

    tarefas = []

    with ThreadPoolExecutor(max_workers=9) as executor:
        for row, dy in enumerate(reversed(offsets)):  # invertido para manter Norte para cima
            for col, dx in enumerate(offsets):
                ra = ra_centro + dx / np.cos(np.radians(dec_centro))
                dec = dec_centro + dy
                numero = index_map[row][col]
                nome_arquivo = f"img_{numero}.png"

                tarefas.append(executor.submit(
                    buscar_imagem_colorida,
                    ra_graus=ra,
                    dec_graus=dec,
                    data_hora=data_hora,
                    survey_list=survey_list,
                    raio=raio,
                    pixels=pixels,
                    nome_arquivo=nome_arquivo,
                    pasta_destino='img'
                ))

        for future in as_completed(tarefas):
            try:
                future.result()
            except Exception as e:
                print(f"❌ Erro em thread: {e}")

    print("✅ Todas as imagens foram geradas corretamente em 'img/'.")


# ================== EXECUÇÃO DIRETA ==================
if __name__ == "__main__":
    ra_central = 10.6847      # Exemplo: Galáxia de Andrômeda
    dec_central = 41.2689
    data_hora = "2025-10-04 11:31:02"
    survey_list = ['DSS2 Red', 'DSS2 Blue', 'DSS2 IR']
    raio = 0.1
    pixels = 1000

    gerar_grade_imagens_threaded(
        ra_centro=ra_central,
        dec_centro=dec_central,
        data_hora=data_hora,
        survey_list=survey_list,
        raio=raio,
        pixels=pixels,
        sobreposicao=0.1   # opcional: 10% de sobreposição entre imagens
    )

