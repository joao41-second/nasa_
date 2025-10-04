

from astroquery.skyview import SkyView
from astropy import units as u
from astropy.coordinates import SkyCoord
from astropy.time import Time
import matplotlib.pyplot as plt
from astropy.visualization import ImageNormalize, MinMaxInterval, SqrtStretch
from datetime import datetime

def buscar_imagem_com_tempo(ra_graus, dec_graus, data_hora=None, survey='DSS', 
                             raio=0.5, pixels=1000, nome_arquivo='imagem_espacial.png'):
    """
    Busca imagem do espaço usando coordenadas em graus e timestamp
    
    Args:
        ra_graus: float - Ascensão Reta em graus (0-360)
        dec_graus: float - Declinação em graus (-90 a +90)
        data_hora: str ou datetime - Data/hora da observação (opcional)
                   Formato: 'YYYY-MM-DD HH:MM:SS' ou objeto datetime
        survey: str - Survey desejado
        raio: float - Raio em graus
        pixels: int - Resolução da imagem
        nome_arquivo: str - Nome do arquivo de saída
    """
    
    # Processar timestamp
    if data_hora is None:
        # Usar tempo atual (UTC)
        tempo = Time.now()
        data_hora_str = datetime.utcnow().strftime('%Y-%m-%d %H:%M:%S')
    elif isinstance(data_hora, str):
        tempo = Time(data_hora)
        data_hora_str = data_hora
    else:
        tempo = Time(data_hora)
        data_hora_str = data_hora.strftime('%Y-%m-%d %H:%M:%S')
    
    # Criar coordenadas
    coords = SkyCoord(
        ra=ra_graus*u.degree, 
        dec=dec_graus*u.degree, 
        frame='icrs',
        obstime=tempo  # Tempo da observação
    )
    
    # Informações
    print("="*60)
    print("🌌 BUSCANDO IMAGEM DO ESPAÇO")
    print("="*60)
    print(f"📍 Coordenadas:")
    print(f"   RA (Ascensão Reta):  {ra_graus:.6f}° = {coords.ra.to_string(unit=u.hour, precision=2)}")
    print(f"   Dec (Declinação):    {dec_graus:.6f}° = {coords.dec.to_string(unit=u.degree, precision=2)}")
    print(f"   Frame:               {coords.frame.name}")
    print(f"\n🕐 Tempo:")
    print(f"   Data/Hora (UTC):     {data_hora_str}")
    print(f"   Julian Date:         {tempo.jd:.6f}")
    print(f"   MJD:                 {tempo.mjd:.6f}")
    print(f"\n⚙️  Parâmetros:")
    print(f"   Survey:              {survey}")
    print(f"   Raio:                {raio}°")
    print(f"   Resolução:           {pixels}x{pixels} pixels")
    print(f"   Escala:              {(raio*3600)/pixels:.2f} arcsec/pixel")
    print("="*60)
    
    try:
        # Buscar imagem
        print("\n⏳ Baixando imagem do SkyView...")
        images = SkyView.get_images(
            position=coords,
            survey=survey,
            radius=raio*u.degree,
            pixels=pixels,
            coordinates='J2000'  # Época padrão
        )
        
        # Processar dados
        image_data = images[0][0].data
        header = images[0][0].header
        
        print(f"✅ Imagem baixada com sucesso!")
        print(f"   Dimensões: {image_data.shape}")
        print(f"   Min/Max valores: {image_data.min():.2e} / {image_data.max():.2e}")
        
        # Normalizar
        norm = ImageNormalize(
            image_data,
            interval=MinMaxInterval(),
            stretch=SqrtStretch()
        )
        normalized_data = norm(image_data)
        
        # Criar visualização
        fig = plt.figure(figsize=(12, 10), facecolor='black')
        ax = plt.subplot(111)
        
        im = ax.imshow(normalized_data, cmap='gray', origin='lower')
        ax.axis('off')
        
        # Adicionar informações na imagem
        info_text = f'RA: {ra_graus:.4f}°  Dec: {dec_graus:.4f}°\n'
        info_text += f'Survey: {survey}  |  {data_hora_str} UTC'
        
        fig.text(0.5, 0.02, info_text, 
                ha='center', color='white', fontsize=10,
                bbox=dict(boxstyle='round', facecolor='black', alpha=0.7))
        
        plt.tight_layout()
        plt.savefig(nome_arquivo, dpi=150, bbox_inches='tight', 
                   facecolor='black', edgecolor='none')
        plt.close()
        
        print(f"\n💾 Arquivo salvo: {nome_arquivo}")
        print("="*60)
        
        return {
            'arquivo': nome_arquivo,
            'coordenadas': coords,
            'tempo': tempo,
            'dimensoes': image_data.shape,
            'header': header
        }
        
    except Exception as e:
        print(f"\n❌ Erro ao buscar imagem: {e}")
        return None




if __name__ == "__main__":

    print("\n\n📸 EXEMPLO 2: Imagem com tempo atual\n")
    resultado2 = buscar_imagem_com_tempo(
        ra_graus=10.6847,         # M31 (Andrômeda)
        dec_graus=41.2689,
        data_hora="2025-10-04 11:31:02",           # None = tempo atual
        survey='DSS2 Red',
        raio=0.1,
        pixels=2000,
        nome_arquivo='imagem_tempo_atual.png'
    )
