#include <opencv2/opencv.hpp>
#include <QApplication>
#include <QPushButton>
#include <QSlider>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QImage>
#include <QMouseEvent>
#include <iostream>
#include "but.hpp"

using namespace cv;
using namespace std;

// ================= VARIÁVEIS GLOBAIS =====================
Mat imagem_original;
double zoom_factor = 1.0;
Point2f centro_zoom;
QLabel* label_imagem = nullptr; 

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

// ================= FUNÇÃO PARA ATUALIZAR A IMAGEM =====================
void atualizarImagem() {
    if (imagem_original.empty()) return;

    // Limitar zoom mínimo para caber na janela
    double zoom_x = static_cast<double>(WINDOW_WIDTH) / imagem_original.cols;
    double zoom_y = static_cast<double>(WINDOW_HEIGHT) / imagem_original.rows;
    double zoom_min = min(zoom_x, zoom_y);
    if (zoom_factor < zoom_min) zoom_factor = zoom_min;

    int nova_largura = static_cast<int>(imagem_original.cols * zoom_factor);
    int nova_altura  = static_cast<int>(imagem_original.rows * zoom_factor);

    Mat imagem_zoom;
    resize(imagem_original, imagem_zoom, Size(nova_largura, nova_altura), 0, 0, INTER_LINEAR);

    // Cortar para centralizar o zoom
    Point2f centro_na_zoom = centro_zoom * zoom_factor;

    int x_inicio = static_cast<int>(centro_na_zoom.x - WINDOW_WIDTH / 2);
    int y_inicio = static_cast<int>(centro_na_zoom.y - WINDOW_HEIGHT / 2);

    // Garantir que ROI esteja dentro da imagem
    x_inicio = max(0, min(x_inicio, nova_largura - WINDOW_WIDTH));
    y_inicio = max(0, min(y_inicio, nova_altura - WINDOW_HEIGHT));

    Rect roi(x_inicio, y_inicio, min(WINDOW_WIDTH, nova_largura), min(WINDOW_HEIGHT, nova_altura));
    Mat imagem_crop = imagem_zoom(roi).clone();

    // Converter Mat para QImage
    Mat imagem_rgb;
    cvtColor(imagem_crop, imagem_rgb, COLOR_BGR2RGB);
    QImage qimg(imagem_rgb.data, imagem_rgb.cols, imagem_rgb.rows, static_cast<int>(imagem_rgb.step), QImage::Format_RGB888);
    label_imagem->setPixmap(QPixmap::fromImage(qimg));
}

// ================= CALLBACKS ============================
void onSliderChange(int value) {
    zoom_factor = value / 100.0;
    atualizarImagem();
}

void botaoCallback() {
    cout << "Botão clicado!" << endl;
}

// ================= LABEL PERSONALIZADO PARA MOUSE =====================
class ImageLabel : public QLabel {
public:
    ImageLabel(QWidget* parent = nullptr) : QLabel(parent) {}

protected:
    void mouseMoveEvent(QMouseEvent* event) override {
        // Ajustar centro do zoom baseado no mouse
        centro_zoom = Point2f(event->x() + centro_zoom.x - WINDOW_WIDTH/2,
                              event->y() + centro_zoom.y - WINDOW_HEIGHT/2);
        atualizarImagem();
    }
};

// ================= MAIN ================================
int main(int argc, char** argv) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Visualizador com Zoom - OpenCV + Qt5");
    window.setFixedSize(WINDOW_WIDTH + 50, WINDOW_HEIGHT + 150); // tamanho fixo para controles

    QVBoxLayout* layout = new QVBoxLayout();

    // QLabel para imagem com mouse interativo
    label_imagem = new ImageLabel();
    label_imagem->setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    label_imagem->setMouseTracking(true);
    layout->addWidget(label_imagem);

    // Botão
    QPushButton* button = new QPushButton("Clique Aqui");
    layout->addWidget(button);
    QObject::connect(button, &QPushButton::clicked, botaoCallback);

    // Slider de zoom
    QSlider* slider = new QSlider(Qt::Horizontal);
    slider->setRange(10, 500); // 10% a 500%
    slider->setValue(100);
    layout->addWidget(slider);
    QObject::connect(slider, &QSlider::valueChanged, onSliderChange);

    window.setLayout(layout);
    window.show();

    // ================== Carregar imagem fixa ==================
    imagem_original = get_img("./img"); // sua função de carregamento
    if (imagem_original.empty()) {
        cerr << "Não foi possível carregar a imagem." << endl;
        return -1;
    }

    centro_zoom = Point2f(imagem_original.cols / 2.0f, imagem_original.rows / 2.0f);

    atualizarImagem(); // exibir imagem inicial

    return app.exec();
}
