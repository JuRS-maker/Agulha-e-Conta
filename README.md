# 🧶 Agulha & Conta

> O sistema que calcula quanto cobrar pelo seu amigurumi, sem prejuízo e sem susto.

Aplicação de terminal, escrita em **C**, que ajuda artesãs de crochê a **precificar peças** e a **guardar a receita** (o tutorial) de cada projeto. Trabalho final da disciplina de **Linguagem de Programação**.

---

## 💡 A ideia

Quem trabalha com crochê sabe que precificar é difícil:

- Um novelo de linha raramente é usado inteiro em uma peça só.
- Olhos, enchimento e até os pedacinhos de linha dos detalhes também custam dinheiro.
- Materiais de artesanato são vendidos em pacotes grandes, e cobrar o pacote inteiro deixa a peça cara demais.

O **Agulha & Conta** resolve isso cobrando **só a parte do material que foi realmente usada**, somando o valor do seu tempo e a margem de lucro que você quer ter.

---

## ✨ Funcionalidades

| Recurso | O que faz |
|---|---|
| 🧵 **Materiais** | Cadastra nome, marca, cor, tipo de venda (unidade ou grama), preço do pacote e quantidade usada |
| ⚖️ **Pesagem da sobra** | Para linhas, basta pesar o que sobrou do novelo e o programa calcula quanto foi gasto |
| 📖 **Receita por partes** | Registra a receita dividida em partes (cabeça, braço, corpinho...), com os passos de cada uma, carreira por carreira |
| ⏱️ **Mão de obra e lucro** | Recebe as horas trabalhadas, o valor da hora e a porcentagem de lucro desejada |
| 🧮 **Precificação final** | Calcula o preço final da peça |
| 📄 **Relatório** | Mostra na tela e salva em `relatorio_croche.txt` |
| 📚 **Meus projetos** | Cria, lista, abre e exclui vários projetos, e busca por pedaço do nome |

Cada projeto também guarda o **nome**, a **agulha** utilizada (2,5 mm, por exemplo) e o **tamanho dos pontos** (apertado, normal ou solto).

---

## 🧮 Como o preço é calculado

```
custo de cada material = (preço do pacote ÷ quantidade do pacote) × quantidade usada

materiais   = soma do custo de todos os materiais
mão de obra = horas trabalhadas × valor da hora
subtotal    = materiais + mão de obra
lucro       = subtotal × (% de lucro ÷ 100)

PREÇO FINAL = subtotal + lucro
```

### Exemplo: Ursinho Narigudo

| Material | Pacote | Usado | Custo |
|---|---|---|---|
| Fio Amigurumi Chenille (Bege) | R$ 30,00 por 200 g | 50 g | R$ 7,50 |
| Olhos de segurança (Preto) | R$ 12,00 por 6 pares | 1 par | R$ 2,00 |
| Fibra de enchimento (Branca) | R$ 20,00 por 500 g | 80 g | R$ 3,20 |
| Linha preta do focinho | R$ 5,00 por 50 g | 2 g | R$ 0,20 |

Com **6,5 horas** de trabalho a **R$ 15,00 por hora** e **30% de lucro**:

| Item | Valor |
|---|---|
| Materiais | R$ 12,90 |
| Mão de obra (6,5 h × R$ 15,00) | R$ 97,50 |
| Subtotal | R$ 110,40 |
| Lucro (30%) | R$ 33,12 |
| **Preço final** | **R$ 143,52** |

---

## 🖥️ Exemplo de relatório

```
==================================================
              PRECIFICAÇÃO DE CROCHÊ
==================================================
Projeto : Ursinho Narigudo
Agulha  : 2,5mm
Pontos  : Normal

---------------- MATERIAIS ----------------
- Fio Amigurumi Chenille (Circulo, Bege)
    usado 50,0 g de 200,0 g  ->  R$ 7,50
- Olhos de segurança (generica, Preto)
    usado 1,0 un de 6,0 un  ->  R$ 2,00

---------------- RECEITA ----------------

Cabeça:
   1. 1ª carr.: 8 pb. no anel mágico
   2. 2ª carr.: 8 aum. = 16 pb.
   3. 3ª carr.: *1 pb., 1 aum.* = 24 pb.

---------------- PRECIFICAÇÃO ----------------
Materiais ............... R$    12,90
Horas trabalhadas ....... 6,50 h x R$ 15,00 = R$ 97,50
Subtotal ................ R$   110,40
Lucro (30,0%) ............ R$    33,12
==================================================
PREÇO FINAL ............. R$   143,52
==================================================
```

---

## 🚀 Como executar

**Requisito:** um compilador C, como o `gcc`.

```bash
gcc precificacao_croche.c -o croche
./croche            # no Windows: croche.exe
```

> 💬 **Acentos no Windows:** o programa usa `setlocale`. Salve o arquivo `.c` como **ANSI (Windows-1252)** para os acentos aparecerem corretamente no terminal. No Linux e no Mac, use UTF-8.
>
> 🔢 **Números:** você pode digitar decimais com vírgula ou com ponto (`2,5` ou `2.5`).

---

## 🧭 Como usar

```
MEUS PROJETOS
 1 - Novo projeto
 2 - Listar projetos
 3 - Abrir projeto
 4 - Excluir projeto
 5 - Buscar projeto
 0 - Sair

PROJETO
 1 - Dados do projeto (nome, agulha, pontos)
 2 - Materiais
 3 - Receita (partes e passos)
 4 - Horas, valor da hora e lucro
 5 - Ver precificação final
 6 - Salvar relatório em arquivo .txt
 0 - Voltar
```

Um bom caminho para a primeira peça: **novo projeto → materiais → receita → horas e lucro → ver precificação**.

---

## 🧱 Como o código está organizado

O programa gira em torno de três `struct`:

```c
typedef struct { /* nome, marca, cor, tipo, precoTotal, qtdTotal, qtdUsada */ } Material;
typedef struct { /* nome, numPassos, passos[MAX_PASSOS][TAM_PASSO] */        } Parte;
typedef struct { /* nome, agulha, tamPontos, materiais[], partes[],
                    horas, valorHora, lucroPct */                            } Projeto;
```

| Grupo | Funções |
|---|---|
| Leitura segura | `lerTexto`, `lerNumero`, `lerOpcao`, `pausar` |
| Cálculos | `custoMaterial`, `custoTotalMateriais`, `custoMaoDeObra`, `valorLucro`, `precoFinal` |
| Cadastros | `cadastrarProjeto`, `adicionarMaterial`, `listarMateriais`, `removerMaterial`, `adicionarParte`, `removerParte`, `informarTrabalho` |
| Relatório | `imprimirRelatorio`, `salvarRelatorio` |
| Projetos | `novoProjeto`, `listarProjetos`, `abrirProjeto`, `excluirProjeto`, `buscarProjeto` |
| Menus | `menuProjetos`, `menuProjeto`, `menuMateriais`, `menuReceita` |

---

## 📚 Conteúdos de Linguagem de Programação aplicados

| Conteúdo | Onde aparece |
|---|---|
| **Structs** | `Material`, `Parte` e `Projeto` |
| **Ponteiros** | Funções que recebem `Projeto *p` e `Material *m`; acesso com `->`; uso de `const` nas funções que só leem |
| **Matriz** | `passos[MAX_PASSOS][TAM_PASSO]`: cada linha é um passo da receita |
| **Strings** | `fgets`, `strcpy`, `strcspn`, `memset`; busca de texto dentro do nome do projeto |
| **Operações condicionais e lógicas** | `if`/`else`, operador ternário, `&&` e `||` na validação das entradas |
| **Entrada e saída** | `printf`, `fgets`, `fprintf` e gravação do relatório em arquivo (`fopen`/`fclose`) |
| **Laços de repetição** | `for` (cálculos e listagens), `while` (leitura dos passos) e `do...while` (menus) |
| **Busca sequencial** | `buscarProjeto`, que percorre a lista comparando os nomes |

---

## 🛣️ Próximos passos

- [ ] Gravar todos os projetos em arquivo, para não se perderem ao fechar o programa
- [ ] Ordenar os projetos por nome ou por preço
- [ ] Busca binária pelo nome exato do projeto
- [ ] Custo por parte da peça (cabeça, braço, corpinho)

---

## 👩‍💻 Autoras

- **Ana Vitória Bernardes de Aguiar**
- **Júlia Ribeiro de Souza**

Projeto final da disciplina de **Linguagem de Programação**, 2026.
