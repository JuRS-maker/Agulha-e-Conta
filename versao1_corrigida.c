#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>    /* Permite ajustar o idioma do terminal para aceitar
                          pontuação decimal e caracteres acentuados da língua portuguesa.
                          (Não explicado nos arquivos de aula) */

#define MAX_MAT     30 // Máximo de materiais
#define MAX_PARTES  10 // Máximo de partes
#define MAX_PASSOS  30 // Máximo de carreiras por parte
#define TAM_NOME    60 // Máximo de caracteres dos textos
#define TAM_PASSO   200 // Tamanho máximo dos textos de cada instrução

#define UNIDADE 1      /* Simplificação da leitura do tipo de material */
#define GRAMA   2

typedef struct {    // Cria o tipo Material com as especificações do material usado
    char  nome[TAM_NOME];
    char  marca[TAM_NOME];
    char  cor[TAM_NOME];
    int   tipo;          /* UNIDADE ou GRAMA            */
    float precoTotal;    /* quanto pagou no pacote/rolo */
    float qtdTotal;      /* quanto vem no pacote/rolo   */
    float qtdUsada;      /* quanto usou na peça         */
} Material;

typedef struct { // Cria o tipo parte
    char nome[TAM_NOME];
    int  numPassos;
    char passos[MAX_PASSOS][TAM_PASSO];   /* Matriz: cada linha é um passo */
} Parte;

typedef struct { // Projeto completo integrado com as structs que criamos acima
    char     nome[TAM_NOME];
    char     agulha[20];
    char     tamPontos[20];
    int      numMat;
    Material materiais[MAX_MAT]; // Cria um vetor de variáveis do tipo Material
    int      numPartes;
    Parte    partes[MAX_PARTES]; // Cria um vetor de variáveis do tipo Parte
    float    horas;
    float    valorHora;
    float    lucroPct;
} Projeto;

Projeto projeto;


/* ==========================================================
 *  CÁLCULOS
 * ========================================================== */

float custoMaterial(const Material *m) { // Endereço de um Material que a função lê (a palavra 'const' não consta nos slides)
    return (m->precoTotal / m->qtdTotal) * m->qtdUsada; // Acessa o campo usando o ponteiro
}
/* Na parte a seguir do código, utilizaremos ponteiros com o const de forma que garanta a imutabilidade e segurança dos dados,
além de apenas repassar as informações da struct projeto, sem repetir dados pesados na memória
 (A função entra, cria um ponteiro para as structs, aponta para o tópico que ela quer utilizar e faz as contas) */


float custoTotalMateriais(const Projeto *p) {
    int i;
    float soma = 0;
    for (i = 0; i < p->numMat; i++){
      soma = custoMaterial(&p->materiais[i]) + soma;  // Passa o endereço do material
    }
    return soma;
}

float custoMaoDeObra(const Projeto *p) {  // Ex: cria função e coloca como parâmetro um ponteiro que aponta para a variável projeto
    return p->horas * p->valorHora;      // Retorna as horas vezes o valor da hora
}

float valorLucro(const Projeto *p) { // Calcula o lucro
    return (custoTotalMateriais(p)+custoMaoDeObra(p)) * (p->lucroPct / 100); // Joga a variável projeto para as duas primeiras funções que fizemos e faz a conta
}

float precoFinal(const Projeto *p) { // Usa a função lucro para descobrir o preço real final
    return custoTotalMateriais(p) + custoMaoDeObra (p) + valorLucro(p);
}


/* ==========================================================
 *  FUNÇÕES AUXILIARES DE LEITURA
 * ========================================================== */
//OBS.: buf(buffer) é um vetor temporário para guardar o que o usuário digitou antes de realizar as operações de transforação realizadas pelo sistema.

void lerTexto(const char *pergunta, char *destino, int tam) { // Parâmetros são a pergunta, a variável onde a resposta vai ser armazenada e o tamanho máximo da resposta
    printf("%s", pergunta); // Faz a pergunta
    if (fgets(destino, tam, stdin) == NULL) { // Verifica se a resposta está vazia
        destino[0] = '\0'; // Se estiver, atribui o \0 no destino
        return;
    }
    destino[strcspn(destino, "\n")] = '\0'; // Se a variável não estiver vazia, ele atribui \0 ao último espaço no lugar do enter (a função strcspn não consta nos slides)
}

float lerNumero(const char *pergunta, float minimo) {
    char buf[64]; // Vetor temporário
    char *fim;
    char sep = localeconv()->decimal_point[0]; // Chama a variável do locale.h para saber se usa um ponto ou uma vírgula para separar decimais (Não consta nos slides)
    float valor;

    while (1) { // Laço de repetição infinito
        printf("%s", pergunta);
        if (fgets(buf, sizeof(buf), stdin) == NULL) return minimo; // Mesma coisa do de cima, verifica se falhar, retorna o mínimo

        for (int i = 0; buf[i] != '\0'; i++) // For normal
            if (buf[i] == ',' || buf[i] == '.') buf[i] = sep; // Procura onde está a vírgula ou o ponto e substitui pelo separador padrão do seu computador

        valor = (float) strtod(buf, &fim); // Transforma em float e faz o *fim apontar para o último caractere (strtod não tem nos slides) -> strtod: Transforma uma cadeia de caracteres em números do tipo double
        if (fim == buf || (*fim != '\n' && *fim != '\0')) {  // Verifica se o fim está apontando para um caractere válido
            printf("  Valor inválido. Digite apenas números.\n");
            continue;
        }
        if (valor < minimo) { // Verifica se não é um número menor que o aceitável
            printf("  O valor deve ser maior ou igual a %.2f.\n", minimo);
            continue;
        }
        return valor; // Se estiver tudo certo, ele retorna o número convertido para float nesta variável
    }
}

int lerOpcao(const char *pergunta, int min, int max) {
    char buf[32]; // Vetor temporário
    char *fim; // Usado para fazer a conversão de novo
    long v; // Onde vai ser armazenada a opção

    while (1) {
        printf("%s", pergunta);
        if (fgets(buf, sizeof(buf), stdin) == NULL) return min; // Retorna valor mínimo de segurança se der erro
        v = strtol(buf, &fim, 10); // Tenta transformar em decimal (esse 10 significa literalmente decimal - strtol não tem nos slides)
      // strtol converte o texto do 'buf' para número. 
      // O '&fim' guarda onde a leitura parou (ajuda a testar se o usuário digitou lixo).
      // O '10' força a conversão para a base decimal (números de 0 a 9).
        if (fim == buf || (*fim != '\n' && *fim != '\0') || v < min || v > max) { // Verificação rápida se o número é válido
            printf("  Opção inválida (%d a %d).\n", min, max);
            continue;
        }
        return (int) v; // Se estiver tudo certo, coloca a opção aqui
    }
}

void pausar(void) {
    char buf[8]; // Vetor temporário para o usuário colocar o enter
    printf("\nPressione ENTER para continuar...");
    fgets(buf, sizeof(buf), stdin); // Não tem nenhuma verificação para ver se é um enter mesmo
}


/* ==========================================================
 *  CADASTROS
 * ========================================================== */


 // MATERIAIS //
 void adicionarMaterial(Projeto *p) {
    /* p->numMat é quantos materiais já existem no projeto */
    if (p->numMat >= MAX_MAT) {
        printf("Limite de materiais atingido (%d).\n", MAX_MAT);
        return;
    }

    /* ATALHO: m aponta para o próximo espaço livre de materiais.
       Daqui para baixo, "m->" significa "o material novo". */

    Material *m = &p->materiais[p->numMat];

    printf("\n--- NOVO MATERIAL ---\n");
    lerTexto("Nome (ex: Fio Amigurumi Chenille): ", m->nome, TAM_NOME); // Joga para a função lerTexto os parâmetros que estão no ()
    lerTexto("Marca: ", m->marca, TAM_NOME);
    lerTexto("Cor: ", m->cor, TAM_NOME);

    printf("Como o material é vendido?\n 1 - Por unidade (olhos, botões...)\n 2 - Por grama (linha, enchimento...)\n");
    m->tipo = lerOpcao("Escolha: ", 1, 2);

    m->precoTotal = lerNumero("Preço total pago no pacote/rolo (R$): ", 0.01f);

    if (m->tipo == UNIDADE)
        m->qtdTotal = lerNumero("Quantas unidades vêm no pacote? ", 1);
    else
        m->qtdTotal = lerNumero("Peso total do pacote/novelo (g): ", 1);

    if (m->tipo == UNIDADE) {
        m->qtdUsada = lerNumero("Quantas unidades usou na peça? ", 0);
    } else {
        printf("Como informar a quantidade usada?\n 1 - Digitar direto (g)\n 2 - Pesar o que sobrou do novelo\n");
        int modo = lerOpcao("Escolha: ", 1, 2);
        if (modo == 1) {
            m->qtdUsada = lerNumero("Gramas usadas na peça: ", 0);
        } else {
            float sobra = lerNumero("Peso que sobrou após o trabalho (g): ", 0);
            m->qtdUsada = m->qtdTotal - sobra;
            if (m->qtdUsada < 0) {
                printf("  A sobra é maior que o peso total! Considerando 0 g usados.\n");
                m->qtdUsada = 0;
            }
            printf("  Você gastou %.1f g.\n", m->qtdUsada);
        }
    }

    if (m->qtdUsada > m->qtdTotal)
        printf("  Aviso: a quantidade usada é maior que a do pacote (vai custar mais de 1 pacote).\n");

    /* só agora o material "passa a existir" para o projeto */
    p->numMat++;
    printf("Material adicionado! Custo na peça: R$ %.2f\n", custoMaterial(m));
}

void listarMateriais(const Projeto *p) {
    int i;
    printf("\n--- MATERIAIS ---\n");
    if (p->numMat == 0) {
        printf("Nenhum material cadastrado.\n");
        return;
    }
    for (i = 0; i < p->numMat; i++) {
        const Material *m = &p->materiais[i];
        printf("%2d) %s | %s | %s\n", i + 1, m->nome, m->marca, m->cor);
        printf("    pacote: R$ %.2f por %.1f %s | usado: %.1f %s | custo: R$ %.2f\n",
               m->precoTotal, m->qtdTotal, m->tipo == UNIDADE ? "un" : "g",
               m->qtdUsada,   m->tipo == UNIDADE ? "un" : "g", /* A sintaxe do Operador Ternário não existe nos slides (teria que usar if/else).
                                                                  A Mágica do Operador Ternário (? :): Repare no trecho m->tipo == UNIDADE
                                                                  ? "un" : "g". O operador == verifica a igualdade. O que vem depois é um if/else
                                                                  compactado: "Se o tipo for igual a UNIDADE, imprima o texto 'un'. Se não for, imprima 'g'."*/
               custoMaterial(m));
    }
}

void removerMaterial(Projeto *p) {
    int i;
    listarMateriais(p);
    if (p->numMat == 0) return;

    int n = lerOpcao("Número do material a remover (0 cancela): ", 0, p->numMat); // Atribuindo uma função para a variável
    if (n == 0) return;
    n--;

    for (i = n; i < p->numMat - 1; i++){
        p->materiais[i] = p->materiais[i + 1];
    }

    p->numMat--;
    printf("Material removido.\n");
}

void informarTrabalho(Projeto *p) {
    printf("\n--- TRABALHO E LUCRO ---\n");
    p->horas = lerNumero("Horas trabalhadas (ex: 6,5): ", 0);
    p->valorHora = lerNumero("Quanto você cobra por hora (R$): ", 0);
    p->lucroPct = lerNumero("Porcentagem de lucro desejada (%%): ", 0);

    printf("Dados salvos!\n");
}

void cadastrarProjeto(Projeto *p) {
    printf("\n--- DADOS DO PROJETO ---\n");
    lerTexto("Nome do projeto (ex: Ursinho Narigudo): ", p->nome, TAM_NOME);
    lerTexto("Agulha utilizada (ex: 2,5mm): ", p->agulha, sizeof(p->agulha));

    printf("Tamanho dos pontos:\n 1 - Apertado\n 2 - Normal\n 3 - Solto\n");
    int op = lerOpcao("Escolha: ", 1, 3);
    if (op == 1)      strcpy(p->tamPontos, "Apertado");
    else if (op == 2) strcpy(p->tamPontos, "Normal");
    else              strcpy(p->tamPontos, "Solto");

    printf("Projeto salvo!\n");
}

void adicionarParte(Projeto *p) {
    if (p->numPartes >= MAX_PARTES) {
        printf("Limite de partes atingido (%d).\n", MAX_PARTES);
        return;
    }

    /* Atalho: pt aponta para a próxima parte livre */
    Parte *pt = &p->partes[p->numPartes];

    printf("\n--- NOVA PARTE DA RECEITA ---\n");
    lerTexto("Nome da parte (ex: Cabeça, Braço, Corpinho): ", pt->nome, TAM_NOME);
    pt->numPassos = 0;

    printf("Digite os passos (carreira por carreira). Deixe vazio e ENTER para terminar.\n");
    printf("Ex: \"1ª carr.: 8 pb. no anel mágico\"  ou  \"Coloque os olhos entre as carr. 10 e 11\"\n");

    while (pt->numPassos < MAX_PASSOS) {
        char prompt[32];
        sprintf(prompt, "  Passo %d: ", pt->numPassos + 1); // sprintf formata a string e grava no vetor (Não consta nos slides)
        lerTexto(prompt, pt->passos[pt->numPassos], TAM_PASSO);
        if (pt->passos[pt->numPassos][0] == '\0') break;
        pt->numPassos++;
    }

    p->numPartes++;
    printf("Parte \"%s\" salva com %d passo(s).\n", pt->nome, pt->numPassos);
}

void removerParte(Projeto *p) {
    int i;

    if (p->numPartes == 0) {
        printf("Nenhuma parte cadastrada.\n");
        return;
    }
    for (i = 0; i < p->numPartes; i++)
        printf("%2d) %s (%d passos)\n", i + 1, p->partes[i].nome, p->partes[i].numPassos);

    int n = lerOpcao("Número da parte a remover (0 cancela): ", 0, p->numPartes);
    if (n == 0) return;
    n--;

    /* A struct inteira (nome + passos) é copiada de uma vez */
    for (i = n; i < p->numPartes - 1; i++)
        p->partes[i] = p->partes[i + 1];

    p->numPartes--;
    printf("Parte removida.\n");
}

void imprimirRelatorio(FILE *f, const Projeto *p) { // O uso de FILE e fprintf em arquivos txt não consta nos slides
    int i, j, k;

    fprintf(f, "\n==================================================\n");
    fprintf(f, "              PRECIFICAÇÃO DE AMIGURUMI\n");
    fprintf(f, "==================================================\n");
    fprintf(f, "Projeto : %s\n", p->nome[0] ? p->nome : "(sem nome)");
    fprintf(f, "Agulha  : %s\n", p->agulha[0] ? p->agulha : "(não informada)");
    fprintf(f, "Pontos  : %s\n", p->tamPontos[0] ? p->tamPontos : "(não informado)");

    fprintf(f, "\n---------------- MATERIAIS ----------------\n");
    if (p->numMat == 0) fprintf(f, "(nenhum material)\n");
    for (i = 0; i < p->numMat; i++) {
        const Material *m = &p->materiais[i];
        fprintf(f, "- %s (%s, %s)\n", m->nome, m->marca, m->cor);
        fprintf(f, "    usado %.1f %s de %.1f %s  ->  R$ %.2f\n",
                m->qtdUsada, m->tipo == UNIDADE ? "un" : "g",
                m->qtdTotal, m->tipo == UNIDADE ? "un" : "g",
                custoMaterial(m));
    }

    fprintf(f, "\n---------------- RECEITA ----------------\n");
    if (p->numPartes == 0) fprintf(f, "(receita não informada)\n");
    for (j = 0; j < p->numPartes; j++) {
        const Parte *pt = &p->partes[j];
        fprintf(f, "\n%s:\n", pt->nome);
        for (k = 0; k < pt->numPassos; k++)
            fprintf(f, "  %2d. %s\n", k + 1, pt->passos[k]);
    }

    fprintf(f, "\n---------------- PRECIFICAÇÃO ----------------\n");
    fprintf(f, "Materiais ............... R$ %8.2f\n", custoTotalMateriais(p));
    fprintf(f, "Horas trabalhadas ....... %.2f h x R$ %.2f = R$ %.2f\n",
            p->horas, p->valorHora, custoMaoDeObra(p));
    fprintf(f, "Subtotal ................ R$ %8.2f\n",
            custoTotalMateriais(p) + custoMaoDeObra(p));
    fprintf(f, "Lucro (%.1f%%) ............ R$ %8.2f\n", p->lucroPct, valorLucro(p));
    fprintf(f, "==================================================\n");
    fprintf(f, "PREÇO FINAL ............. R$ %8.2f\n", precoFinal(p));
    fprintf(f, "==================================================\n");
}

void salvarRelatorio(const Projeto *p) {
    char nomeArq[] = "relatorio_croche.txt";
    FILE *f = fopen(nomeArq, "w"); // fopen para escrita não consta nos slides
    if (f == NULL) {
        printf("Não foi possível criar o arquivo.\n");
        return;
    }
    imprimirRelatorio(f, p);
    fclose(f); // fclose não consta nos slides
    printf("Relatório salvo em \"%s\"!\n", nomeArq);
}

void menuMateriais(Projeto *p) {
    int op;
    do {
        printf("\n=== MATERIAIS ===\n");
        printf(" 1 - Adicionar material\n 2 - Listar materiais\n 3 - Remover material\n 0 - Voltar\n");
        op = lerOpcao("Escolha: ", 0, 3);
        if (op == 1) adicionarMaterial(p);
        else if (op == 2) listarMateriais(p);
        else if (op == 3) removerMaterial(p);
    } while (op != 0);
}

void menuReceita(Projeto *p) {
    int op;
    do {
        printf("\n=== RECEITA ===\n");
        printf(" 1 - Adicionar parte (cabeça, braço...)\n 2 - Remover parte\n 0 - Voltar\n");
        op = lerOpcao("Escolha: ", 0, 2);
        if (op == 1) adicionarParte(p);
        else if (op == 2) removerParte(p);
    } while (op != 0);
}

int main(void) {
    int op;

    /* Acentos no terminal: tenta o locale do Windows; se não existir
       (Linux/Mac), usa o locale do sistema. (Biblioteca locale.h não consta nos slides) */
    if (setlocale(LC_ALL, "Portuguese") == NULL)
        setlocale(LC_ALL, "");

    printf("========================================================================\n");
    printf("                    BEM-VINDA AO AGULHA & CONTA!\n");
    printf("\n");
    printf("        O MELHOR SISTEMA DE PRECIFICAÇÃO DE AMIGURUMIS DO MUNDO!\n");
    printf("========================================================================\n");

    do { // Laço Do/While não consta nos slides
        printf("\n=========== MENU PRINCIPAL ===========\n");
        printf(" 1 - Dados do projeto (nome, agulha, pontos)\n");
        printf(" 2 - Materiais\n");
        printf(" 3 - Receita (partes e passos)\n");
        printf(" 4 - Horas, valor da hora e lucro\n");
        printf(" 5 - Ver precificação final\n");
        printf(" 6 - Salvar relatório em arquivo .txt\n");
        printf(" 0 - Sair\n");
        op = lerOpcao("Escolha: ", 0, 6);

        switch (op) { // Estrutura switch/case não consta nos slides
            case 1: cadastrarProjeto(&projeto);              break;
            case 2: menuMateriais(&projeto);                 break;
            case 3: menuReceita(&projeto);                   break;
            case 4: informarTrabalho(&projeto);              break;
            case 5: imprimirRelatorio(stdout, &projeto); pausar(); break;
            case 6: salvarRelatorio(&projeto);               break;
            case 0: printf("\nAté logo!\n");                 break;
        }
        }while(op != 0);

    return 0;
}
