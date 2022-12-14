#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/*

    Gui Taglietti - Assets (Stack interpreter)
    QTCreator base classes for make GUI functional

*/

//Global vector to store the parsed instruction and be used in classes
vector<string> parsed;


//Classes
class Control{
public:
    void parseInstruction(string const &str, const char delim, vector<string> &out){
        stringstream ss(str);
        string s;
        while(getline(ss, s, delim)) {
            out.push_back(s);
        }
    }

    int hexConverter(string hex){
        return stoi(hex, 0, 16);
    }

    string lower(string x){
        for(int i = 0; i < x.length(); i++){
            x[i] = tolower(x[i]);
        }
        return x;
    }

    bool isFloat(string num){
        return num.find_first_not_of("0123456789.") == string::npos;
    }
};

class Memory : public Control{
private:
    float mem[256];
    float memValues[256];
    string hexValues[256];
    int size = -1;

public:
    Memory(){
        memset(this->mem, 0, sizeof(this->mem));
    }

    void clearMemory(){
        memset(this->mem, 0, sizeof(this->mem));
        this->size = -1;
    }

    void setMemory(string hex, float value){
        this->mem[this->hexConverter(hex)] = value;
        this->memValues[++size] = value;
        this->hexValues[size] = hex;
    }

    float getMemory(string hex){
        return this->mem[this->hexConverter(hex)];
    }

    void showMemoryChanges(QPlainTextEdit *textEdit){
        QString text = "";
        for(int i = 0; i < this->size + 1; i++){
            text += "O endere??o de mem??ria " + QString::fromStdString(this->hexValues[i]) + " recebeu o valor: " +
                    QString::number(this->memValues[i]) + "\n";
        }
        textEdit->setPlainText(text);
    }
};

class Stack : public Memory{
private:
    float stack[16];
    int size = -1;

public:
    Stack(){
        memset(this->stack, 0, sizeof(this->stack));
    }

    void clearStack(){
        this->size = -1;
        memset(this->stack, 0, sizeof(this->stack));
    }

    void checkOverflow(QWidget *parent){
        if(this->size > 14){
            QMessageBox::about(parent, "Error", "STACK OVERFLOW!");
            sleep(2000);
            throw underflow_error("STACK OVERFLOW");
        }
    }

    void checkUnderflow(QWidget *parent){
        if(this->size < 0){
            QMessageBox::about(parent, "Error", "STACK UNDERFLOW!");
            sleep(2000);
            throw underflow_error("STACK OVERFLOW");
        }
    }

    void push(float n, QWidget *parent){
        this->checkOverflow(parent);
        this->stack[++this->size] = n;
    }

    float pop(QWidget *parent){
        this->checkUnderflow(parent);
        return this->stack[size--];
    }

    float peek(){
        return this->stack[size];
    }

    QString getText(){
        QString aux = "";
        for(int i = 0; i < this->size + 1; i++){
            aux += QString::number(this->stack[i]) + (i < this->size ? ", " : "");
        }
        return aux;
    }

    void showStack(QLineEdit *text){
        text->setText(this->getText());
    }
};

class Operations : public Stack{
public:
    void input(QWidget *parent){
        QString text = QInputDialog::getText(parent, "Input function", "Digite um n??mero de ponto flutuante:");
        if(this->isFloat(text.toStdString())){
            float aux = text.toFloat();
            this->push(aux, parent);
        }
        else{
            QMessageBox::about(parent, "Error", "Valor digitado deve ser um n??mero");
        }
    }

    void print(QWidget *parent){
        QMessageBox::about(parent, "Print function", "N??mero retirado e exibido: " + QString::number(this->pop(parent)));
    }

    void add(QWidget *parent){
        float aux1 = this->pop(parent);
        float aux2 = this->pop(parent);
        this->push(aux2 + aux1, parent);
    }

    void sub(QWidget *parent){
        float aux1 = this->pop(parent);
        float aux2 = this->pop(parent);
        this->push(aux2 - aux1, parent);
    }

    void mul(QWidget *parent){
        float aux1 = this->pop(parent);
        float aux2 = this->pop(parent);
        this->push(aux2 * aux1, parent);
    }

    void div(QWidget *parent){
        float aux1 = this->pop(parent);
        if(aux1 == 0){
            QMessageBox::about(parent, "Error", "DIVIDE BY ZERO!");
            sleep(2000);
            throw runtime_error("DIVIDE BY ZERO ERROR");
        }
        float aux2 = this->pop(parent);
        this->push(aux2 / aux1, parent);
    }

    void swap(QWidget *parent){
        float first = this->pop(parent);
        float second = this->pop(parent);
        this->push(first, parent);
        this->push(second, parent);
    }

    void drop(QWidget *parent){
        this->pop(parent);
    }

    void dup(QWidget *parent){
        float aux = this->pop(parent);
        this->push(aux, parent);
        this->push(aux, parent);
    }

    void hlt(QWidget *parent){
        QMessageBox::about(parent, "HLT Function", "A pilha e a mem??ria ser??o mostradas por 5 segundos at?? o t??rmino do programa!");
        sleep(5000);
        this->clearStack();
        QMessageBox::about(parent, "HLT Function", "Pilha e mem??ria esvaziadas!");
    }
};

class Machine : public Operations{
private:
    string instructions[14] = {
        "pushi", "push",
        "pop", "input",
        "print", "add",
        "sub", "mul",
        "div", "swap",
        "drop", "dup", "hlt"
    };

public:
    int execute(string instruction){
        instruction = this->lower(instruction);
        for(int i = 0; i < 14; i++){
            if(this->instructions[i] == instruction){
                return i;
            }
        }
        return -1;
    }

    bool handleErrors(int num, QWidget *parent){
        if(num == 0){
            if(!this->isFloat(parsed[1])){
                QMessageBox::about(parent, "Error", "Fun????o pushi aceita somente n??meros!");
                return false;
            }
            return true;
        }
        else if(num == 1 || num == 2){
            if(parsed[1].size() != 2){
                QMessageBox::about(parent, "Error", "Erro: endere??o hexadecimal deve ser especificado com 2 d??gitos (Ex: 00 - FF)!");
                return false;
            }
            return true;
        }
        else{
            if(parsed.size() != 1){
                 QMessageBox::about(parent, "Error", "Erro: instru????o inexistente!");
                 return false;
            }
            return true;
        }
    }
};

//Button click event

Machine machine;

void MainWindow::on_pushButton_clicked()
{
    QString ins = this->ui->lineEdit->text();
    machine.parseInstruction(ins.toStdString(), ' ', parsed);
    int index = machine.execute(parsed[0]);
    if(machine.handleErrors(index, this)){
        switch(index){
            case 0:
                machine.push(QString::toStdString(parsed[1]).toFloat(), this);
                machine.showStack(this->ui->lineEdit_2);
                break;

            case 1:
                machine.push(machine.getMemory(parsed[1]), this);
                machine.showStack(this->ui->lineEdit_2);
                break;

            case 2:
                machine.setMemory(parsed[1], machine.pop(this));
                machine.showMemoryChanges(this->ui->plainTextEdit);
                machine.showStack(this->ui->lineEdit_2);
                break;

            case 3:
                machine.input(this);
                machine.showStack(this->ui->lineEdit_2);
                break;

            case 4:
                machine.print(this);
                machine.showStack(this->ui->lineEdit_2);
                break;

            case 5:
                machine.add(this);
                machine.showStack(this->ui->lineEdit_2);
                break;

            case 6:
                machine.sub(this);
                machine.showStack(this->ui->lineEdit_2);
                break;

            case 7:
                machine.mul(this);
                machine.showStack(this->ui->lineEdit_2);
                break;

            case 8:
                machine.div(this);
                machine.showStack(this->ui->lineEdit_2);
                break;

            case 9:
                machine.swap(this);
                machine.showStack(this->ui->lineEdit_2);
                break;

            case 10:
                machine.drop(this);
                machine.showStack(this->ui->lineEdit_2);
                break;

            case 11:
                machine.dup(this);
                machine.showStack(this->ui->lineEdit_2);
                break;

            case 12:
                machine.hlt(this);
                machine.showStack(this->ui->lineEdit_2);
                machine.showMemoryChanges(this->ui->plainTextEdit);
                this->ui->plainTextEdit->setPlainText("");
                break;

            default:
                QMessageBox::about(this, "Error", "Instru????o inexistente!");
        }
    }
    parsed.clear();
}

void MainWindow::on_actionInstru_es_da_pilha_triggered()
{
    QMessageBox::about(this, "Instru????es", "PUSHI VALOR (EX: PUSHI 5.5) -> Adiciona o valor 5.5 na pilha\n\nPOP ENDERE??O HEXADECIMAL (EX: POP 8A) -> Adiciona o ??ltimo valor da pilha na mem??ria\n\nPUSH ENDERE??O HEXADECIMAL (EX: PUSH 8A) -> Insere na pilha o valor contido no endere??o 8A da mem??ria\n\nINPUT -> Insere um valor, escolhido pelo usu??rio, na pilha\n\nPRINT -> Retira e exibe o ??ltimo valor da pilha\n\nADD -> Soma os ??ltimos 2 valores da pilha\n\nSUB -> Subtrai os ??ltimos 2 valores da pilha\n\nMUL -> Multiplica os ??ltimos dois valores da pilha\n\nDIV -> Divide os ??ltimos dois valores da pilha\n\nSWAP -> Troca os dois ??ltimos valores de posi????o na pilha\n\nDROP -> Retira o ??ltimo valor da pilha\n\nDUP -> Duplica o ??ltimo valor da pilha\n\nHLT -> T??rmino do programa");
}


void MainWindow::on_actionSobre_o_projeto_triggered()
{
    QMessageBox::about(this, "Sobre o projeto", "O interpretador de pilha ?? um projeto que foi solicitado como projeto final da disciplina de arquitetura e organiza????o de computadores no curso de ci??ncia da computa????o. Gostei muito da ideia base e decidi ir mais longe do que a simples aplica????o de console, usando o QTCreator para a GUI (Graphical User Interface) e desenvolver uma aplica????o desktop. Tamb??m fiz a aplica????o de console, mas quis ir mais longe ent??o fiz uma aplica????o QT desktop e uma vers??o web com JS, que voc??s podem encontrar no meu github.\n\nAgradecimento especial ao professor Marcos Jos?? Brusso, g??nio da disciplina. Ele me ajudou muito em todas as quest??es, com certeza aprendi muito fazendo tudo isso!\n\nAproveite o programa!\n\nDesenvolvido por: Gui Taglietti\n\nGitHub: GuiTaglietti");
}

