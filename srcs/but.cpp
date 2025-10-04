#include <opencv2/opencv.hpp>
#include <vector>
#include <functional>
#include <iostream>

using namespace cv;
using namespace std;

// Estrutura para representar um botão
struct Button {
    string label;
    Rect region;
    Scalar normalColor;
    Scalar pressedColor;
    Scalar textColor;
    function<void()> onClick;
    bool pressed;
    bool enabled;
    
    Button(const string& lbl, const Rect& rect, const Scalar& normalClr, 
           const Scalar& pressedClr, const Scalar& txtClr, function<void()> callback)
        : label(lbl), region(rect), normalColor(normalClr), pressedColor(pressedClr),
          textColor(txtClr), onClick(callback), pressed(false), enabled(true) {}
};

class ButtonManager {
private:
    vector<Button> buttons;
    Mat canvas;
    string windowName;
    Scalar backgroundColor;
    
public:
    ButtonManager(const string& winName, int width, int height) 
        : windowName(winName), backgroundColor(Scalar(60, 60, 80)) {
        canvas = Mat::zeros(height, width, CV_8UC3);
    }
    
    void setBackgroundColor(const Scalar& color) {
        backgroundColor = color;
    }
    
    void addButton(const string& label, const Rect& region, const Scalar& normalColor, 
                  function<void()> callback) {
        // Cores padrão para botão pressionado e texto
        Scalar pressedColor = normalColor * 0.6;
        Scalar textColor = Scalar(255, 255, 255);
        buttons.emplace_back(label, region, normalColor, pressedColor, textColor, callback);
    }
    
    void addButton(const string& label, const Rect& region, const Scalar& normalColor,
                  const Scalar& pressedColor, const Scalar& textColor, function<void()> callback) {
        buttons.emplace_back(label, region, normalColor, pressedColor, textColor, callback);
    }
    
    void addButton(const string& label, const Point& position, const Size& size,
                  const Scalar& normalColor, function<void()> callback) {
        Rect region(position, size);
        addButton(label, region, normalColor, callback);
    }
    
    void enableButton(const string& label, bool enable) {
        for (auto& button : buttons) {
            if (button.label == label) {
                button.enabled = enable;
                break;
            }
        }
    }
    
    void drawButtons() {
        // Fundo com gradiente para melhor aparência
        for (int i = 0; i < canvas.rows; i++) {
            float ratio = (float)i / canvas.rows;
            Scalar rowColor = backgroundColor * (0.8 + 0.2 * ratio);
            line(canvas, Point(0, i), Point(canvas.cols, i), rowColor);
        }
        
        // Título
        putText(canvas, "Sistema de Botoes Interativo", Point(50, 30), 
               FONT_HERSHEY_SIMPLEX, 0.8, Scalar(255, 255, 200), 2);
        
        for (auto& button : buttons) {
            if (!button.enabled) continue;
            
            // Cor baseada no estado
            Scalar fillColor = button.pressed ? button.pressedColor : button.normalColor;
            
            // Desenha o retângulo do botão com efeito 3D
            rectangle(canvas, button.region, fillColor, FILLED);
            
            // Borda com efeito de profundidade
            if (button.pressed) {
                // Borda interna para efeito pressionado
                rectangle(canvas, button.region, fillColor * 0.8, 2);
            } else {
                // Bordas claras para efeito 3D elevado
                line(canvas, button.region.tl(), button.region.tl() + Point(button.region.width, 0), 
                     Scalar(255, 255, 255), 1);
                line(canvas, button.region.tl(), button.region.tl() + Point(0, button.region.height), 
                     Scalar(255, 255, 255), 1);
                line(canvas, button.region.br() + Point(-button.region.width, 0), button.region.br(), 
                     Scalar(100, 100, 100), 1);
                line(canvas, button.region.br() + Point(0, -button.region.height), button.region.br(), 
                     Scalar(100, 100, 100), 1);
            }
            
            // Sombra suave
            Rect shadowRect = button.region + Point(2, 2);
            rectangle(canvas, shadowRect, Scalar(0, 0, 0, 100), FILLED);
            
            // Calcula posição do texto centralizado
            int baseline = 0;
            Size textSize = getTextSize(button.label, FONT_HERSHEY_DUPLEX, 0.5, 2, &baseline);
            Point textOrg(
                button.region.x + (button.region.width - textSize.width) / 2,
                button.region.y + (button.region.height + textSize.height) / 2
            );
            
            // Desenha o texto com sombra
            putText(canvas, button.label, textOrg + Point(1, 1), 
                   FONT_HERSHEY_DUPLEX, 0.5, Scalar(0, 0, 0), 2);
            putText(canvas, button.label, textOrg, 
                   FONT_HERSHEY_DUPLEX, 0.5, button.textColor, 2);
        }
        
        // Rodapé com informações
        string info = "Total de botoes: " + to_string(buttons.size()) + " | Pressione ESC para sair";
        putText(canvas, info, Point(10, canvas.rows - 10), 
               FONT_HERSHEY_SIMPLEX, 0.4, Scalar(200, 200, 200), 1);
    }
    
    void handleClick(int x, int y) {
        Point clickPoint(x, y);
        
        for (auto& button : buttons) {
            if (button.enabled && button.region.contains(clickPoint)) {
                // Feedback visual de pressionado
                button.pressed = true;
                drawButtons();
                imshow(windowName, canvas);
                
                // Pequena pausa para feedback
                waitKey(80);
                
                // Executa a função do botão
                if (button.onClick) {
                    button.onClick();
                }
                
                // Restaura estado normal
                button.pressed = false;
                drawButtons();
                imshow(windowName, canvas);
                break;
            }
        }
    }
    
    void handleMouseMove(int x, int y) {
        Point mousePoint(x, y);
        bool needsRedraw = false;
        
        // Verifica se o mouse está sobre algum botão
        for (auto& button : buttons) {
            if (!button.enabled) continue;
            
            bool wasPressed = button.pressed;
            button.pressed = button.region.contains(mousePoint);
            
            if (wasPressed != button.pressed) {
                needsRedraw = true;
            }
        }
        
        if (needsRedraw) {
            drawButtons();
            imshow(windowName, canvas);
        }
    }
    
    void show() {
        drawButtons();
        imshow(windowName, canvas);
    }
    
    Mat getCanvas() const {
        return canvas;
    }
};

// Variáveis globais para demonstração


