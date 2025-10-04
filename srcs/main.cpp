#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// Variáveis globais
Mat imagem_original;
Mat imagem_exibida;
string nome_janela = "🔍 Zoom Interativo - OpenCV";
int zoom_slider = 100;  // Valor inicial: 100% (sem zoom)
int zoom_min = 10;      // Zoom mínimo: 10%
int zoom_max = 500;     // Zoom máximo: 500%

// Ponto central para zoom (será o centro da imagem)
Point2f centro_zoom;

// Callback para o slider de zoom
void onZoomChange(int valor, void* userData) {
    if (imagem_original.empty()) return;
    
    // Calcular o fator de zoom (10% a 500%)
    double fator_zoom = valor / 100.0;
    
    // Calcular novo tamanho
    int nova_largura = static_cast<int>(imagem_original.cols * fator_zoom);
    int nova_altura = static_cast<int>(imagem_original.rows * fator_zoom);
    
    // Redimensionar a imagem
    Mat imagem_zoom;
    resize(imagem_original, imagem_zoom, Size(nova_largura, nova_altura), 0, 0, INTER_LINEAR);
    
    // Obter tamanho da janela
    Size tamanho_janela = imagem_original.size();
    
    // Calcular região de interesse (ROI) centrada
    int x_inicio = max(0, (nova_largura - tamanho_janela.width) / 2);
    int y_inicio = max(0, (nova_altura - tamanho_janela.height) / 2);
    int x_fim = min(nova_largura, x_inicio + tamanho_janela.width);
    int y_fim = min(nova_altura, y_inicio + tamanho_janela.height);
    
    // Se a imagem com zoom for menor que a janela, centralizar
    if (nova_largura < tamanho_janela.width || nova_altura < tamanho_janela.height) {
        imagem_exibida = Mat::zeros(tamanho_janela, imagem_original.type());
        int x_offset = (tamanho_janela.width - nova_largura) / 2;
        int y_offset = (tamanho_janela.height - nova_altura) / 2;
        
        Rect roi(x_offset, y_offset, nova_largura, nova_altura);
        imagem_zoom.copyTo(imagem_exibida(roi));
    } else {
        // Extrair região central
        Rect roi(x_inicio, y_inicio, x_fim - x_inicio, y_fim - y_inicio);
        imagem_exibida = imagem_zoom(roi).clone();
    }
    
    // Adicionar informações na imagem
    string texto = "Zoom: " + to_string(valor) + "%";
    putText(imagem_exibida, texto, Point(10, 30), 
            FONT_HERSHEY_SIMPLEX, 1.0, Scalar(0, 255, 0), 2);
    
    // Adicionar instruções
    string instrucoes = "Mouse Wheel: Zoom | ESC: Sair | F: Tela Cheia | R: Reset";
    putText(imagem_exibida, instrucoes, Point(10, imagem_exibida.rows - 10), 
            FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 255), 1);
    
    // Exibir a imagem
    imshow(nome_janela, imagem_exibida);
    
    // Debug
    cout << "Zoom: " << valor << "% | Tamanho: " << nova_largura << "x" << nova_altura << endl;
}

// Callback para eventos do mouse
void onMouse(int event, int x, int y, int flags, void* userdata) {
    // Zoom com a roda do mouse
    if (event == EVENT_MOUSEWHEEL) {
        int delta = getMouseWheelDelta(flags);
        
        // Ajustar zoom
        if (delta > 0) {
            zoom_slider = min(zoom_max, zoom_slider + 10);  // Zoom in
        } else {
            zoom_slider = max(zoom_min, zoom_slider - 10);  // Zoom out
        }
        
        // Atualizar slider
        setTrackbarPos("Zoom (%)", nome_janela, zoom_slider);
    }
    
    // Clique para definir centro do zoom (funcionalidade futura)
    if (event == EVENT_LBUTTONDOWN) {
        centro_zoom = Point2f(x, y);
        cout << "Centro de zoom definido em: (" << x << ", " << y << ")" << endl;
    }
}

int main(int argc, char** argv) {
    // Verificar argumentos
    string caminho_imagem;
    
    if (argc > 1) {
        caminho_imagem = argv[1];
    } else {
        // Usar imagem padrão (ajuste o caminho!)
        caminho_imagem = "imagem_tempo_atual.png";  // ← MUDE AQUI
        cout << "⚠️  Uso: " << argv[0] << " <caminho_da_imagem>" << endl;
        cout << "   Usando imagem padrão: " << caminho_imagem << endl << endl;
    }
    
    // Carregar a imagem
    imagem_original = imread(caminho_imagem, IMREAD_COLOR);
    
    if (imagem_original.empty()) {
        cerr << "❌ Erro: Não foi possível carregar a imagem: " << caminho_imagem << endl;
        cerr << "   Verifique se o caminho está correto!" << endl;
        return -1;
    }
    
    cout << "✅ Imagem carregada com sucesso!" << endl;
    cout << "   Dimensões: " << imagem_original.cols << "x" << imagem_original.rows << endl;
    cout << "   Canais: " << imagem_original.channels() << endl << endl;
    
    // Definir centro do zoom como centro da imagem
    centro_zoom = Point2f(imagem_original.cols / 2.0f, imagem_original.rows / 2.0f);
    
    // Criar janela
    namedWindow(nome_janela, WINDOW_NORMAL);
    
    // Definir tamanho inicial da janela
    resizeWindow(nome_janela, imagem_original.cols, imagem_original.rows);
    
    // Criar trackbar (slider)
    createTrackbar("Zoom (%)", nome_janela, &zoom_slider, zoom_max, onZoomChange);
    
    // Configurar callback do mouse
    setMouseCallback(nome_janela, onMouse, nullptr);
    
    // Exibir instruções
    cout << "🎮 CONTROLES:" << endl;
    cout << "   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl;
    cout << "   🖱️  Roda do Mouse    → Zoom In/Out rápido" << endl;
    cout << "   📊 Slider            → Ajuste preciso do zoom" << endl;
    cout << "   🖱️  Clique Esquerdo  → Definir centro do zoom" << endl;
    cout << "   ⌨️  Tecla 'F'        → Alternar tela cheia" << endl;
    cout << "   ⌨️  Tecla 'R'        → Reset (100%)" << endl;
    cout << "   ⌨️  Tecla '+'        → Zoom In" << endl;
    cout << "   ⌨️  Tecla '-'        → Zoom Out" << endl;
    cout << "   ⌨️  Tecla 'ESC'      → Sair" << endl;
    cout << "   ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << endl << endl;
    
    // Exibir imagem inicial
    onZoomChange(zoom_slider, nullptr);
    
    // Loop principal
    bool tela_cheia = false;
    
    while (true) {
        int tecla = waitKey(10);
        
        if (tecla == 27) {  // ESC
            cout << "👋 Saindo..." << endl;
            break;
        }
        else if (tecla == 'f' || tecla == 'F') {  // Tela cheia
            tela_cheia = !tela_cheia;
            if (tela_cheia) {
                setWindowProperty(nome_janela, WND_PROP_FULLSCREEN, WINDOW_FULLSCREEN);
                cout << "🖥️  Modo tela cheia ATIVADO" << endl;
            } else {
                setWindowProperty(nome_janela, WND_PROP_FULLSCREEN, WINDOW_NORMAL);
                cout << "🖥️  Modo tela cheia DESATIVADO" << endl;
            }
        }
        else if (tecla == 'r' || tecla == 'R') {  // Reset
            zoom_slider = 100;
            setTrackbarPos("Zoom (%)", nome_janela, zoom_slider);
            cout << "🔄 Zoom resetado para 100%" << endl;
        }
        else if (tecla == '+' || tecla == '=') {  // Zoom in
            zoom_slider = min(zoom_max, zoom_slider + 10);
            setTrackbarPos("Zoom (%)", nome_janela, zoom_slider);
        }
        else if (tecla == '-' || tecla == '_') {  // Zoom out
            zoom_slider = max(zoom_min, zoom_slider - 10);
            setTrackbarPos("Zoom (%)", nome_janela, zoom_slider);
        }
    }
    
    // Liberar recursos
    destroyAllWindows();
    
    return 0;
}
