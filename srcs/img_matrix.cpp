#include <algorithm>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <string>
#include <iostream>
#include <filesystem>
#include <vector>
#include <ostream>
#include <string>
#include <iostream>
#include <vector>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>

using namespace cv;
using namespace std;

std::vector<std::string> listarArquivos(const std::string& caminhoPasta) {
    std::vector<std::string> arquivos;
    
    DIR *dir = opendir(caminhoPasta.c_str());
    if (dir == nullptr) {
        std::cout << "Erro: Não foi possível abrir a pasta '" << caminhoPasta << "'" << std::endl;
        std::cout << "Verifique se o caminho está correto e se você tem permissão de leitura." << std::endl;
        return arquivos;
    }
    
    struct dirent *entrada;
    std::cout << "Listando arquivos da pasta: " << caminhoPasta << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    while ((entrada = readdir(dir)) != NULL) {
        // Ignora os diretórios especiais . e ..
        if (strcmp(entrada->d_name, ".") == 0 || strcmp(entrada->d_name, "..") == 0) {
            continue;
        }
        
        std::string nomeArquivo = entrada->d_name;
        std::string caminhoCompleto = caminhoPasta + "/" + nomeArquivo;
	std::cout <<  caminhoCompleto << std::endl;
        

           arquivos.push_back(caminhoCompleto);
        
    }
    
    closedir(dir);
    return arquivos;
}


cv::Mat get_img(std::string caminhoPasta, int crop = 20)
{
    std::vector<std::string> arquivos = listarArquivos(caminhoPasta);
    std::vector<cv::Mat> imgs;

    // Carregar imagens
    for (auto &path : arquivos) {
        cv::Mat temp = cv::imread(path, cv::IMREAD_COLOR);
        if (!temp.empty()) imgs.push_back(temp);
    }

    if (imgs.empty()) {
        std::cerr << "Nenhuma imagem carregada!" << std::endl;
        return cv::Mat();
    }

    // Recorta bordas pretas (crop)
    for (auto &im : imgs) {
        if (crop > 0 && im.cols > 2 * crop && im.rows > 2 * crop) {
            cv::Rect roi(crop, crop, im.cols - 2 * crop, im.rows - 2 * crop);
            im = im(roi).clone();
        }
    }

    // Redimensiona todas pro mesmo tamanho
    int w = imgs[0].cols;
    int h = imgs[0].rows;
    for (auto &im : imgs)
        cv::resize(im, im, cv::Size(w, h));

    int cols = 3;
    int rows = 3;

    // Cria imagem final
    cv::Mat combined(h * rows, w * cols, imgs[0].type(), cv::Scalar::all(0));

    // Copia cada imagem pro lugar certo
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            if (idx >= imgs.size()) break;

            cv::Rect roi(j * w, i * h, w, h);
            imgs[idx].copyTo(combined(roi));
        }
    }

    return combined;
}






