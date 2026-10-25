#include <stdio.h>
#include <stdlib.h>

/* Comando de limpar tela conforme o sistema */
#ifdef _WIN32
#define LIMPAR "cls"
#else
#define LIMPAR "clear"
#endif

/* Prototipos das funcoes (as operacoes do menu) */
void opcao_iniciar(void);
void opcao_sobre(void);

int main(void) {
    char entrada[16];
    int opcao = -1;

    while (opcao != 0) {

        /* ---------- TELA INICIAL ---------- */
        system(LIMPAR);
        printf("==========================================\n");
        printf("|                                        |\n");
        printf("|            Agulha e Conta              |\n");
        printf("|              Versao 1.0                |\n");
        printf("|                                        |\n");
        printf("==========================================\n");
        printf("|                                        |\n");
        printf("|   1 - Iniciar                          |\n");
        printf("|   2 - Sobre                            |\n");
        printf("|   0 - Sair                             |\n");
        printf("|                                        |\n");
        printf("==========================================\n");
        printf("\nEscolha uma opcao: ");

        /* ---------- LEITURA DA OPCAO ---------- */
        if (fgets(entrada, sizeof(entrada), stdin) == NULL) break;
        if (sscanf(entrada, "%d", &opcao) != 1) opcao = -1;

        /* ---------- CHAMA AS OPERACOES ---------- */
        switch (opcao) {
            case 1:
                opcao_iniciar();
                break;

            case 2:
                opcao_sobre();
                break;

            case 0:
                printf("\nAte logo!\n");
                break;

            default:
                printf("\nOpcao invalida!\n");
                printf("\nPressione ENTER para tentar de novo...");
                while (getchar() != '\n');
        }
    }

    return 0;
}

/* ---------- DEFINICAO DAS FUNCOES ---------- */

void opcao_iniciar(void) {
    system(LIMPAR);
    printf("=== INICIAR ===\n");
    printf("Aqui entra a logica principal do seu programa.\n");
    printf("\nPressione ENTER para voltar ao menu...");
    while (getchar() != '\n');
}

void opcao_sobre(void) {
    system(LIMPAR);
    printf("=== SOBRE ===\n");
    printf("Programa de exemplo feito em C.\n");
    printf("\nPressione ENTER para voltar ao menu...");
    while (getchar() != '\n');
}
