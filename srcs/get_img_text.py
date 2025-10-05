import os
from astropy.coordinates import SkyCoord
from astropy.time import Time
from astroquery.skyview import SkyView
import astropy.units as u
import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime
from concurrent.futures import ThreadPoolExecutor, as_completed
from astropy.visualization import MinMaxInterval, SqrtStretch, ImageNormalize

import argparse

# ================== FUNÇÃO PRINCIPAL ==================
def buscar_imagem_colorida(ra_graus, dec_graus, data_hora=None, survey_list=None,
                           raio=0.5, pixels=1000, nome_arquivo='imagem.png',
                           pasta_destino='img', vmin_global=None, vmax_global=None,
                           balance_global=None):
    """
    Busca imagem colorida do DSS2 e aplica correção de cores global.
    """
    if survey_list is None:
        survey_list = ['DSS2 Red', 'DSS2 Blue', 'DSS2 IR']

    # Tempo de observação
    if data_hora is None:
        tempo = Time.now()
    elif isinstance(data_hora, str):
        tempo = Time(data_hora)
    else:
        tempo = Time(data_hora)

    coords = SkyCoord(ra=ra_graus * u.deg, dec=dec_graus * u.deg, frame='icrs', obstime=tempo)

    rgb_data = []
    for survey in survey_list:
        try:
            images = SkyView.get_images(position=coords, survey=survey,
                                        radius=raio * u.deg, pixels=pixels)
            if images and len(images) > 0:
                data = images[0][0].data
                rgb_data.append(data)
                print(f"✅ {survey} baixado para RA={ra_graus:.4f}, Dec={dec_graus:.4f}")
            else:
                raise ValueError("Nenhuma imagem retornada")
        except Exception as e:
            print(f"❌ Erro ao baixar {survey}: {e}")
            rgb_data.append(np.zeros((pixels, pixels)))

    while len(rgb_data) < 3:
        rgb_data.append(np.zeros((pixels, pixels)))

    # Normalização global
    if vmin_global is None or vmax_global is None:
        all_data = np.concatenate([d.flatten() for d in rgb_data])
        vmin_global, vmax_global = np.percentile(all_data, [0.5, 99.5])

    normalized = [np.clip((d - vmin_global) / (vmax_global - vmin_global), 0, 1) for d in rgb_data]

    # Combinação RGB com verde ajustado
    rgb_image = np.zeros((pixels, pixels, 3))
    rgb_image[:, :, 0] = normalized[0]  # Red
    rgb_image[:, :, 1] = 0.7 * normalized[1] + 0.3 * normalized[2]  # Green
    rgb_image[:, :, 2] = normalized[1]  # Blue

    # Balanceamento global fixo
    if balance_global is not None:
        balance_r, balance_g, balance_b = balance_global
        rgb_image[:, :, 0] *= balance_r
        rgb_image[:, :, 1] *= balance_g
        rgb_image[:, :, 2] *= balance_b

    rgb_image = np.clip(rgb_image, 0, 1)

    os.makedirs(pasta_destino, exist_ok=True)
    caminho = os.path.join(pasta_destino, nome_arquivo)
    plt.imsave(caminho, rgb_image)
    print(f"💾 Salvo: {caminho}")
    return caminho


# ================== FUNÇÃO PARA CALCULAR BALANÇO GLOBAL ==================
def calcular_balanceamento_global(ra_centro, dec_centro, raio, pixels, survey_list):
    """
    Calcula coeficientes globais R/G/B baseados na imagem central.
    """
    print("🎯 Calculando balanço de cor global baseado na imagem central...")
    images = []
    for survey in survey_list:
        data = SkyView.get_images(position=SkyCoord(ra_centro * u.deg, dec_centro * u.deg),
                                  survey=survey, radius=raio * u.deg, pixels=pixels)[0][0].data
        images.append(data)

    all_data = np.concatenate([d.flatten() for d in images])
    vmin, vmax = np.percentile(all_data, [0.5, 99.5])
    norm = [np.clip((d - vmin) / (vmax - vmin), 0, 1) for d in images]

    rgb = np.zeros((pixels, pixels, 3))
    rgb[:, :, 0] = norm[0]
    rgb[:, :, 1] = 0.7 * norm[1] + 0.3 * norm[2]
    rgb[:, :, 2] = norm[1]

    mean_r, mean_g, mean_b = np.mean(rgb[:, :, 0]), np.mean(rgb[:, :, 1]), np.mean(rgb[:, :, 2])
    mean_total = (mean_r + mean_g + mean_b) / 3

    balance_r = mean_total / (mean_r + 1e-8)
    balance_g = mean_total / (mean_g + 1e-8)
    balance_b = mean_total / (mean_b + 1e-8)

    print(f"🔧 Coeficientes globais: R={balance_r:.3f}, G={balance_g:.3f}, B={balance_b:.3f}")
    return (vmin, vmax), (balance_r, balance_g, balance_b)


# ================== GRADE DE IMAGENS ==================
def gerar_grade_imagens_threaded(ra_centro, dec_centro, data_hora=None,
                                 survey_list=None, raio=0.02, pixels=2000,
                                 sobreposicao=0.0):
    """
    Gera 9 imagens (3x3) com balanço global uniforme.
    """
    delta = 2 * raio * (1 - sobreposicao)
    offsets = [-delta, 0, delta]
    index_map = [
        [1, 2, 3],
        [4, 5, 6],
        [7, 8, 9]
    ]

    # Calcular normalização e balanço global baseados no centro
    (vmin, vmax), balance = calcular_balanceamento_global(ra_centro, dec_centro, raio, pixels, survey_list)

    tarefas = []
    with ThreadPoolExecutor(max_workers=9) as executor:
        for row, dy in enumerate(reversed(offsets)):  # norte para cima
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
                    pasta_destino='img',
                    vmin_global=vmin,
                    vmax_global=vmax,
                    balance_global=balance
                ))

        for future in as_completed(tarefas):
            try:
                future.result()
            except Exception as e:
                print(f"❌ Erro: {e}")

    print("✅ Todas as imagens foram geradas com balanço uniforme.")


# ================== EXECUÇÃO DIRETA ==================


if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Gera uma grade 3x3 de imagens do DSS2 com balanço global uniforme."
    )

    parser.add_argument("--ra", type=float, required=True, help="Ascensão reta central (graus)")
    parser.add_argument("--dec", type=float, required=True, help="Declinação central (graus)")
    parser.add_argument("--raio", type=float, default=0.02, help="Raio de busca (graus)")
    parser.add_argument("--pixels", type=int, default=2000, help="Resolução da imagem em pixels")
    parser.add_argument("--sobreposicao", type=float, default=0.1, help="Sobreposição entre imagens (0–0.2)")
    parser.add_argument("--data_hora", type=str, default="2025-10-04 11:31:02", help="Data e hora (UTC) opcional")

#    parser.add_argument("--data_hora", type=str, default="2025-10-04 11:31:02", help="Data e hora (UTC) opcional")

    args = parser.parse_args()

    survey_list = ['DSS2 Red', 'DSS2 Blue', 'DSS2 IR']

    gerar_grade_imagens_threaded(
        ra_centro=args.ra,
        dec_centro=args.dec,
        data_hora=args.data_hora,
        survey_list=survey_list,
        raio=args.raio,
        pixels=args.pixels,
        sobreposicao=args.sobreposicao,
    )

