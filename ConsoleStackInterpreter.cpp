/*
    
    -- Gui Taglietti --
    
    Machine interpreter based on stack data structure
    Console version algorithm

*/

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <string>
using namespace std;

class Control{
    public:
        bool blankSpace(string x){
            for(auto c : x){
                if(c == ' '){
                    return true;
                }
            }
            return false;
        }

        int findRange(string x){
            int range = 0;
            for(char c : x){
                if(c == ' '){
                    return range + 1;
                }
                range++;
            }
            return 0;
        }

        int findLenght(string x){
            int lenght = 0;
            string vars = "0123456789.";
            if(findRange(x)){
                for(char c : x){
                    if(count(vars.begin(), vars.end(), c)){
                        lenght += count(vars.begin(), vars.end(), c);
                    }
                }
            }
            return lenght;
        }

        string hexString(string x){
            return x.substr(this->findRange(x), 2);
        }
        
        string lower(string x){
            for(int i = 0; i < x.length(); i++){
                x[i] = tolower(x[i]);
            }
            return x;
        }

        float toFloat(string x){
            return stof(x.substr(this->findRange(x), this->findLenght(x)));
        }

        int hexConverter(string x){
            return stoi(hexString(x), 0, 16);
        }

        string parseInstruction(string x){
            int l = 0;
            for(auto c : x){
                if(c == ' '){
                    return x.substr(0, l);
                }
                l++;
            }
            return 0;
        }

};

class Memory : public Control{
    protected:
        int pos = -1;
        float mem[256];
        float access[256];
        string hexAccess[256];


    public:
        Memory(){
            memset(this->mem, 0, sizeof(this->mem));
        }
        
        void setMemory(int local, float value){
            this->mem[local] = value;
        }

        float getMemory(string hex){
            return this->mem[this->hexConverter(hex)];
        }

        void addInfo(string hex, float value){
            this->hexAccess[++this->pos] = hex;
            this->access[this->pos] = value;
        }
        
        void showInfo(){
            cout << "Endereços modificados na memória:" << endl;
            for(int i = 0; i <= pos; i++){
                cout << "Endereço: " << this->hexAccess[i] << " recebeu o valor " <<
                this->access[i] << endl;
            }
        }

};

class Stack : public Memory{
    private:
        float arr[16];
        int size = -1;
    
    public:
        Stack(){
            memset(this->arr, 0, sizeof(this->arr));
        }
        
        void underflowError(){
            if(this->size < 0){
                throw underflow_error("STACK UNDERFLOW");
            }
        }
        
        void overflowError(){
            if(this->size > 14){
                throw underflow_error("STACK OVERFLOW");
            }
        };
        
        void push(float e){
            this->overflowError();
            this->arr[++this->size] = e; 
        }
        
        float pop(){
            this->underflowError();
            return this->arr[size--];
        }
        
        void show(){
            cout << "Valores restantes na pilha: ";
            for(int i = 0; i < this->size + 1; i++){
                cout << this->arr[i] << " ";
            }
            cout << endl;
        }
};

class Operations : public Stack{
    public:
        void input(){
            float aux;
            cout << "Digite o valor para ser inserido na pilha: " << endl;
            cin >> aux;
            cin.ignore();
            this->push(aux);
        }
        
        void print(){
            cout << "Valor do topo da pilha: " << this->pop() << endl;
        }

        void add(){
            float n1 = this->pop();
            float n2 = this->pop();
            this->push(n2 + n1);
            cout << "Foi empilhado o valor " << n2 + n1 << " visto que os valores somados foram " << n2 
            << " + "  << n1 << endl;
        }
        
        void sub(){
            float n1 = this->pop();
            float n2 = this->pop();
            this->push(n2 - n1);
            cout << "Foi empilhado o valor " << n2 + n1 << " visto que os valores subtraidos foram " << n2 
            << " - "  << n1 << endl;
        }
        
        void mul(){
            float n1 = this->pop();
            float n2 = this->pop();
            this->push(n2 * n1);
            cout << "Foi empilhado o valor " << n2 * n1 << " visto que os valores multiplicados foram " << n2 
            << " * "  << n1 << endl;
        }
        
        void div(){
            float n1 = this->pop();
            if(n1 == 0){
                throw runtime_error("DIVIDE BY ZERO ERROR");
            }
            float n2 = this->pop();
            this->push(n2 / n1);
            cout << "Foi empilhado o valor " << n2 / n1 << " visto que os valores divididos foram " << n2 
            << " / "  << n1 << endl;
        }
        
        void swap(){
            float first = this->pop();
            float second = this->pop();
            this->push(first);
            this->push(second);
            cout << "Valores trocados: " << first << " trocou com " << second << endl;
        }
        
        void drop(){
            this->pop();
        }
        
        void dup(){
            float aux = this->pop();
            this->push(aux);
            this->push(aux);
        }
        
        void hlt(){
            cout << endl;
            this->showInfo();
            cout << endl;
            this->show();
            cout << endl;
            exit(0);
        }
};

class Instructions : public Operations{
    private:
        string ins[14] = {
            "pushi", "push",
            "pop", "input",
            "print", "add",
            "sub", "mul",
            "div", "swap",
            "drop", "dup", "hlt"
        };

    public:
        int decode(string order){
            string aux = this->lower(order);
            for(int i = 0; i < 14; i++){
                if(this->ins[i] == aux){
                    return i;
                }
            }
            return -1;
        }

        int loadInstruction(string order){
            if(this->blankSpace(order)){
                string aux = this->parseInstruction(order);
                int expr = this->decode(aux);
                return expr;
            }
            else{
                int expr = this->decode(order);
                return expr;
            } 
        }

        void solveInstruction(string ins){
            int aux = this->loadInstruction(ins);
            switch(aux){
                case 0:
                    this->push(this->toFloat(ins));
                    break;
                case 1:
                    this->push(this->getMemory(ins));
                    break;
                case 2:
                {
                    float holder = this->pop();
                    this->setMemory(this->hexConverter(ins), holder);
                    this->addInfo(this->hexString(ins), holder);
                    break;
                }
                case 3:
                    this->input();
                    break;
                case 4:
                    cout << "Valor retirado e exibido: " << this->pop() << endl;
                    break;
                case 5:
                    this->add();
                    break;
                case 6:
                    this->sub();
                    break;
                case 7:
                    this->mul();
                    break;
                case 8:
                    this->div();
                    break;
                case 9:
                    this->swap();
                    break;
                case 10:
                    cout << "O valor " << this->pop() << " foi retirado da pilha!" << endl;
                    break;
                case 11:
                    this->dup();
                    cout << "Valor do topo duplicado com sucesso!" << endl; 
                    break;
                case 12:
                    this->hlt();
                    break;
                default:
                {
                    string error = "INSTRUCTION " + ins + " DOESN'T EXIST!";
                    throw logic_error(error);
                    break;
                }
            }
        }
};

int main(){
    Instructions *s = new Instructions();
    string i;
    while(true){
        cout << "Digite uma instrução:" << endl;
        getline(cin, i);
        s->solveInstruction(i);
    }
    delete s;
    return 0;
}
