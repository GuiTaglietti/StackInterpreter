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
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <iostream>
#include <cstdlib>
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
            text += "O endereço de memória " + QString::fromStdString(this->hexValues[i]) + " recebeu o valor: " +
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
            Sleep(2000);
            throw underflow_error("STACK OVERFLOW");
        }
    }

    void checkUnderflow(QWidget *parent){
        if(this->size < 0){
            QMessageBox::about(parent, "Error", "STACK UNDERFLOW!");
            Sleep(2000);
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
        QString text = QInputDialog::getText(parent, "Input function", "Digite um número de ponto flutuante:");
        if(this->isFloat(text.toStdString())){
            float aux = text.toFloat();
            this->push(aux, parent);
        }
        else{
            QMessageBox::about(parent, "Error", "Valor digitado deve ser um número");
        }
    }

    void print(QWidget *parent){
        QMessageBox::about(parent, "Print function", "Número retirado e exibido: " + QString::number(this->pop(parent)));
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
            Sleep(2000);
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
        QMessageBox::about(parent, "HLT Function", "A pilha e a memória serão mostradas por 5 segundos até o término do programa!");
        Sleep(5000);
        this->clearStack();
        QMessageBox::about(parent, "HLT Function", "Pilha e memória esvaziadas!");
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
                QMessageBox::about(parent, "Error", "Função pushi aceita somente números!");
                return false;
            }
            return true;
        }
        else if(num == 1 || num == 2){
            if(parsed[1].size() != 2){
                QMessageBox::about(parent, "Error", "Erro: endereço hexadecimal deve ser especificado com 2 dígitos (Ex: 00 - FF)!");
                return false;
            }
            return true;
        }
        else{
            if(parsed.size() != 1){
                 QMessageBox::about(parent, "Error", "Erro: instrução inexistente!");
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
                machine.push(stof(parsed[1]), this);
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
                QMessageBox::about(this, "Error", "Instrução inexistente!");
        }
    }
    parsed.clear();
}

void MainWindow::on_actionInstru_es_da_pilha_triggered()
{
    QMessageBox::about(this, "Instruções", "PUSHI VALOR (EX: PUSHI 5.5) -> Adiciona o valor 5.5 na pilha\n\nPOP ENDEREÇO HEXADECIMAL (EX: POP 8A) -> Adiciona o último valor da pilha na memória\n\nPUSH ENDEREÇO HEXADECIMAL (EX: PUSH 8A) -> Insere na pilha o valor contido no endereço 8A da memória\n\nINPUT -> Insere um valor, escolhido pelo usuário, na pilha\n\nPRINT -> Retira e exibe o último valor da pilha\n\nADD -> Soma os últimos 2 valores da pilha\n\nSUB -> Subtrai os últimos 2 valores da pilha\n\nMUL -> Multiplica os últimos dois valores da pilha\n\nDIV -> Divide os últimos dois valores da pilha\n\nSWAP -> Troca os dois últimos valores de posição na pilha\n\nDROP -> Retira o último valor da pilha\n\nDUP -> Duplica o último valor da pilha\n\nHLT -> Término do programa");
}


void MainWindow::on_actionSobre_o_projeto_triggered()
{
    QMessageBox::about(this, "Sobre o projeto", "O interpretador de pilha é um projeto que foi solicitado como projeto final da disciplina de arquitetura e organização de computadores no curso de ciência da computação. Gostei muito da ideia base e decidi ir mais longe do que a simples aplicação de console, usando o QTCreator para a GUI (Graphical User Interface) e desenvolver uma aplicação desktop. Também fiz a aplicação de console, mas quis ir mais longe então fiz uma aplicação QT desktop e uma versão web com JS, que vocês podem encontrar no meu github.\n\nAgradecimento especial ao professor Marcos José Brusso, gênio da disciplina. Ele me ajudou muito em todas as questões, com certeza aprendi muito fazendo tudo isso!\n\nAproveite o programa!\n\nDesenvolvido por: Gui Taglietti\n\nGitHub: GuiTaglietti");
}

