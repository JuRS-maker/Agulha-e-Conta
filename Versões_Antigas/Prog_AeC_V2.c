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

typedef struct {
    char  nome[TAM_NOME];
    char  marca[TAM_NOME];
    char  cor[TAM_NOME];
    int   tipo;          /* UNIDADE ou GRAMA            */
    float precoTotal;    /* quanto pagou no pacote/rolo */
    float qtdTotal;      /* quanto vem no pacote/rolo   */
    float qtdUsada;      /* quanto usou na peça         */
} Material;

typedef struct {
    char nome[TAM_NOME];
    int  numPassos;
    char passos[MAX_PASSOS][TAM_PASSO];   /* matriz: cada linha é um passo */
} Parte;

typedef struct {
    char     nome[TAM_NOME];
    char     agulha[20];
    char     tamPontos[20];
    int      numMat;
    Material materiais[MAX_MAT];
    int      numPartes;
    Parte    partes[MAX_PARTES];
    float    horas;
    float    valorHora;
    float    lucroPct;
} Projeto;

Projeto projeto;


/* ==========================================================
 *  CÁLCULOS
 * ========================================================== */

float custoMaterial(const Material *m) { //endereço de um Material que a função lê
    return (m->precoTotal / m->qtdTotal) * m->qtdUsada; //acessar o campo do ponteiro
}

float custoTotalMateriais(const Projeto *p) {
    int i;
    float soma = 0;
    for (i = 0; i < p->numMat; i++){
      soma = custoMaterial(&p->materiais[i]) + soma;  //Passa o endereço do materaial
    }
    return soma;
}

float custoMaoDeObra(const Projeto *p) {
    return p->horas * p->valorHora;
}

float valorLucro(const Projeto *p) {
    return (custoTotalMateriais(p)+custoMaoDeObra(p)) * (p->lucroPct / 100);
}

float precoFinal(const Projeto *p) {
    return custoTotalMateriais(p) + custoMaoDeObra (p) + valorLucro(p);
}


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
    char buf[8];
    printf("\nPressione ENTER para continuar...");
    fgets(buf, sizeof(buf), stdin);
}


/* ==========================================================
 *  CADASTROS
 * ========================================================== */

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
    lerTexto("Nome (ex: Fio Amigurumi Chenille): ", m->nome, TAM_NOME);
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
               m->qtdUsada,   m->tipo == UNIDADE ? "un" : "g",
               custoMaterial(m));
    }
}

void removerMaterial(Projeto *p) {
    int i;
    listarMateriais(p);
    if (p->numMat == 0) return;

    int n = lerOpcao("Número do material a remover (0 cancela): ", 0, p->numMat);
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
    p->lucroPct = lerNumero("Porcentagem de lucro desejada (%): ", 0);

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

    /* atalho: pt aponta para a próxima parte livre */
    Parte *pt = &p->partes[p->numPartes];

    printf("\n--- NOVA PARTE DA RECEITA ---\n");
    lerTexto("Nome da parte (ex: Cabeça, Braço, Corpinho): ", pt->nome, TAM_NOME);
    pt->numPassos = 0;

    printf("Digite os passos (carreira por carreira). Deixe vazio e ENTER para terminar.\n");
    printf("Ex: \"1ª carr.: 8 pb. no anel mágico\"  ou  \"Coloque os olhos entre as carr. 10 e 11\"\n");

    while (pt->numPassos < MAX_PASSOS) {
        char prompt[32];
        sprintf(prompt, "  Passo %d: ", pt->numPassos + 1);
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

    /* a struct inteira (nome + passos) é copiada de uma vez */
    for (i = n; i < p->numPartes - 1; i++)
        p->partes[i] = p->partes[i + 1];

    p->numPartes--;
    printf("Parte removida.\n");
}

void imprimirRelatorio(FILE *f, const Projeto *p) {
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
    FILE *f = fopen(nomeArq, "w");
    if (f == NULL) {
        printf("Não foi possível criar o arquivo.\n");
        return;
    }
    imprimirRelatorio(f, p);
    fclose(f);
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
