
#include <cstddef>
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
#include <iostream>

#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QProcess>
#include <QStringList>
#include <iostream>
#include <sys/wait.h>
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

void abrirFormulario(QWidget* parent = nullptr) {
    QWidget* formWindow = new QWidget(parent, Qt::FramelessWindowHint);
    formWindow->setAttribute(Qt::WA_TranslucentBackground);
    formWindow->setWindowTitle("new map");
    formWindow->setFixedSize(400, 400);

    QVBoxLayout* mainLayout = new QVBoxLayout();

    QWidget* container = new QWidget();
    container->setStyleSheet("background-color: rgba(230, 30, 30, 230); border-radius: 10px;");
    QVBoxLayout* containerLayout = new QVBoxLayout();

    QFormLayout* formLayout = new QFormLayout();

    QDoubleSpinBox* raBox = new QDoubleSpinBox();
    raBox->setRange(0.0, 360.0);
    raBox->setDecimals(6);
    raBox->setValue(10.6847);
    formLayout->addRow("RA (graus):", raBox);

    QDoubleSpinBox* decBox = new QDoubleSpinBox();
    decBox->setRange(-90.0, 90.0);
    decBox->setDecimals(6);
    decBox->setValue(41.2689);
    formLayout->addRow("DEC (graus):", decBox);

    QLineEdit* dateTimeEdit = new QLineEdit("2025-10-04 11:31:02");
    formLayout->addRow("Data/Hora:", dateTimeEdit);

    QDoubleSpinBox* raioBox = new QDoubleSpinBox();
    raioBox->setRange(0.0, 10.0);
    raioBox->setDecimals(4);
    raioBox->setValue(0.02);
    formLayout->addRow("Raio:", raioBox);

    QSpinBox* pixelsBox = new QSpinBox();
    pixelsBox->setRange(1, 10000);
    pixelsBox->setValue(2000);
    formLayout->addRow("Pixels:", pixelsBox);

    containerLayout->addLayout(formLayout);

    QPushButton* btnOk = new QPushButton("Confirmar");
    containerLayout->addWidget(btnOk);

    QObject::connect(btnOk, &QPushButton::clicked, [=]() {
        // Captura os valores
        double ra = raBox->value();
        double dec = decBox->value();
        QString dataHora = dateTimeEdit->text();
        double raio = raioBox->value();
        int pixels = pixelsBox->value();

        // Monta os argumentos do Python
        QStringList args;
        args << "./srcs/get_img_text.py"
             << "--ra" << QString::number(ra)
             << "--dec" << QString::number(dec)
             << "--raio" << QString::number(raio)
             << "--pixels" << QString::number(pixels)
             << "--data_hora" << dataHora;

        // Cria o processo
        QProcess* process = new QProcess(parent);

        // Quando o script terminar:
        QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                         [=](int exitCode, QProcess::ExitStatus status) {
            QByteArray output = process->readAllStandardOutput();
            QByteArray error = process->readAllStandardError();

            if (status == QProcess::NormalExit && exitCode == 0) {
                imagem_original = get_img("./img"); // Atualiza imagem
            } else {
            }

            process->deleteLater();
        });

        // Inicia o script Python (em background)
        process->start("python3", args);

        // Fecha o formulário imediatamente
        formWindow->close();
    });

    container->setLayout(containerLayout);
    mainLayout->addWidget(container);
    formWindow->setLayout(mainLayout);

    formWindow->show();
}


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

