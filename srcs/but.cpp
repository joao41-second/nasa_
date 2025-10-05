
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
using namespace std;


cv::Mat get_img(std::string caminhoPasta, int crop = 20);

Mat imagem_original;

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
        // Captura valores
        double ra = raBox->value();
        double dec = decBox->value();
        QString dataHora = dateTimeEdit->text();
        double raio = raioBox->value();
        int pixels = pixelsBox->value();

        // Monta argumentos
        QStringList args;
        args << "--ra" << QString::number(ra)
             << "--dec" << QString::number(dec)
             << "--raio" << QString::number(raio)
             << "--pixels" << QString::number(pixels)
             << "--data_hora" << dataHora;

        // Executa script Python em background
	int pid  = fork();
	if(pid == 0)
	{
        QProcess::startDetached("python3", QStringList() << "./srcs/get_img_text.py" << args);
	exit();
	}
	waitpid(pid,0);

        formWindow->close();
    });

    container->setLayout(containerLayout);
    mainLayout->addWidget(container);
    formWindow->setLayout(mainLayout);

    formWindow->show();
    imagem_original = get_img("./img"); // sua função de carregamento
}

