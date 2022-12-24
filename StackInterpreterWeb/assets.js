class Control{
    parseInstruction(instruction){
        return instruction.split(" ");
    }

    hexConverter(hex){
        return parseInt(hex, 16);
    }

    toFloat(instruction){
        let arr = this.parseInstruction(instruction);
        return parseFloat(arr[1]);
    }
}

class Memory extends Control{
    #mem = [];
    #log = [];
    #values = [];

    constructor(){
        super();
        this.#mem.fill(0);
    }

    _clearMemory(){
        this.#mem = [];
        this.#log = [];
        this.#values = [];
    }

    setMemory(pos, value){
        if(pos > 256 || pos < 0){
            throw new Error("OUT OF MEMORY RANGE");
        }
        else{
            this.#mem[pos] = value;
        }
    }

    getMemory(hex){
        return this.#mem[this.hexConverter(hex)];
    }

    setInfo(hex, value){
        this.#log.push(hex);
        this.#values.push(value);
    }

    showInfo(){
        let message = "";
        for(let i = 0; i < this.#log.length; i++){
            message += `O endereço de memória ${this.#log[i]} recebeu o valor ${this.#values[i]}\n`
        }
        document.getElementById("mem-reg").value = message;
    }
}

class Stack extends Memory{
    #stack = [];

    constructor(){
        super();
        this.#stack.fill(0);
    }

    _clearStack(){
        this.#stack = [];
    }

    underflowError(){
        if(this.#stack.length < 1){
            alert("STACK UNDERFLOW!");
            throw new Error("STACK UNDERFLOW");
        }
    }

    overflowError(){
        if(this.#stack.length > 15){
            alert("STACK OVERFLOW!");
            throw new Error("STACK OVERFLOW");
        }
    }
    
    pushElement(element){
        this.overflowError();
        let aux = parseFloat(element);
        if(isNaN(aux)){
            alert(`${element} não é um número para ser inserido na pilha!`);
        }
        else{
            this.#stack.push(aux);
        }
    }

    popElement(){
        this.underflowError();
        return this.#stack.pop();
    }

    showStack(){
        let message = "";
        for(let i = 0; i < this.#stack.length; i++){
            message += `${this.#stack[i]}, `;
        }
        document.getElementById("stack").value = message.slice(0, message.length - 2);
    }

    peek(){
        return this.#stack[this.#stack.length - 1];
    }
}

class Operations extends Stack{
    constructor(){
        super();
    }

    clearAll(){
        this._clearMemory();
        this._clearStack();
        document.getElementById("stack").value = "";
        document.getElementById("mem-reg").value = "";
    }

    input(){
        let num = prompt("Insira um número de ponto flutuante na pilha:");
        this.pushElement(num);
    }

    print(){
        alert(`Número retirado da pilha e exibido: ${this.popElement()}`);
    }

    add(){
        let a = this.popElement();
        let b = this.popElement();
        this.pushElement(b + a);
    }

    sub(){
        let a = this.popElement();
        let b = this.popElement();
        this.pushElement(b - a);
    }

    mul(){
        let a = this.popElement();
        let b = this.popElement();
        this.pushElement(b * a);
    }

    div(){
        /*
        let a = this.peek() != 0 ? this.popElement() : () => {
            throw new Error("DIVIDE BY ZERO");
        };
        */
        if(this.peek() == 0){
            alert("DIVIDE BY ZERO");
            throw new Error("DIVIDE BY ZERO");
        } 
        else{
            let a = this.popElement();
            let b = this.popElement();
            this.pushElement(b / a);
        }
    }

    swap(){
        let first = this.popElement();
        let second = this.popElement();
        this.pushElement(first);
        this.pushElement(second);
    }

    drop(){
        alert(`Valor descartado da pilha: ${this.popElement()}`);
    }

    dup(){
        let aux = this.popElement();
        this.pushElement(aux);
        this.pushElement(aux);
    }

    hlt(){
        let ans = prompt("Deseja encerrar o programa? (1 = Sim // 2 = Não)");
        switch(parseInt(ans)){
            case 1:
                setTimeout(() => {
                    this.showStack();
                    this.clearAll();
                }, 1000);
                alert("Os valores restantes da pilha e valores modificados na memória estão exibidos.");
                break;

            case 2:
                alert("Operação cancelada!");
                break;

            default:
                alert("Instrução inexistente (Use o dígito 1 para sim e o dígito 2 para não)!");
                break;
        }
    }
}

class Machine extends Operations{
    constructor(){
        super();
    }

    solveInstruction(instruction){
        let parsed = this.parseInstruction(instruction);
        switch(parsed[0].toLowerCase()){
            case "pushi":
                this.pushElement(parsed[1]);
                break;
            
            case "push":
                this.pushElement(this.getMemory(parsed[1]));
                break;

            case "pop":
                let aux = this.popElement();
                let hex = this.hexConverter(parsed[1]);
                this.setMemory(hex, aux);
                this.setInfo(hex, aux);
                break;

            case "input":
                this.input();
                break;
                
            case "print":
                this.print();
                break;

            case "add":
                this.add();
                break;

            case "sub":
                this.sub();
                break;

            case "mul":
                this.mul();
                break;

            case "div":
                this.div();
                break;

            case "swap":
                this.swap();
                break;

            case "drop":
                this.drop();
                break;

            case "dup":
                this.dup();
                break

            case "hlt":
                this.hlt();
                break;

            default:
                alert("Instrução inexistente!");
                break;
        }
    }
}

const m = new Machine();
let button = document.getElementById("button");
button.addEventListener("click", (e) => {
    e.preventDefault();
    let ans = document.getElementById("inst").value;
    m.solveInstruction(ans);
    m.showStack();
    m.showInfo();
})

document.getElementById("inst").addEventListener("keypress", (e) => {
    if(e.key === "Enter"){
        m.solveInstruction(document.getElementById("inst").value);
        m.showStack();
        m.showInfo();
    }
})
