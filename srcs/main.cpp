
#include <opencv2/opencv.hpp>
#include <QApplication>
#include <QPushButton>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QImage>
#include <QMouseEvent>
#include <QWheelEvent>
#include <iostream>
#include "but.hpp" // sua função abrirFormulario e get_img

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

    int nova_largura = static_cast<int>(imagem_original.cols * zoom_factor);
    int nova_altura  = static_cast<int>(imagem_original.rows * zoom_factor);

    Mat imagem_zoom;
    resize(imagem_original, imagem_zoom, Size(nova_largura, nova_altura), 0, 0, INTER_LINEAR);

    Point2f centro_na_zoom = centro_zoom * zoom_factor;

    int x_inicio = static_cast<int>(centro_na_zoom.x - WINDOW_WIDTH / 2);
    int y_inicio = static_cast<int>(centro_na_zoom.y - WINDOW_HEIGHT / 2);

    x_inicio = max(0, min(x_inicio, nova_largura - WINDOW_WIDTH));
    y_inicio = max(0, min(y_inicio, nova_altura - WINDOW_HEIGHT));

    Rect roi(x_inicio, y_inicio, min(WINDOW_WIDTH, nova_largura), min(WINDOW_HEIGHT, nova_altura));
    Mat imagem_crop = imagem_zoom(roi).clone();

    Mat imagem_rgb;
    cvtColor(imagem_crop, imagem_rgb, COLOR_BGR2RGB);
    QImage qimg(imagem_rgb.data, imagem_rgb.cols, imagem_rgb.rows, static_cast<int>(imagem_rgb.step), QImage::Format_RGB888);
    label_imagem->setPixmap(QPixmap::fromImage(qimg));

    imagem_original = get_img("./img"); // sua função de carregamento
}

// ================= FUNÇÃO PARA LIMITAR O CENTRO DO ZOOM =====================
void limitarCentroZoom() {
    if (imagem_original.empty()) return;

    float halfWidth = WINDOW_WIDTH / (2.0f * zoom_factor);
    float halfHeight = WINDOW_HEIGHT / (2.0f * zoom_factor);

    centro_zoom.x = std::max(halfWidth, std::min(centro_zoom.x, static_cast<float>(imagem_original.cols) - halfWidth));
    centro_zoom.y = std::max(halfHeight, std::min(centro_zoom.y, static_cast<float>(imagem_original.rows) - halfHeight));
}

// ================= LABEL PERSONALIZADO PARA MOUSE E ZOOM =====================
class ImageLabel : public QLabel {
public:
    ImageLabel(QWidget* parent = nullptr) : QLabel(parent) {
        setMouseTracking(true);
    }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            dragging = true;
            lastMousePos = Point2f(event->x(), event->y());
        }
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (dragging) {
            Point2f currentPos(event->x(), event->y());
            Point2f delta = lastMousePos - currentPos;

            centro_zoom += delta / zoom_factor; // ajustar pelo zoom
            limitarCentroZoom();

            lastMousePos = currentPos;
            atualizarImagem();
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            dragging = false;
        }
    }

    void wheelEvent(QWheelEvent* event) override {
        if (imagem_original.empty()) return;

        // Zoom simples pelo scroll
        double delta = event->angleDelta().y() / 120.0;
        double zoom_step = 1.1;
        zoom_factor *= pow(zoom_step, delta);

        // Limitar zoom mínimo para caber na tela
        double zoom_x = static_cast<double>(WINDOW_WIDTH) / imagem_original.cols;
        double zoom_y = static_cast<double>(WINDOW_HEIGHT) / imagem_original.rows;
        double zoom_min = min(zoom_x, zoom_y);
        if (zoom_factor < zoom_min) zoom_factor = zoom_min;

        limitarCentroZoom();
        atualizarImagem();
    }

private:
    Point2f lastMousePos;
    bool dragging = false;
};

// ================= MAIN ================================
int main(int argc, char** argv) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Universel space map");
    window.setFixedSize(WINDOW_WIDTH + 50, WINDOW_HEIGHT + 150);

    QVBoxLayout* layout = new QVBoxLayout();

    // QLabel para imagem com mouse interativo
    label_imagem = new ImageLabel();
    label_imagem->setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    layout->addWidget(label_imagem);

    // Botão para abrir formulário
    QPushButton* openFormButton = new QPushButton("new map");
    layout->addWidget(openFormButton);
    QObject::connect(openFormButton, &QPushButton::clicked, [&]() {
        abrirFormulario(&window);
    });

    window.setLayout(layout);
    window.show();

    // ================== Carregar imagem ==================
    imagem_original = get_img("./img"); // sua função de carregamento
    if (imagem_original.empty()) {
        cerr << "Não foi possível carregar a imagem." << endl;
        return -1;
    }

    centro_zoom = Point2f(imagem_original.cols / 2.0f, imagem_original.rows / 2.0f);

    atualizarImagem(); // exibir imagem inicial

    return app.exec();
}

