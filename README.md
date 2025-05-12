<h1 align='center'>Crazy Compiler :D</h1>

<!--  Overview  -->
## <img  align= center width =60px src="https://cdn-icons-png.flaticon.com/512/8632/8632710.png"> Overview <a id="overview"></a>
This is a custom C++ compiler project built using **Lex** and **Yacc** (Flex and Bison), which performs:
- Lexical analysis
- Syntax analysis
- Semantic analysis
- Symbol table construction
- Intermediate code generation (Quadruples)


<h2 href="#Structure">📁 Project Structure</h2>
 <div> 
  <pre>
├── src
│   └── compiler.cpp
│   └── compiler.h
│   └── lexer.l
│   └── parser.y
├── logs
│   └── bison_errors.log
│   └── flex_errors.log
│   └── gcc_errors.log
├── outputs
│   └── action.txt
│   └── error.txt
│   └── symbol.txt
├── GUI
│   └── gui.py
├── run.sh
├── run.bat
└── README.md
    </pre>
</div>

<!-- Getting Started -->
## <img align="center" width="60px" height="60px" src="https://media3.giphy.com/media/wuZWV7keWqi2jJGzdB/giphy.gif?cid=6c09b952wp4ev7jtywg3j6tt7ec7vr3piiwql2vhrlsgydyz&ep=v1_internal_gif_by_id&rid=giphy.gif&ct=s"> Getting Started <a id="started"></a>

1. **Clone the Repository**
    ```bash
    git clone https://github.com/Sara-Gamal12/compilers-project.git
    ```

2. **Change Directory**
    ```bash
    cd compilers-project
    ```

3. **Running the project (UNIX)**
    ```bash
    chmod +x run.sh
    ./run.sh
    ```

3. **Running the project (WINDOWS)**
    ```bat
    run.bat
    ```

## Syntax

Tha language supports the following data types:

- Integer
- Float
- Boolean
- String
- Character




### Operators

The language supports the common operators.

```c
// Arithmetic operators
a = b + c;
a = b - c;
a = b * c;
a = b / c;
a = b % c;
a = b++;
a = ++b;
a = b--;
a = --b;
```
### Conditional Statements

The language supports the if-else, and switch-case statements.

```c
int a = 10;
int b = 100;
// if statement
if (a <= 10) {
    print(a);
}
else {
    if (b > 100) {
        print(b);
    }
    else {
        print("else");
    }
}

// switch-case statement
switch (a) {
    case 1: 
        print("1");
        break;
    
    case 2: 
        print("2");
        break;
    
    case 3: 
        print("3");
        break;
    
    default: 
        print("default");
        break;
}

```

### Loops

The language supports the while, for, and repeat-until loops.

```c
// while loop
int a = 0;
while (a < 20) {
    print(a);
    a = a + 1;
}
print(a);
while (a < 20) {
    if (a == 10) {
        print(a);
    }
    a = a + 1;
}
// for loop
for (int b=2 ; b<10; b++ ) {
    print(b);
    while (b < 10) {
        if (b == 5) {
            print("hi");
            print(b);
        }
    }
}

```

### Functions

The language supports functions with and without parameters. The language also checks for parameters count and types in the function call

```c
def func_1 (){
    print("func_1");
    return ;
}
def func_2(int a, int b=10) {
    print("func_2");
    print(a);
    print(b);
    return ;
}
func_2(1); // function call using the default value
```

<!-- Contributors -->
## <img  align= center width=50px height=50px src="https://media1.giphy.com/media/WFZvB7VIXBgiz3oDXE/giphy.gif?cid=6c09b952tmewuarqtlyfot8t8i0kh6ov6vrypnwdrihlsshb&rid=giphy.gif&ct=s"> Contributors <a id = "contributors"></a>
<!-- readme: collaborators -start -->
<table  align='center'> 
<tr>
    <td align="center">
        <a href="https://github.com/yousefosama654">
            <img src="https://avatars.githubusercontent.com/u/93356614?v=4" width="100;" alt="yousefosama654"/>
            <br />
            <sub><b>Yousef</b></sub>
        </a>
    </td>
    <td align="center">
        <a href="https://github.com/EmanElbedwihy">
            <img src="https://avatars.githubusercontent.com/u/120182209?v=4" width="100;" alt="EmanElbedwihy"/>
            <br />
            <sub><b>Eman</b></sub>
        </a>
    </td>
        <td align="center">
        <a href="https://github.com/nesma-shafie">
            <img src="https://avatars.githubusercontent.com/u/120175134?v=4" width="100;" alt="nesma-shafie"/>
            <br />
            <sub><b>Nesma</b></sub>
        </a>
    </td>
    <td align="center">
        <a href="https://github.com/Sara-Gamal1">
            <img src="https://avatars.githubusercontent.com/u/106556638?v=4" width="100;" alt="Sara-Gamal1"/>
            <br />
            <sub><b>Sara</b></sub>
        </a>
    </td></tr>
</table>
