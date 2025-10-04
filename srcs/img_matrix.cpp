
#include <opencv2/opencv.hpp>
#include <dirent.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>

using namespace cv;
using namespace std;

// Lista e ordena os ficheiros dentro da pasta
std::vector<std::string> listarArquivos(const std::string& caminhoPasta) {
    std::vector<std::string> arquivos;
    DIR *dir = opendir(caminhoPasta.c_str());
    if (!dir) {
        cerr << "Erro: não foi possível abrir a pasta '" << caminhoPasta << "'" << endl;
        return arquivos;
    }

    struct dirent *entrada;
    while ((entrada = readdir(dir)) != nullptr) {
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0)
            continue;
        arquivos.push_back(caminhoPasta + "/" + entrada->d_name);
    }
    closedir(dir);

    // Ordena alfabeticamente (img_1.png, img_2.png, …)
    std::sort(arquivos.begin(), arquivos.end());
    return arquivos;
}

// Junta as imagens numa grelha 3x3 (centro = img_5)
cv::Mat get_img(const std::string caminhoPasta, int crop = 0) {
    vector<string> arquivos = listarArquivos(caminhoPasta);
    vector<cv::Mat> imgs;

    for (auto &path : arquivos) {
        Mat temp = imread(path, IMREAD_COLOR);
        if (!temp.empty())
            imgs.push_back(temp);
    }
    if (imgs.empty()) {
        cerr << "Nenhuma imagem carregada!" << endl;
        return Mat();
    }

    // Recorta bordas pretas
    for (auto &im : imgs) {
        if (crop > 0 && im.cols > 2 * crop && im.rows > 2 * crop) {
            Rect roi(crop, crop, im.cols - 2 * crop, im.rows - 2 * crop);
            im = im(roi).clone();
        }
    }

    // Redimensiona todas para o mesmo tamanho
    int w = imgs[0].cols;
    int h = imgs[0].rows;
    for (auto &im : imgs)
        resize(im, im, Size(w, h));

    const int rows = 3, cols = 3;
    Mat combined(h * rows, w * cols, imgs[0].type(), Scalar::all(0));

    // Mapa de índices correto (0-based)
    int index_map[3][3] = {
        {8, 7, 6},
        {5, 4, 3},
        {2, 1, 0}
    };

    // Copia cada imagem para a posição correspondente
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            int idx = index_map[i][j];
            if (idx >= static_cast<int>(imgs.size())) continue;
            Rect roi(j * w, i * h, w, h);
            imgs[idx].copyTo(combined(roi));
        }
    }

    return combined;
}

