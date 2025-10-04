
#include <QWidget>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <QLabel>
#include <QApplication>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <iostream>
#include <vector>
#include <string>

using namespace std;


void abrirFormulario(QWidget* parent = nullptr) {
    QWidget* formWindow = new QWidget(parent, Qt::FramelessWindowHint);
    formWindow->setAttribute(Qt::WA_TranslucentBackground);
    formWindow->setWindowTitle("Formulário de Observação");
    formWindow->setFixedSize(400, 400);

    QVBoxLayout* mainLayout = new QVBoxLayout();

    // Widget container com fundo semi-transparente
    QWidget* container = new QWidget();
    container->setStyleSheet("background-color: rgba(30, 255, 30, 1); border-radius: 10px;");
    QVBoxLayout* containerLayout = new QVBoxLayout();

    QFormLayout* formLayout = new QFormLayout();

    // ==== Campos com valores padrão ====
    QDoubleSpinBox* raBox = new QDoubleSpinBox();
    raBox->setRange(0.0, 360.0);
    raBox->setDecimals(6);
    raBox->setValue(189.2291667);  // RA padrão
    formLayout->addRow("RA (graus):", raBox);

    QDoubleSpinBox* decBox = new QDoubleSpinBox();
    decBox->setRange(-90.0, 90.0);
    decBox->setDecimals(6);
    decBox->setValue(62.2375);     // DEC padrão
    formLayout->addRow("DEC (graus):", decBox);

    QLineEdit* dateTimeEdit = new QLineEdit("2025-10-04 11:31:02"); // Data/Hora padrão
    formLayout->addRow("Data/Hora:", dateTimeEdit);

    QComboBox* surveyCombo = new QComboBox();
    vector<string> surveys = {"DSS2 Red", "DSS2 Blue", "DSS2 IR"};
    for (auto& s : surveys) surveyCombo->addItem(QString::fromStdString(s));
    surveyCombo->setCurrentIndex(0);  // seleciona primeiro survey por padrão
    formLayout->addRow("Survey:", surveyCombo);

    QDoubleSpinBox* raioBox = new QDoubleSpinBox();
    raioBox->setRange(0.0, 10.0);
    raioBox->setDecimals(4);
    raioBox->setValue(0.02); // Raio padrão
    formLayout->addRow("Raio:", raioBox);

    QSpinBox* pixelsBox = new QSpinBox();
    pixelsBox->setRange(1, 10000);
    pixelsBox->setValue(2000); // Pixels padrão
    formLayout->addRow("Pixels:", pixelsBox);

    QLineEdit* fileNameEdit = new QLineEdit("hubble_deep_field_colorido.png"); // Nome do arquivo padrão
    formLayout->addRow("Nome do arquivo:", fileNameEdit);

    containerLayout->addLayout(formLayout);

    // Botão confirmar
    QPushButton* btnOk = new QPushButton("Confirmar");
    containerLayout->addWidget(btnOk);

    QObject::connect(btnOk, &QPushButton::clicked, [=]() {
        cout << "RA: " << raBox->value() << endl;
        cout << "DEC: " << decBox->value() << endl;
        cout << "Data/Hora: " << dateTimeEdit->text().toStdString() << endl;
        cout << "Survey: " << surveyCombo->currentText().toStdString() << endl;
        cout << "Raio: " << raioBox->value() << endl;
        cout << "Pixels: " << pixelsBox->value() << endl;
        cout << "Nome arquivo: " << fileNameEdit->text().toStdString() << endl;
        formWindow->close();
    });

    container->setLayout(containerLayout);
    mainLayout->addWidget(container);
    formWindow->setLayout(mainLayout);

    formWindow->show();
}

