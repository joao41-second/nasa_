from astroquery.skyview import SkyView
from astropy import units as u
import matplotlib.pyplot as plt
from astropy.visualization import MinMaxInterval, SqrtStretch, ImageNormalize

# Suas coordenadas
coordenadas = '15h12m30s +41d12m00s'

# Buscar imagem
images = SkyView.get_images(
    position=coordenadas,
    survey='DSS',
    radius=0.5*u.degree,
    pixels=1000
)

# Pegar os dados da imagem
image_data = images[0][0].data

# Normalizar para melhor visualização
norm = ImageNormalize(image_data, interval=MinMaxInterval(), stretch=SqrtStretch())
normalized_data = norm(image_data)

# Salvar como PNG
plt.figure(figsize=(10, 10))
plt.imshow(normalized_data, cmap='gray', origin='lower')
plt.axis('off')
plt.tight_layout(pad=0)
plt.savefig('imagem_espacial.png', dpi=150, bbox_inches='tight')
plt.close()

print("✅ Imagem convertida e salva como: imagem_espacial.png")
