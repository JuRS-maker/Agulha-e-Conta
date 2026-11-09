# 🧶 Agulha & Conta

> O sistema que calcula quanto cobrar pelo seu amigurumi, sem prejuízo e sem susto.

Aplicação de terminal, escrita em **C**, que ajuda artesãs de crochê a **precificar peças**, **guardar a receita** (o tutorial) de cada projeto e **manter todos os projetos salvos** para consultar depois. Trabalho final da disciplina de **Linguagem de Programação**.

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
| 📚 **Vários projetos** | Cadastra, escolhe, edita e exclui projetos (até 50), cada um com nome, agulha e tamanho dos pontos (apertado, normal ou solto) |
| 🧵 **Materiais** | Cadastra nome, marca, cor, tipo de venda (unidade ou grama), preço do pacote e quantidade usada |
| ♻️ **Reaproveitar material** | Reutiliza um material já cadastrado no projeto (o mesmo novelo na cabeça e no corpo, por exemplo) e pergunta só a quantidade usada |
| ⚖️ **Pesagem da sobra** | Para linhas, basta pesar o que sobrou do novelo e o programa calcula quanto foi gasto |
| 📖 **Receita por partes** | Registra a receita dividida em partes (cabeça, braço, corpinho...), em **passos enumerados** (carreira por carreira) ou em **bloco de texto corrido** |
| ⏱️ **Mão de obra e lucro** | Recebe as horas trabalhadas, o valor da hora e a porcentagem de lucro desejada |
| 🧮 **Precificação final** | Calcula o preço final da peça |
| 🔎 **Pesquisa** | Busca materiais por nome, marca ou cor (por trecho, sem diferenciar maiúsculas nem acentos) e busca projetos salvos pelo nome |
| 💾 **Armazenamento** | Salva todos os projetos em arquivo e os carrega automaticamente ao abrir o programa |
| 📄 **Relatório** | Mostra na tela e salva em `.txt`, com um arquivo novo a cada salvamento (nunca sobrescreve o anterior) |

Entradas inválidas são tratadas: textos obrigatórios não aceitam campo vazio, números aceitam **vírgula ou ponto** (`2,5` ou `2.5`) e opções fora do menu pedem nova digitação.

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
              PRECIFICAÇÃO DE AMIGURUMI
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
gcc Prog_AeC_V8.c -o agulha_conta
./agulha_conta            # no Windows: agulha_conta.exe
```

> 💬 **Acentos:** se o terminal mostrar símbolos estranhos no lugar de letras acentuadas, confira a codificação do arquivo `.c` e a fonte do terminal (use uma fonte como Consolas).

---

## 🧭 Como usar

```
MENU PRINCIPAL
 1 - Escolher ou cadastrar projeto
 2 - Dados do projeto atual (nome, agulha, pontos)
 3 - Materiais
 4 - Receita (partes e passos)
 5 - Horas, valor da hora e lucro
 6 - Ver precificação final
 7 - Salvar relatório em arquivo .txt
 8 - Pesquisar material
 9 - Salvar
10 - Visualizar/pesquisar projetos salvos
11 - Excluir projeto
 0 - Sair
```

Um bom caminho para a primeira peça: **1 (cadastrar projeto) → 3 (materiais) → 4 (receita) → 5 (horas e lucro) → 6 (ver precificação) → 9 (salvar)**.

### Como funciona o armazenamento

- Os projetos ficam no arquivo `dados.bin`, criado na pasta de onde o programa é executado.
- Ao abrir o programa, os projetos salvos são **carregados automaticamente**.
- Antes de trocar de projeto ou de consultar os salvos, o programa **pergunta se você quer salvar** o projeto atual.
- Ao excluir um projeto (com confirmação), o arquivo é atualizado na hora.
- O relatório `.txt` é diferente do `dados.bin`: é um arquivo de leitura, nomeado `relatorio_<nome do projeto>_001.txt`, `_002.txt` e assim por diante.

> ⚠️ O `dados.bin` só serve para a versão do programa que o criou. Se os limites do código forem alterados, apague o arquivo e cadastre os projetos de novo.

### Limites do sistema

| Item | Limite |
|---|---|
| Projetos | 50 |
| Materiais por projeto | 30 |
| Partes da receita por projeto | 10 |
| Passos por parte | 30 |
| Tamanho de cada passo (ou do bloco de texto) | 2999 caracteres |

---

## 🧱 Como o código está organizado

O programa gira em torno de três `struct`, e todos os projetos ficam em um vetor global:

```c
typedef struct { /* nome, marca, cor, tipo, precoTotal, qtdTotal, qtdUsada */ } Material;
typedef struct { /* nome, numPassos, passos[MAX_PASSOS][TAM_PASSO] */        } Parte;
typedef struct { /* nome, agulha, tamPontos, materiais[], partes[],
                    horas, valorHora, lucroPct */                            } Projeto;

Projeto projetos[MAX_PROJETOS];   /* todos os projetos */
int     numProjetos;              /* quantos estão em uso */
```

Como a `struct Projeto` não tem ponteiros dentro dela, cada projeto ocupa um bloco contínuo de memória e pode ser gravado no arquivo com um único `fwrite`.

| Grupo | Funções |
|---|---|
| Leitura segura | `lerTexto`, `lerTextoObrigatorio`, `lerNumero`, `lerOpcao`, `pausar` |
| Cálculos | `custoMaterial`, `custoTotalMateriais`, `custoMaoDeObra`, `valorLucro`, `precoFinal` |
| Materiais | `adicionarMaterial`, `reaproveitarMaterial`, `mostrarMaterial`, `listarMateriais`, `removerMaterial` |
| Receita | `adicionarParte`, `removerParte` |
| Projetos | `cadastrarProjeto`, `informarTrabalho`, `selecionarProjeto`, `exigeProjeto`, `removerProjeto` |
| Relatório | `imprimirRelatorio`, `nomeSeguroArquivo`, `salvarRelatorio` |
| Armazenamento | `salvarDados`, `carregarDados`, `perguntarSalvar` |
| Pesquisa | `textoMinusculoUTF8`, `contemTexto`, `pesquisarSequencial`, `pesquisarProjeto`, `visualizarProjetosSalvos` |
| Menus | `menuMateriais`, `menuReceita`, `menuPesquisa`, `menuCarregar`, `main` |

---

## 📚 Conteúdos de Linguagem de Programação aplicados

| Conteúdo | Onde aparece |
|---|---|
| **Structs** | `Material`, `Parte` e `Projeto`, com vetores de structs dentro de structs |
| **Ponteiros** | Funções que recebem `Projeto *p`, `Material *m`, `int *atual` e `FILE *`; acesso com `->`; `const` nas funções que só leem; endereço de elementos do vetor (`&projetos[atual]`) |
| **Matriz** | `passos[MAX_PASSOS][TAM_PASSO]`: cada linha é um passo da receita |
| **Strings** | `fgets`, `strcpy`, `strcspn`, `strstr`, `sprintf`; comparação de textos sem diferenciar maiúsculas e acentos |
| **Operações condicionais e lógicas** | `if`/`else`, `switch`, operador ternário, `&&` e `||` na validação das entradas |
| **Entrada e saída** | `printf`, `fgets`, `fprintf` e arquivos (`fopen`, `fclose`, `fwrite`, `fread`) |
| **Laços de repetição** | `for` (cálculos, listagens e deslocamento de itens ao remover), `while` (leitura dos passos e validações) e `do...while` (menus) |
| **Pesquisa sequencial** | `pesquisarSequencial` (materiais) e `pesquisarProjeto` (projetos), que percorrem os itens um por um |
| **Persistência em arquivo** | Gravação e leitura de todos os projetos em arquivo binário |

---

## 🛣️ Ideias para o futuro

- [ ] Ordenar os projetos por nome ou por preço
- [ ] Busca binária pelo nome exato do projeto
- [ ] Custo por parte da peça (cabeça, braço, corpinho)
- [ ] Editar um material já cadastrado, sem precisar remover e adicionar de novo

---

## 👩‍💻 Autoras

- **[Nome da integrante 1]**
- **[Nome da integrante 2]**

Projeto final da disciplina de **Linguagem de Programação**, 2026.
