
from astropy.coordinates import SkyCoord
from astropy.time import Time
from astroquery.skyview import SkyView
from astropy.visualization import ImageNormalize, MinMaxInterval, SqrtStretch
import astropy.units as u
import matplotlib.pyplot as plt
import numpy as np
from datetime import datetime

def buscar_imagem_colorida(ra_graus, dec_graus, data_hora=None, survey_list=None, 
                           raio=0.5, pixels=1000, nome_arquivo='imagem_espacial_colorida.png'):
    """
    Busca imagem colorida do espaço combinando filtros RGB do DSS2 e balanceando cores.
    """
    if survey_list is None:
        survey_list = ['DSS2 Red', 'DSS2 Blue', 'DSS2 IR']  # padrão RGB

    # Processar timestamp
    if data_hora is None:
        tempo = Time.now()
        data_hora_str = datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')
    elif isinstance(data_hora, str):
        tempo = Time(data_hora)
        data_hora_str = data_hora
    else:
        tempo = Time(data_hora)
        data_hora_str = data_hora.strftime('%Y-%m-%d %H:%M:%S')
    
    coords = SkyCoord(ra=ra_graus*u.degree, dec=dec_graus*u.degree, frame='icrs', obstime=tempo)
    
    print(f"\n🌌 Buscando imagem colorida para RA={ra_graus}°, Dec={dec_graus}°...")
    
    rgb_data = []
    for survey in survey_list:
        try:
            images = SkyView.get_images(position=coords, survey=survey, radius=raio*u.degree, pixels=pixels)
            if images and len(images) > 0:
                data = images[0][0].data
                norm = ImageNormalize(data, interval=MinMaxInterval(), stretch=SqrtStretch())
                rgb_data.append(norm(data))
                print(f"✅ {survey} baixado")
            else:
                raise ValueError(f"Nenhuma imagem disponível para {survey}")
        except Exception as e:
            print(f"❌ Erro ao baixar {survey}: {e}")
            rgb_data.append(np.zeros((pixels, pixels)))  # fallback
    
    # Garantir que temos exatamente 3 canais
    while len(rgb_data) < 3:
        rgb_data.append(np.zeros((pixels, pixels)))
    
    # ===================================
    # Combinar em RGB com balanceamento
    # ===================================
    rgb_image = np.zeros((pixels, pixels, 3))
    # Vermelho = Red
    rgb_image[:, :, 0] = rgb_data[0]
    # Verde = média de Blue + IR (evita verde dominante)
    rgb_image[:, :, 1] = (rgb_data[1] + rgb_data[2]) / 2
    # Azul = Blue
    rgb_image[:, :, 2] = rgb_data[1]

    # Normalizar cada canal
    for i in range(3):
        channel = rgb_image[:, :, i]
        channel = (channel - channel.min()) / (channel.max() - channel.min() + 1e-8)
        rgb_image[:, :, i] = channel

    # Exibir e salvar
    fig = plt.figure(figsize=(12, 10), facecolor='black')
    ax = plt.subplot(111)
    ax.imshow(rgb_image, origin='lower')
    ax.axis('off')
    
    info_text = f'RA: {ra_graus:.4f}°  Dec: {dec_graus:.4f}°\n'
    info_text += f'Survey: {", ".join(survey_list)}  |  {data_hora_str} UTC'
    fig.text(0.5, 0.02, info_text, ha='center', color='white', fontsize=10,
             bbox=dict(boxstyle='round', facecolor='black', alpha=0.7))
    
    plt.tight_layout()
    plt.savefig(nome_arquivo, dpi=150, bbox_inches='tight', facecolor='black', edgecolor='none')
    plt.close()
    
    print(f"💾 Arquivo salvo: {nome_arquivo}")
    
    return {
        'arquivo': nome_arquivo,
        'coordenadas': coords,
        'tempo': tempo,
        'dimensoes': rgb_image.shape
    }

# Exemplo de uso
if __name__ == "__main__":
        resultado = buscar_imagem_colorida(
        ra_graus=189.2291667,  # Hubble Deep Field (North)
        dec_graus=62.2375,
        data_hora="2025-10-04 11:31:02",
        survey_list=['DSS2 Red', 'DSS2 Blue', 'DSS2 IR'],
        raio=0.02,
        pixels=2000,
        nome_arquivo='hubble_deep_field_colorido.png'
    )
