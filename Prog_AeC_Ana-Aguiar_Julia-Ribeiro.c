/*
 * ==========================================================
 *  PRECIFICADOR DE CROCHÊ
 *  Linguagem de Programação - Sugestão de Projeto
 *
 *  Fórmula:
 *    custo dos materiais (só a parte usada!)
 *  + horas trabalhadas * valor da hora
 *  = subtotal
 *  + % de lucro sobre o subtotal
 *  = PREÇO FINAL
 *
 *  Sem struct: usa vetores paralelos (a posição i de cada
 *  vetor pertence ao mesmo material / à mesma parte).
 *
 *  Compilar:  gcc precificacao_croche.c -o croche
 *  Executar:  ./croche   (no Windows: croche.exe)
 *
 *  Acentos: usa setlocale(). No Windows, salve este arquivo
 *  como ANSI (Windows-1252). Em Linux/Mac, use a versão UTF-8.
 * ==========================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>   /* setlocale: acentos no terminal */

#define MAX_MAT     30
#define MAX_PARTES  10
#define MAX_PASSOS  30
#define TAM_NOME    60
#define TAM_PASSO   200

#define UNIDADE 1
#define GRAMA   2

/* ---------- DADOS DO PROJETO ---------- */
char projNome[TAM_NOME]  = "";
char agulha[20]          = "";
char tamPontos[20]       = "";

/* ---------- MATERIAIS (vetores paralelos) ---------- */
int   numMat = 0;
char  matNome[MAX_MAT][TAM_NOME];
char  matMarca[MAX_MAT][TAM_NOME];
char  matCor[MAX_MAT][TAM_NOME];
int   matTipo[MAX_MAT];          /* UNIDADE ou GRAMA            */
float matPrecoTotal[MAX_MAT];    /* quanto pagou no pacote/rolo */
float matQtdTotal[MAX_MAT];      /* quanto vem no pacote/rolo   */
float matQtdUsada[MAX_MAT];      /* quanto usou na peça         */

/* ---------- RECEITA ---------- */
int  numPartes = 0;
char parteNome[MAX_PARTES][TAM_NOME];
int  numPassos[MAX_PARTES];
char passos[MAX_PARTES][MAX_PASSOS][TAM_PASSO];

/* ---------- TRABALHO E LUCRO ---------- */
float horas     = 0;
float valorHora = 0;
float lucroPct  = 0;

/* ==========================================================
 *  FUNÇÕES AUXILIARES DE LEITURA
 * ========================================================== */

void lerTexto(const char *pergunta, char *destino, int tam) {
    printf("%s", pergunta);
    if (fgets(destino, tam, stdin) == NULL) {
        destino[0] = '\0';
        return;
    }
    destino[strcspn(destino, "\n")] = '\0';
}

/* Lê um número real. Aceita vírgula (2,5) ou ponto (2.5).
   Só aceita valores >= minimo. */
float lerNumero(const char *pergunta, float minimo) {
    char buf[64];
    char *fim;
    char sep = localeconv()->decimal_point[0];
    float valor;

    while (1) {
        printf("%s", pergunta);
        if (fgets(buf, sizeof(buf), stdin) == NULL) return minimo;

        for (int i = 0; buf[i] != '\0'; i++)
            if (buf[i] == ',' || buf[i] == '.') buf[i] = sep;

        valor = (float) strtod(buf, &fim);
        if (fim == buf || (*fim != '\n' && *fim != '\0')) {
            printf("  Valor inválido. Digite apenas números.\n");
            continue;
        }
        if (valor < minimo) {
            printf("  O valor deve ser maior ou igual a %.2f.\n", minimo);
            continue;
        }
        return valor;
    }
}

int lerOpcao(const char *pergunta, int min, int max) {
    char buf[32];
    char *fim;
    long v;

    while (1) {
        printf("%s", pergunta);
        if (fgets(buf, sizeof(buf), stdin) == NULL) return min;
        v = strtol(buf, &fim, 10);
        if (fim == buf || (*fim != '\n' && *fim != '\0') || v < min || v > max) {
            printf("  Opção inválida (%d a %d).\n", min, max);
            continue;
        }
        return (int) v;
    }
}

void pausar(void) {
    printf("\nPressione ENTER para continuar...");
    char buf[8];
    fgets(buf, sizeof(buf), stdin);
}

/* ==========================================================
 *  CÁLCULOS
 * ========================================================== */

/* Custo proporcional: só cobra o que foi realmente usado.
   Ex.: rolo de R$ 30 com 200 g, usou 50 g -> R$ 7,50 */
float custoMaterial(int i) {
    return (matPrecoTotal[i] / matQtdTotal[i]) * matQtdUsada[i];
}

float custoTotalMateriais(void) {
    float soma = 0;
    for (int i = 0; i < numMat; i++) soma += custoMaterial(i);
    return soma;
}

float custoMaoDeObra(void) {
    return horas * valorHora;
}

float valorLucro(void) {
    return (custoTotalMateriais() + custoMaoDeObra()) * (lucroPct / 100.0f);
}

float precoFinal(void) {
    return custoTotalMateriais() + custoMaoDeObra() + valorLucro();
}

/* ==========================================================
 *  CADASTROS
 * ========================================================== */

void cadastrarProjeto(void) {
    printf("\n--- DADOS DO PROJETO ---\n");
    lerTexto("Nome do projeto (ex: Ursinho Narigudo): ", projNome, TAM_NOME);
    lerTexto("Agulha utilizada (ex: 2,5mm): ", agulha, sizeof(agulha));

    printf("Tamanho dos pontos:\n 1 - Apertado\n 2 - Normal\n 3 - Solto\n");
    int op = lerOpcao("Escolha: ", 1, 3);
    if (op == 1)      strcpy(tamPontos, "Apertado");
    else if (op == 2) strcpy(tamPontos, "Normal");
    else              strcpy(tamPontos, "Solto");

    printf("Projeto salvo!\n");
}

void adicionarMaterial(void) {
    if (numMat >= MAX_MAT) {
        printf("Limite de materiais atingido (%d).\n", MAX_MAT);
        return;
    }
    int i = numMat;

    printf("\n--- NOVO MATERIAL ---\n");
    lerTexto("Nome (ex: Fio Amigurumi Chenille): ", matNome[i], TAM_NOME);
    lerTexto("Marca: ", matMarca[i], TAM_NOME);
    lerTexto("Cor: ", matCor[i], TAM_NOME);

    printf("Como o material é vendido?\n 1 - Por unidade (olhos, botões...)\n 2 - Por grama (linha, enchimento...)\n");
    matTipo[i] = lerOpcao("Escolha: ", 1, 2);

    matPrecoTotal[i] = lerNumero("Preço total pago no pacote/rolo (R$): ", 0.01f);

    if (matTipo[i] == UNIDADE)
        matQtdTotal[i] = lerNumero("Quantas unidades vêm no pacote? ", 1);
    else
        matQtdTotal[i] = lerNumero("Peso total do pacote/novelo (g): ", 1);

    if (matTipo[i] == UNIDADE) {
        matQtdUsada[i] = lerNumero("Quantas unidades usou na peça? ", 0);
    } else {
        printf("Como informar a quantidade usada?\n 1 - Digitar direto (g)\n 2 - Pesar o que sobrou do novelo\n");
        int modo = lerOpcao("Escolha: ", 1, 2);
        if (modo == 1) {
            matQtdUsada[i] = lerNumero("Gramas usadas na peça: ", 0);
        } else {
            float sobra = lerNumero("Peso que sobrou após o trabalho (g): ", 0);
            matQtdUsada[i] = matQtdTotal[i] - sobra;
            if (matQtdUsada[i] < 0) {
                printf("  A sobra é maior que o peso total! Considerando 0 g usados.\n");
                matQtdUsada[i] = 0;
            }
            printf("  Você gastou %.1f g.\n", matQtdUsada[i]);
        }
    }

    if (matQtdUsada[i] > matQtdTotal[i])
        printf("  Aviso: a quantidade usada é maior que a do pacote (vai custar mais de 1 pacote).\n");

    numMat++;
    printf("Material adicionado! Custo na peça: R$ %.2f\n", custoMaterial(i));
}

void listarMateriais(void) {
    printf("\n--- MATERIAIS ---\n");
    if (numMat == 0) {
        printf("Nenhum material cadastrado.\n");
        return;
    }
    for (int i = 0; i < numMat; i++) {
        printf("%2d) %s | %s | %s\n", i + 1, matNome[i], matMarca[i], matCor[i]);
        printf("    pacote: R$ %.2f por %.1f %s | usado: %.1f %s | custo: R$ %.2f\n",
               matPrecoTotal[i], matQtdTotal[i], matTipo[i] == UNIDADE ? "un" : "g",
               matQtdUsada[i], matTipo[i] == UNIDADE ? "un" : "g",
               custoMaterial(i));
    }
}

void removerMaterial(void) {
    listarMateriais();
    if (numMat == 0) return;

    int n = lerOpcao("Número do material a remover (0 cancela): ", 0, numMat);
    if (n == 0) return;
    n--;

    /* puxa todos os itens seguintes uma posicao para tras */
    for (int i = n; i < numMat - 1; i++) {
        strcpy(matNome[i], matNome[i + 1]);
        strcpy(matMarca[i], matMarca[i + 1]);
        strcpy(matCor[i], matCor[i + 1]);
        matTipo[i]       = matTipo[i + 1];
        matPrecoTotal[i] = matPrecoTotal[i + 1];
        matQtdTotal[i]   = matQtdTotal[i + 1];
        matQtdUsada[i]   = matQtdUsada[i + 1];
    }
    numMat--;
    printf("Material removido.\n");
}

void adicionarParte(void) {
    if (numPartes >= MAX_PARTES) {
        printf("Limite de partes atingido (%d).\n", MAX_PARTES);
        return;
    }
    int p = numPartes;

    printf("\n--- NOVA PARTE DA RECEITA ---\n");
    lerTexto("Nome da parte (ex: Cabeça, Braço, Corpinho): ", parteNome[p], TAM_NOME);
    numPassos[p] = 0;

    printf("Digite os passos (carreira por carreira). Deixe vazio e ENTER para terminar.\n");
    printf("Ex: \"1ª carr.: 8 pb. no anel mágico\"  ou  \"Coloque os olhos entre as carr. 10 e 11\"\n");

    while (numPassos[p] < MAX_PASSOS) {
        char prompt[32];
        sprintf(prompt, "  Passo %d: ", numPassos[p] + 1);
        lerTexto(prompt, passos[p][numPassos[p]], TAM_PASSO);
        if (passos[p][numPassos[p]][0] == '\0') break;
        numPassos[p]++;
    }
    numPartes++;
    printf("Parte \"%s\" salva com %d passo(s).\n", parteNome[p], numPassos[p]);
}

void removerParte(void) {
    if (numPartes == 0) {
        printf("Nenhuma parte cadastrada.\n");
        return;
    }
    for (int i = 0; i < numPartes; i++)
        printf("%2d) %s (%d passos)\n", i + 1, parteNome[i], numPassos[i]);

    int n = lerOpcao("Número da parte a remover (0 cancela): ", 0, numPartes);
    if (n == 0) return;
    n--;

    for (int i = n; i < numPartes - 1; i++) {
        strcpy(parteNome[i], parteNome[i + 1]);
        numPassos[i] = numPassos[i + 1];
        for (int j = 0; j < numPassos[i + 1]; j++)
            strcpy(passos[i][j], passos[i + 1][j]);
    }
    numPartes--;
    printf("Parte removida.\n");
}

void informarTrabalho(void) {
    printf("\n--- TRABALHO E LUCRO ---\n");
    horas     = lerNumero("Horas trabalhadas (ex: 6,5): ", 0);
    valorHora = lerNumero("Quanto você cobra por hora (R$): ", 0);
    lucroPct  = lerNumero("Porcentagem de lucro desejada (%): ", 0);
    printf("Dados salvos!\n");
}

/* ==========================================================
 *  SAÍDA (serve para a tela E para o arquivo)
 * ========================================================== */

void imprimirRelatorio(FILE *f) {
    fprintf(f, "\n==================================================\n");
    fprintf(f, "              PRECIFICAÇÃO DE AMIGURUMI\n");
    fprintf(f, "==================================================\n");
    fprintf(f, "Projeto : %s\n", projNome[0] ? projNome : "(sem nome)");
    fprintf(f, "Agulha  : %s\n", agulha[0] ? agulha : "(não informada)");
    fprintf(f, "Pontos  : %s\n", tamPontos[0] ? tamPontos : "(não informado)");

    fprintf(f, "\n---------------- MATERIAIS ----------------\n");
    if (numMat == 0) fprintf(f, "(nenhum material)\n");
    for (int i = 0; i < numMat; i++) {
        fprintf(f, "- %s (%s, %s)\n", matNome[i], matMarca[i], matCor[i]);
        fprintf(f, "    usado %.1f %s de %.1f %s  ->  R$ %.2f\n",
                matQtdUsada[i], matTipo[i] == UNIDADE ? "un" : "g",
                matQtdTotal[i], matTipo[i] == UNIDADE ? "un" : "g",
                custoMaterial(i));
    }

    fprintf(f, "\n---------------- RECEITA ----------------\n");
    if (numPartes == 0) fprintf(f, "(receita não informada)\n");
    for (int p = 0; p < numPartes; p++) {
        fprintf(f, "\n%s:\n", parteNome[p]);
        for (int j = 0; j < numPassos[p]; j++)
            fprintf(f, "  %2d. %s\n", j + 1, passos[p][j]);
    }

    fprintf(f, "\n---------------- PRECIFICAÇÃO ----------------\n");
    fprintf(f, "Materiais ............... R$ %8.2f\n", custoTotalMateriais());
    fprintf(f, "Horas trabalhadas ....... %.2f h x R$ %.2f = R$ %.2f\n",
            horas, valorHora, custoMaoDeObra());
    fprintf(f, "Subtotal ................ R$ %8.2f\n",
            custoTotalMateriais() + custoMaoDeObra());
    fprintf(f, "Lucro (%.2f%%) ............ R$ %8.2f\n", lucroPct, valorLucro());
    fprintf(f, "==================================================\n");
    fprintf(f, "PREÇO FINAL ............. R$ %8.2f\n", precoFinal());
    fprintf(f, "==================================================\n");
}

void salvarRelatorio(void) {
    char nomeArq[] = "relatorio_croche.txt";
    FILE *f = fopen(nomeArq, "w");
    if (f == NULL) {
        printf("Não foi possível criar o arquivo.\n");
        return;
    }
    imprimirRelatorio(f);
    fclose(f);
    printf("Relatório salvo em \"%s\"!\n", nomeArq);
}

/* ==========================================================
 *  MENUS
 * ========================================================== */

void menuMateriais(void) {
    int op;
    do {
        printf("\n=== MATERIAIS ===\n");
        printf(" 1 - Adicionar material\n 2 - Listar materiais\n 3 - Remover material\n 0 - Voltar\n");
        op = lerOpcao("Escolha: ", 0, 3);
        if (op == 1) adicionarMaterial();
        else if (op == 2) listarMateriais();
        else if (op == 3) removerMaterial();
    } while (op != 0);
}

void menuReceita(void) {
    int op;
    do {
        printf("\n=== RECEITA ===\n");
        printf(" 1 - Adicionar parte (cabeça, braço...)\n 2 - Remover parte\n 0 - Voltar\n");
        op = lerOpcao("Escolha: ", 0, 2);
        if (op == 1) adicionarParte();
        else if (op == 2) removerParte();
    } while (op != 0);
}

int main(void) {
    int op;

    /* Acentos no terminal: tenta o locale do Windows; se não existir
       (Linux/Mac), usa o locale do sistema. */
    if (setlocale(LC_ALL, "Portuguese") == NULL)
        setlocale(LC_ALL, "");

    printf("========================================================================\n");
    printf("                    BEM-VINDA AO AGULHA & CONTA!\n");
    printf("\n");
    printf("        O MELHOR SISTEMA DE PRECIFICAÇÃO DE AMIGURUMIS DO MUNDO!\n");
    printf("========================================================================\n");

    do {
        printf("\n=========== MENU PRINCIPAL ===========\n");
        printf(" 1 - Dados do projeto (nome, agulha, pontos)\n");
        printf(" 2 - Materiais\n");
        printf(" 3 - Receita (partes e passos)\n");
        printf(" 4 - Horas, valor da hora e lucro\n");
        printf(" 5 - Ver precificação final\n");
        printf(" 6 - Salvar relatório em arquivo .txt\n");
        printf(" 0 - Sair\n");
        op = lerOpcao("Escolha: ", 0, 6);

        switch (op) {
            case 1: cadastrarProjeto(); break;
            case 2: menuMateriais();    break;
            case 3: menuReceita();      break;
            case 4: informarTrabalho(); break;
            case 5: imprimirRelatorio(stdout); pausar(); break;
            case 6: salvarRelatorio();  break;
            case 0: printf("\nAté logo!\n"); break;
        }
    } while (op != 0);

    return 0;
}
