#include <stdio.h>
#include <stdlib.h>   /* qsort() e bsearch() também ficam aqui (não constam nos slides como uso de ponteiro para função) */
#include <string.h>
#include <ctype.h>    /* tolower(): usada para comparar textos sem diferenciar maiúsculas/minúsculas (não consta nos slides) */
#include <locale.h>   /* Permite ajustar o idioma do terminal para aceitar
                         pontuação decimal da língua portuguesa.
                         (Não explicado nos arquivos de aula) */

#ifdef _WIN32
#include <windows.h>  /* SetConsoleOutputCP / SetConsoleCP: colocam o console em UTF-8 (só existe no Windows) */
#endif

#define MAX_PROJETOS 50 // Máximo de projetos que o sistema guarda ao mesmo tempo
#define MAX_MAT     30 // Máximo de materiais
#define MAX_PARTES  10 // Máximo de partes
#define MAX_PASSOS  30 // Máximo de carreiras por parte
#define TAM_NOME    60 // Máximo de caracteres dos textos
#define TAM_PASSO   200 // Tamanho máximo dos textos de cada instrução

#define ARQ_BINARIO "dados.bin"   /* Nome do arquivo de persistência (o usuário nunca vê esse nome) */

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

/* IMPORTANTE (persistência): a struct Projeto NÃO tem nenhum ponteiro dentro dela,
   apenas vetores de tamanho fixo, int e float. Por isso ela ocupa um bloco ÚNICO e contínuo
   de memória, e podemos gravá-la inteira com UM fwrite e lê-la inteira com UM fread.
   (Se houvesse char* dentro dela, gravaríamos só o endereço, e não o texto.) */

/* ---------------------------------------------------------------
 * MÚLTIPLOS PROJETOS: VETOR ESTÁTICO
 *
 * Existe um VETOR com MAX_PROJETOS (50) variáveis do tipo Projeto, uma colada na outra
 * na memória.
 *
 * COMO A MEMÓRIA É ALOCADA:
 *   - Por ser variável GLOBAL, o vetor é alocado UMA VEZ, quando o programa começa, em
 *     uma área de memória fixa (área de dados estáticos), e vive até o programa acabar.
 *     Não usamos malloc: o tamanho é decidido em tempo de compilação.
 *   - Tamanho total = 50 x sizeof(Projeto) = cerca de 3,3 MB.
 *     Isso só é possível porque o vetor é GLOBAL: uma variável LOCAL desse tamanho
 *     estouraria a pilha (stack) da função.
 *   - Variáveis globais começam ZERADAS: todos os textos começam vazios ("\0") e todos
 *     os números começam com 0. Então um projeto ainda não usado já está "limpo".
 *
 * COMO OS ÍNDICES FUNCIONAM:
 *   - O vetor tem 50 espaços, mas só os primeiros 'numProjetos' estão EM USO:
 *       projetos[0] ... projetos[numProjetos - 1]  -> projetos cadastrados
 *       projetos[numProjetos]                      -> PRÓXIMO espaço livre
 *   - Para mexer em um projeto, passamos o ENDEREÇO dele: &projetos[i]. As funções
 *     que recebem "Projeto *p" não sabem se esse projeto é o 0, o 7 ou o 49, só
 *     recebem o endereço de um Projeto.
 * --------------------------------------------------------------- */
Projeto projetos[MAX_PROJETOS];
int     numProjetos = 0;

/* Projeto "em branco" (zerado, por ser global). É copiado para um espaço do vetor
   antes de cadastrar um projeto novo, garantindo que nenhum lixo antigo fique lá. */
Projeto projetoVazio;

/* Controle da ordenação: 1 = vetor de materiais está em ordem alfabética (bsearch pode ser usado);
   0 = não está (precisa ordenar antes). Zera sempre que a ordem pode ter sido quebrada.
   COM VÁRIOS PROJETOS: esta variável é uma só, mas cada projeto tem o seu próprio vetor de
   materiais. Por isso ela também é zerada ao TROCAR de projeto (veja selecionarProjeto). */
int materiaisOrdenados = 0;


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

void lerTexto(const char *pergunta, char *destino, int tam) { // Parâmetros são a pergunta, a variável onde a resposta vai ser armazenada e o tamanho máximo da resposta
    printf("%s", pergunta); // Faz a pergunta
    if (fgets(destino, tam, stdin) == NULL) { // Verifica se a resposta está vazia
        destino[0] = '\0'; // Se estiver, atribui o \0 no destino
        return;
    }
    destino[strcspn(destino, "\n")] = '\0'; // Se a variável não estiver vazia, ele atribui \0 ao último espaço no lugar do enter (a função strcspn não consta nos slides)
}

/* ---------------------------------------------------------------
 * lerTextoObrigatorio: igual ao lerTexto, mas NÃO aceita texto vazio.
 *
 * LÓGICA:
 *   - Fica dentro de um while(1): só sai (com return) quando o usuário digita pelo menos
 *     um caractere que não seja espaço. Caso contrário mostra o erro e repete a pergunta.
 *   - Se o usuário aperta só ENTER, o fgets grava "\n" em destino. Depois que o "\n" é
 *     trocado por '\0' (mesma técnica do lerTexto), a string fica vazia: destino[0] == '\0'.
 *   - Também recusa texto feito só de espaços ("   "): o laço for procura alguma letra
 *     que não seja espaço (isspace, da ctype.h, não consta nos slides).
 *
 * PONTEIROS (passagem por referência):
 *   - 'destino' é o ENDEREÇO do vetor de char do chamador (ex.: p->nome). O fgets escreve
 *     os caracteres digitados DIRETO nesse endereço; por isso a função é void e mesmo
 *     assim o chamador recebe o texto.
 *   - 'pergunta' é const char *: a função só lê o texto da pergunta.
 *
 * FIM DE ARQUIVO (EOF): se a entrada fechar (fgets devolve NULL), repetir a pergunta
 * seria um laço infinito. Nesse caso gravamos "Sem nome" com strcpy e saímos.
 * --------------------------------------------------------------- */
void lerTextoObrigatorio(const char *pergunta, char *destino, int tam) {
    int i, temConteudo;

    while (1) {
        printf("%s", pergunta);
        if (fgets(destino, tam, stdin) == NULL) {   /* entrada encerrada: evita laço infinito */
            strcpy(destino, "Sem nome");
            return;
        }
        destino[strcspn(destino, "\n")] = '\0';      /* troca o ENTER por '\0' */

        temConteudo = 0;
        for (i = 0; destino[i] != '\0'; i++) {
            if (!isspace((unsigned char) destino[i])) {
                temConteudo = 1;                     /* achou um caractere de verdade */
                break;
            }
        }

        if (temConteudo) return;                     /* texto válido: sai do while */
        printf("  Este campo é obrigatório. Digite pelo menos uma letra.\n");
    }
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

        valor = (float) strtod(buf, &fim); // Transforma em float e faz o *fim apontar para o último caractere (strtod não tem nos slides)
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
        if (fim == buf || (*fim != '\n' && *fim != '\0') || v < min || v > max) { // Verificação rápida se o número é válido
            printf("  Opção inválida. Digite um número de %d a %d.\n", min, max);
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

void listarMateriais(const Projeto *p);   /* protótipo: a função é definida mais abaixo, mas é usada aqui */

/* ---------------------------------------------------------------
 * reaproveitarMaterial: cria um material NOVO no projeto copiando os dados de base
 * (nome, marca, cor, tipo, preço total e quantidade total) de um material JÁ cadastrado
 * no mesmo projeto. Só a "quantidade usada" é perguntada de novo.
 * Exemplo: o mesmo novelo rosa usado na cabeça (20 g) e no corpo (35 g).
 *
 * COMO O C COPIA AS STRINGS:
 *   - Em C, um texto é um vetor de char terminado em '\0'. Não dá para copiar vetor com
 *     "=": escrever novo->nome = origem->nome é ERRO de compilação, porque o nome de um
 *     vetor é um endereço fixo, não uma variável que aceita atribuição.
 *   - strcpy(destino, origem) copia caractere por caractere, do primeiro até o '\0' (o '\0'
 *     também é copiado). Depois disso, destino e origem têm o mesmo texto, mas em
 *     lugares DIFERENTES da memória: mudar um não muda o outro.
 *   - strcpy NÃO confere o tamanho do destino. Aqui é seguro porque origem e destino
 *     são o mesmo campo (nome, marca, cor) de duas variáveis Material, ambos com
 *     TAM_NOME caracteres, e o texto de origem já cabia lá.
 *   - tipo, precoTotal e qtdTotal são números (int/float), não strings. Para eles
 *     usamos a atribuição normal "=", que copia o valor. (strcpy só serve para texto.)
 *   - 'origem' aponta para o material escolhido (&p->materiais[n - 1]) e é const,
 *     pois só será lido. 'novo' aponta para o próximo espaço livre do vetor de materiais
 *     (&p->materiais[p->numMat]) e é por onde escrevemos.
 *   - Retorna 1 se criou o material, 0 se o usuário cancelou.
 * --------------------------------------------------------------- */
int reaproveitarMaterial(Projeto *p) {
    listarMateriais(p);
    int n = lerOpcao("Número do material a reaproveitar (0 cancela): ", 0, p->numMat);
    if (n == 0) return 0;

    const Material *origem = &p->materiais[n - 1];   /* a lista começa em 1, o vetor em 0 */
    Material *novo = &p->materiais[p->numMat];       /* próximo espaço livre */

    strcpy(novo->nome,  origem->nome);               /* texto: strcpy copia até o '\0' */
    strcpy(novo->marca, origem->marca);
    strcpy(novo->cor,   origem->cor);
    novo->tipo       = origem->tipo;                 /* números: atribuição simples */
    novo->precoTotal = origem->precoTotal;
    novo->qtdTotal   = origem->qtdTotal;

    printf("\nReaproveitando: %s | %s | %s\n", novo->nome, novo->marca, novo->cor);

    /* Única informação pedida: quanto deste material vai na peça atual */
    if (novo->tipo == UNIDADE)
        novo->qtdUsada = lerNumero("Quantas unidades usou nesta peça? ", 0);
    else
        novo->qtdUsada = lerNumero("Gramas usadas nesta peça: ", 0);

    if (novo->qtdUsada > novo->qtdTotal)
        printf("  Aviso: a quantidade usada é maior que a do pacote (vai custar mais de 1 pacote).\n");

    p->numMat++;               /* só agora o material "passa a existir" no projeto */
    materiaisOrdenados = 0;    /* entrou no fim do vetor: a ordem alfabética foi quebrada */
    printf("Material adicionado! Custo na peça: R$ %.2f\n", custoMaterial(novo));
    return 1;
}

 void adicionarMaterial(Projeto *p) {
    /* p->numMat é quantos materiais já existem no projeto */
    if (p->numMat >= MAX_MAT) {
        printf("Limite de materiais atingido (%d).\n", MAX_MAT);
        return;
    }

    /* Novo ou reaproveitar? A opção de reaproveitar só aparece se o projeto já tem material */
    if (p->numMat > 0) {
        printf("\nO que deseja fazer?\n 1 - Cadastrar material novo\n 2 - Reaproveitar material já cadastrado neste projeto\n");
        if (lerOpcao("Escolha: ", 1, 2) == 2) {
            reaproveitarMaterial(p);
            return;            /* o material já foi criado por reaproveitarMaterial */
        }
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
    materiaisOrdenados = 0; /* o novo material entrou no fim do vetor: a ordem alfabética foi quebrada */
    printf("Material adicionado! Custo na peça: R$ %.2f\n", custoMaterial(m));
}

/* Mostra UM material na tela. Recebe só o endereço (const: apenas lê, não altera). */
void mostrarMaterial(const Material *m, int numero) {
    printf("%2d) %s | %s | %s\n", numero, m->nome, m->marca, m->cor);
    printf("    pacote: R$ %.2f por %.1f %s | usado: %.1f %s | custo: R$ %.2f\n",
           m->precoTotal, m->qtdTotal, m->tipo == UNIDADE ? "un" : "g",
           m->qtdUsada,   m->tipo == UNIDADE ? "un" : "g", /* A sintaxe do Operador Ternário não existe nos slides (teria que usar if/else).
                                                              A Mágica do Operador Ternário (? :): Repare no trecho m->tipo == UNIDADE
                                                              ? "un" : "g". O operador == verifica a igualdade. O que vem depois é um if/else
                                                              compactado: "Se o tipo for igual a UNIDADE, imprima o texto 'un'. Se não for, imprima 'g'."*/
           custoMaterial(m));
}

void listarMateriais(const Projeto *p) {
    int i;
    printf("\n--- MATERIAIS ---\n");
    if (p->numMat == 0) {
        printf("Nenhum material cadastrado.\n");
        return;
    }
    for (i = 0; i < p->numMat; i++)
        mostrarMaterial(&p->materiais[i], i + 1);
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
    /* Remover e "puxar" os de trás para a frente NÃO desfaz a ordem alfabética,
       então materiaisOrdenados continua com o valor que tinha. */
    printf("Material removido.\n");
}

void informarTrabalho(Projeto *p) {
    printf("\n--- TRABALHO E LUCRO ---\n");
    p->horas = lerNumero("Horas trabalhadas (ex: 6,5): ", 0);
    p->valorHora = lerNumero("Quanto você cobra por hora (R$): ", 0);
    p->lucroPct = lerNumero("Porcentagem de lucro desejada (%): ", 0); /* o texto vai por "%s", então um único % aparece normalmente */

    printf("Dados salvos!\n");
}

void cadastrarProjeto(Projeto *p) {
    printf("\n--- DADOS DO PROJETO ---\n");
    lerTextoObrigatorio("Nome do projeto (ex: Ursinho Narigudo): ", p->nome, TAM_NOME); /* não aceita vazio */
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
    lerTextoObrigatorio("Nome da parte (ex: Cabeça, Braço, Corpinho): ", pt->nome, TAM_NOME); /* não aceita vazio */
    pt->numPassos = 0;

    printf("Como deseja escrever esta parte da receita?\n");
    printf(" 1 - Passos enumerados (carreira por carreira)\n");
    printf(" 2 - Bloco de texto corrido (todo o tutorial de uma vez)\n");
    int formato = lerOpcao("Escolha: ", 1, 2);

    if (formato == 2) {
        /* ---------------------------------------------------------------
         * RECEITA EM BLOCO DE TEXTO
         *   - 'passos' é uma matriz: cada LINHA é uma string de TAM_PASSO caracteres.
         *     pt->passos[0] é a PRIMEIRA linha, ou seja, o endereço de um vetor de char.
         *   - O texto inteiro é gravado nessa linha 0 (o fgets dentro de lerTextoObrigatorio
         *     escreve direto nesse endereço) e numPassos vira 1: para o resto do programa
         *     (relatório, remoção...) a parte é "uma receita com 1 passo", que é o bloco.
         *     Essa contagem é só interna: o usuário não vê esse número na mensagem final.
         *   - O laço de carreiras é IGNORADO: o programa pula direto para o fim da função.
         *   - Limite: o texto precisa caber em TAM_PASSO (200) caracteres, incluindo o
         *     '\0'. O fgets corta o que passar disso.
         * --------------------------------------------------------------- */
        printf("Digite o tutorial desta parte em uma única linha (máx. %d caracteres):\n", TAM_PASSO - 1);
        lerTextoObrigatorio("  Texto: ", pt->passos[0], TAM_PASSO);
        pt->numPassos = 1;
    } else {
        printf("Digite os passos (carreira por carreira). Deixe vazio e ENTER para terminar.\n");
        printf("Ex: \"1ª carr.: 8 pb. no anel mágico\"  ou  \"Coloque os olhos entre as carr. 10 e 11\"\n");

        while (pt->numPassos < MAX_PASSOS) {
            char prompt[32];
            sprintf(prompt, "  Passo %d: ", pt->numPassos + 1); // sprintf formata a string e grava no vetor (Não consta nos slides)
            lerTexto(prompt, pt->passos[pt->numPassos], TAM_PASSO);
            if (pt->passos[pt->numPassos][0] == '\0') break;
            pt->numPassos++;
        }
    }

    p->numPartes++;
    printf("Parte \"%s\" adicionada com sucesso!\n", pt->nome);   /* mesma mensagem nos dois formatos */
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


/* ==========================================================
 *  SALVAR E CARREGAR INFORMAÇÕES (arquivo binário dados.bin)
 *  Para o usuário aparece só "Salvar" e "Carregar informações":
 *  o nome do arquivo e o formato ficam escondidos dentro do código.
 * ========================================================== */

/* ---------------------------------------------------------------
 * FORMATO DO ARQUIVO dados.bin:
 *      [ int: quantidade de projetos ][ projeto 0 ][ projeto 1 ] ... [ projeto n-1 ]
 * Gravamos só os 'n' projetos em uso (não os 50 espaços), e o contador vai na frente
 * para que o carregamento saiba quantos projetos ler.
 * IMPORTANTE: arquivos dados.bin da versão anterior (um único projeto, sem contador)
 * NÃO são compatíveis. O programa detecta e avisa; basta apagar o dados.bin antigo.
 * --------------------------------------------------------------- */

/* ---------------------------------------------------------------
 * salvarDados: grava os 'n' primeiros projetos do vetor em "dados.bin".
 *
 * LÓGICA DOS PONTEIROS (passagem por referência):
 *   - "const Projeto *v" recebe o ENDEREÇO do primeiro elemento do vetor 'projetos'
 *     (quem chamou escreveu salvarDados(projetos, numProjetos): o nome de um vetor,
 *     sem colchetes, já é o endereço do elemento 0, por isso sem '&'). Nenhuma cópia
 *     dos ~3,3 MB é feita: a função enxerga o vetor original pelo endereço.
 *   - 'const' promete que a função só LÊ os projetos (gravar em disco não os altera).
 *   - 'n' vem por VALOR (uma cópia do número): a função só precisa ler quantos são.
 *   - "FILE *f" é um ponteiro para uma estrutura interna da biblioteca que controla
 *     o arquivo aberto (posição atual, buffer, etc.).
 *   - fwrite(&n, sizeof(int), 1, f): grava o contador. Aqui precisamos do '&', porque
 *     'n' é uma variável simples e o fwrite pede o ENDEREÇO de onde os bytes começam.
 *   - fwrite(v, sizeof(Projeto), n, f): grava 'n' projetos de uma vez.
 *        1º arg: endereço onde os bytes começam (v, que já é endereço);
 *        2º arg: tamanho em bytes de cada item (sizeof(Projeto));
 *        3º arg: quantos itens gravar (n projetos, que estão lado a lado na memória);
 *        4º arg: o arquivo de destino (f).
 *     Retorna quantos itens conseguiu gravar: se != n, deu erro.
 *   - "wb" = write binary: cria o arquivo (ou apaga o conteúdo anterior).
 * --------------------------------------------------------------- */
int salvarDados(const Projeto *v, int n) {
    FILE *f = fopen(ARQ_BINARIO, "wb");
    if (f == NULL) {
        printf("Não foi possível salvar as informações.\n");
        return 0;
    }

    size_t okContador = fwrite(&n, sizeof(int), 1, f);      /* 1º: quantos projetos existem */
    size_t gravados   = fwrite(v, sizeof(Projeto), n, f);   /* 2º: os n projetos, de uma vez */
    fclose(f);

    if (okContador != 1 || gravados != (size_t) n) {
        printf("Não foi possível salvar as informações.\n");
        return 0;
    }
    printf("Informações salvas com sucesso!\n");
    return 1;
}

/* ---------------------------------------------------------------
 * carregarDados: lê "dados.bin" e preenche o vetor de projetos.
 *
 * LÓGICA DOS PONTEIROS (passagem por referência):
 *   - "Projeto *v" (SEM const) é o endereço do vetor 'projetos': a função precisa
 *     ESCREVER nele. Tudo que for feito em v[i] altera o vetor global de verdade.
 *   - "int *n" é o ENDEREÇO da variável 'numProjetos'. Se passássemos só o valor
 *     (int n), a função mudaria uma cópia e o main continuaria achando que existem 0
 *     projetos. Com o endereço, "*n = cont;" grava o novo valor DENTRO de numProjetos.
 *     (O '*' na frente de n significa "o conteúdo apontado por n".)
 *   - fread(&cont, sizeof(int), 1, f): lê o contador para a variável 'cont'
 *     (por isso o '&': o 1º argumento é o endereço onde os bytes lidos serão COLOCADOS).
 *   - fread(tmp, sizeof(Projeto), cont, f): lê 'cont' projetos para o vetor temporário
 *     'tmp'. Só depois de validar tudo copiamos para v. Assim um arquivo corrompido
 *     nunca estraga os dados que já estão na memória.
 *   - 'tmp' é 'static': fica na área de dados fixa, e não na pilha, pois tem ~3,3 MB.
 *   - "rb" = read binary. Se o arquivo não existe (primeira execução), fopen devolve
 *     NULL e a função apenas retorna 0, sem erro.
 *   - 'silencioso' = 1 evita mensagens quando for a carga automática do início.
 * --------------------------------------------------------------- */
int carregarDados(Projeto *v, int *n, int silencioso) {
    static Projeto tmp[MAX_PROJETOS];   /* área temporária para receber os bytes do arquivo */
    FILE *f = fopen(ARQ_BINARIO, "rb");
    int cont, i, j;

    if (f == NULL) {
        if (!silencioso) printf("Ainda não há informações salvas para carregar.\n");
        return 0;
    }

    /* 1º: o contador. Um valor fora de 0..MAX_PROJETOS indica arquivo de outra versão ou corrompido */
    if (fread(&cont, sizeof(int), 1, f) != 1 || cont < 0 || cont > MAX_PROJETOS) {
        fclose(f);
        printf("Não foi possível carregar as informações salvas. Nada foi alterado.\n");
        return 0;
    }

    /* 2º: os projetos. Se vierem menos bytes que o esperado, o arquivo está incompleto */
    if (fread(tmp, sizeof(Projeto), cont, f) != (size_t) cont) {
        fclose(f);
        printf("Não foi possível carregar as informações salvas. Nada foi alterado.\n");
        return 0;
    }
    fclose(f);

    /* Validação básica: contadores fora do limite indicariam arquivo corrompido */
    for (i = 0; i < cont; i++) {
        if (tmp[i].numMat < 0 || tmp[i].numMat > MAX_MAT ||
            tmp[i].numPartes < 0 || tmp[i].numPartes > MAX_PARTES) {
            printf("Não foi possível carregar as informações salvas. Nada foi alterado.\n");
            return 0;
        }
        for (j = 0; j < tmp[i].numPartes; j++) {
            if (tmp[i].partes[j].numPassos < 0 || tmp[i].partes[j].numPassos > MAX_PASSOS) {
                printf("Não foi possível carregar as informações salvas. Nada foi alterado.\n");
                return 0;
            }
        }
    }

    for (i = 0; i < cont; i++)
        v[i] = tmp[i];                 /* atribuição de struct: copia todos os campos (inclusive os vetores de char) */
    *n = cont;                         /* grava DENTRO de numProjetos, pelo endereço recebido */
    materiaisOrdenados = 0;            /* não sabemos se o arquivo foi salvo em ordem: ordenar de novo quando precisar */

    if (!silencioso) printf("Informações carregadas com sucesso!\n");
    return 1;
}


/* ==========================================================
 *  ORDENAÇÃO E PESQUISA DE MATERIAIS
 * ========================================================== */

/* Compara dois textos ignorando maiúsculas/minúsculas ("fio" == "FIO").
   Devolve <0 se a vem antes de b, 0 se são iguais, >0 se a vem depois. (tolower não consta nos slides)
   Com o locale de caracteres em "C" (veja o main), tolower só altera letras ASCII (A-Z),
   então os bytes dos acentos em UTF-8 passam intactos. */
int comparaTextos(const char *a, const char *b) {
    while (*a != '\0' && *b != '\0') {
        int ca = tolower((unsigned char) *a);   /* o cast evita problema com caracteres acentuados (negativos em char) */
        int cb = tolower((unsigned char) *b);
        if (ca != cb) return ca - cb;
        a++;                                    /* avança o ponteiro para a próxima letra */
        b++;
    }
    return tolower((unsigned char) *a) - tolower((unsigned char) *b);
}

/* ---------------------------------------------------------------
 * comp: função de comparação usada TANTO pelo qsort() QUANTO pelo bsearch().
 *
 * LÓGICA DOS PONTEIROS:
 *   - qsort e bsearch são genéricas: servem para qualquer tipo de vetor.
 *     Por isso não sabem que estão lidando com Material e entregam
 *     os elementos como "const void *" (ponteiro genérico, sem tipo).
 *   - Um void* não pode ser acessado com "->". Então fazemos o CAST
 *     (const Material *) para dizer: "este endereço guarda um Material".
 *   - Os parâmetros a e b NÃO são os materiais, e sim os ENDEREÇOS deles
 *     dentro do vetor. O qsort chama comp(&vetor[i], &vetor[j]) várias vezes.
 *   - O qsort recebe o NOME desta função (um ponteiro para função) e a
 *     chama sozinho quantas vezes precisar. Não colocamos () ao passá-la.
 *   - O retorno segue a regra: negativo (a antes de b), 0 (iguais), positivo (a depois de b).
 *   - O critério de ordem (nome, ignorando caixa) é o MESMO usado na busca binária;
 *     se fossem diferentes, o bsearch não funcionaria.
 * --------------------------------------------------------------- */
int comp(const void *a, const void *b) {
    const Material *ma = (const Material *) a;  /* converte void* -> Material* */
    const Material *mb = (const Material *) b;
    return comparaTextos(ma->nome, mb->nome);
}

/* ---------------------------------------------------------------
 * ordenarMateriais: coloca o vetor de materiais em ordem alfabética de nome.
 *
 * LÓGICA DOS PONTEIROS (passagem por referência):
 *   - "Projeto *p" (sem const) porque a função REORGANIZA o vetor do original.
 *   - p->materiais é o nome de um vetor, que em expressões vale o endereço do
 *     1º elemento (&p->materiais[0]). Esse endereço é o 1º argumento do qsort,
 *     que vai trocar os elementos de lugar DIRETO na memória do projeto.
 *   - Os demais argumentos: quantos elementos (p->numMat, só os preenchidos!),
 *     o tamanho de cada um (sizeof(Material)) e a função de comparação (comp).
 *     O qsort precisa do sizeof porque, sendo genérico, só consegue "andar"
 *     de elemento em elemento somando esse número de bytes ao endereço.
 * --------------------------------------------------------------- */
void ordenarMateriais(Projeto *p) {
    if (p->numMat > 1)
        qsort(p->materiais, p->numMat, sizeof(Material), comp);
    materiaisOrdenados = 1;
}

/* ---------------------------------------------------------------
 * pesquisarBinaria: busca um material pelo NOME EXATO com bsearch().
 *
 * Complexidade: O(log n). A cada comparação, metade dos elementos restantes
 * é descartada (30 materiais => no máximo 5 comparações; busca sequencial
 * poderia precisar de 30). Pré-requisito: vetor ORDENADO com o mesmo critério de comp.
 *
 * LÓGICA DOS PONTEIROS:
 *   - 'chave' é um Material auxiliar onde só o campo nome importa. O bsearch
 *     recebe o ENDEREÇO dele (&chave) e o passa como 1º argumento a comp.
 *   - bsearch devolve um PONTEIRO para o elemento encontrado DENTRO do vetor
 *     original (não uma cópia) ou NULL se não achar. Por isso 'achado' é Material*.
 *   - O índice é obtido por ARITMÉTICA DE PONTEIROS: "achado - p->materiais"
 *     subtrai dois endereços do mesmo tipo e o resultado já vem em número de
 *     ELEMENTOS (não em bytes), ou seja, a posição no vetor.
 *   - Se houver nomes repetidos, o bsearch devolve um deles (qualquer um).
 * --------------------------------------------------------------- */
#if 0   /* Busca binária DESATIVADA (mantida aqui só para consulta). Para reativar, apague este #if 0 e o #endif.
           Depois, inclua uma opção no menuPesquisa que chame pesquisarBinaria(p). */
void pesquisarBinaria(Projeto *p) {
    Material chave;
    Material *achado;

    if (p->numMat == 0) {
        printf("Nenhum material cadastrado.\n");
        return;
    }

    if (!materiaisOrdenados) {          /* só ordena se for necessário (cadastro novo ou dados recém-carregados) */
        ordenarMateriais(p);
        printf("(Materiais ordenados por nome para permitir a busca binária.)\n");
    }

    lerTexto("Nome exato do material (sem diferenciar maiúsculas): ", chave.nome, TAM_NOME);

    achado = (Material *) bsearch(&chave, p->materiais, p->numMat, sizeof(Material), comp);
    /* bsearch devolve void*: o cast (Material *) diz que o endereço aponta para um Material */

    if (achado == NULL) {
        printf("Material \"%s\" não encontrado.\n", chave.nome);
    } else {
        int posicao = (int) (achado - p->materiais) + 1; /* subtração de ponteiros = nº de elementos entre eles */
        printf("\nMaterial encontrado (busca binária):\n");
        mostrarMaterial(achado, posicao);
    }
}
#endif

/* Devolve 1 se 'texto' contém 'trecho' (sem diferenciar maiúsculas/minúsculas), senão 0. */
int contemTexto(const char *texto, const char *trecho) {
    char t1[TAM_NOME], t2[TAM_NOME];
    int i;

    for (i = 0; texto[i] != '\0' && i < TAM_NOME - 1; i++) t1[i] = (char) tolower((unsigned char) texto[i]);
    t1[i] = '\0';
    for (i = 0; trecho[i] != '\0' && i < TAM_NOME - 1; i++) t2[i] = (char) tolower((unsigned char) trecho[i]);
    t2[i] = '\0';

    return strstr(t1, t2) != NULL;      /* strstr procura um texto dentro de outro (não consta nos slides) */
}

/* ---------------------------------------------------------------
 * pesquisarSequencial: BUSCA SEQUENCIAL, complexidade O(n).
 * Percorre o vetor do início ao fim comparando elemento por elemento.
 * Serve para dados NÃO ordenados e para campos que não são a chave de
 * ordenação (marca, cor) ou para achar por TRECHO do texto (a binária só acha texto exato).
 *
 * LÓGICA DOS PONTEIROS:
 *   - "const Projeto *p": só lê o projeto, passado por referência (endereço).
 *   - 'm' aponta para cada elemento do vetor a cada volta do laço (&p->materiais[i]);
 *     'alvo' aponta para o campo de texto escolhido dentro desse elemento (não copia o texto).
 * --------------------------------------------------------------- */
void pesquisarSequencial(const Projeto *p) {
    char termo[TAM_NOME];
    const char *alvo;
    int i, achou = 0;

    if (p->numMat == 0) {
        printf("Nenhum material cadastrado.\n");
        return;
    }

    printf("Pesquisar em qual campo?\n 1 - Nome\n 2 - Marca\n 3 - Cor\n");
    int campo = lerOpcao("Escolha: ", 1, 3);
    lerTexto("Trecho a procurar: ", termo, TAM_NOME);

    printf("\nResultados (busca sequencial):\n");
    for (i = 0; i < p->numMat; i++) {
        const Material *m = &p->materiais[i];
        if (campo == 1)      alvo = m->nome;
        else if (campo == 2) alvo = m->marca;
        else                 alvo = m->cor;

        if (contemTexto(alvo, termo)) {
            mostrarMaterial(m, i + 1);
            achou++;
        }
    }
    if (achou == 0) printf("Nenhum material encontrado.\n");
    else            printf("%d material(is) encontrado(s).\n", achou);
}

void menuPesquisa(Projeto *p) {
    int op;
    do {
        printf("\n=== PESQUISAR MATERIAL ===\n");
        printf(" 1 - Busca por trecho (nome, marca ou cor)\n");
        printf(" 2 - Ordenar por nome e listar (qsort)\n");
        printf(" 0 - Voltar\n");
        op = lerOpcao("Escolha: ", 0, 2);
        if (op == 1) pesquisarSequencial(p);
        else if (op == 2) {
            ordenarMateriais(p);
            listarMateriais(p);
        }
    } while (op != 0);
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

/* ---------------------------------------------------------------
 * selecionarProjeto: lista os projetos do vetor, deixa o usuário ESCOLHER qual
 * projeto passa a ser o atual ou CRIAR um novo. Devolve o índice do projeto atual.
 *
 * COMO O NOVO PROJETO É ALOCADO NO VETOR:
 *   - O vetor 'projetos' já existe inteiro na memória (alocação estática, veja o
 *     comentário no topo). "Criar" um projeto, portanto, é apenas USAR o próximo
 *     espaço livre, o índice 'numProjetos':
 *        1) projetos[numProjetos] = projetoVazio;   -> atribuição de struct: copia todos
 *           os campos do projeto zerado para o espaço, limpando qualquer lixo anterior;
 *        2) cadastrarProjeto(&projetos[numProjetos]) -> passa o ENDEREÇO do espaço livre;
 *           a função preenche o projeto lá dentro (passagem por referência);
 *        3) numProjetos++ -> só agora o projeto "passa a existir" (a mesma ideia do
 *           numMat++ em adicionarMaterial).
 *   - O limite é checado antes: com MAX_PROJETOS projetos, a opção "novo" nem aparece.
 *   - Os índices do vetor começam em 0, mas a lista mostrada ao usuário começa em 1;
 *     por isso "op - 1" converte a escolha na posição do vetor.
 *
 * NUMERAÇÃO NA LISTA: 1..numProjetos são os projetos, numProjetos + 1 é "novo projeto"
 * e 0 é "voltar ao menu principal" (agora aparece na lista e avisa que voltou).
 * --------------------------------------------------------------- */
int selecionarProjeto(int atual) {
    int i, op, max;

    printf("\n=== PROJETOS ===\n");
    if (numProjetos == 0) printf("Nenhum projeto cadastrado ainda.\n");
    for (i = 0; i < numProjetos; i++)
        printf("%2d) %s (%d material(is), %d parte(s))%s\n", i + 1, projetos[i].nome,
               projetos[i].numMat, projetos[i].numPartes, i == atual ? "   <- atual" : "");

    if (numProjetos < MAX_PROJETOS) printf("%2d) << CADASTRAR NOVO PROJETO >>\n", numProjetos + 1);
    else                            printf("Limite de %d projetos atingido.\n", MAX_PROJETOS);
    printf(" 0) Voltar ao menu principal\n");

    max = numProjetos + (numProjetos < MAX_PROJETOS ? 1 : 0);
    op = lerOpcao("Escolha: ", 0, max);
    if (op == 0) {                                   /* cancelou */
        printf("Voltando ao menu principal.\n");
        return atual;                                /* continua no mesmo projeto */
    }

    if (op == numProjetos + 1) {                     /* escolheu "novo projeto" */
        projetos[numProjetos] = projetoVazio;        /* 1) limpa o espaço livre */
        cadastrarProjeto(&projetos[numProjetos]);    /* 2) preenche pelo endereço */
        numProjetos++;                               /* 3) passa a existir */
        materiaisOrdenados = 0;
        return numProjetos - 1;                      /* o novo é o último da lista */
    }

    if (op - 1 != atual) materiaisOrdenados = 0;     /* trocou de projeto: a ordem do outro não vale */
    return op - 1;
}

/* Devolve 1 se há um projeto escolhido; senão avisa e devolve 0. */
int exigeProjeto(int atual) {
    if (atual < 0) {
        printf("\nNenhum projeto selecionado. Use a opção 1 para escolher ou cadastrar um projeto.\n");
        return 0;
    }
    return 1;
}

/* ---------------------------------------------------------------
 * perguntarSalvar: chamada toda vez que o usuário escolhe a opção 1 do menu
 * (trocar ou cadastrar projeto). Pergunta se quer salvar antes de continuar.
 *
 * LÓGICA:
 *   - Se 'atual' é -1 (nenhum projeto em uso), não há o que salvar: a função nem pergunta.
 *   - lerOpcao garante que só entram 1 (Sim) ou 2 (Não); qualquer outra coisa pergunta de novo.
 *   - Se for "Sim", chama salvarDados(projetos, numProjetos). O arquivo guarda o vetor
 *     INTEIRO de projetos (não um projeto isolado), então salvar o projeto atual grava junto
 *     todos os projetos que existem na memória naquele momento, já com as alterações feitas
 *     nele. Não é preciso copiar nada: os projetos já estão no vetor global.
 *   - 'atual' vem por VALOR (cópia do número): a função só precisa ler o índice para mostrar
 *     o nome do projeto, e não altera a variável 'atual' do main.
 *   - 'projetos' é passado sem '&' porque o nome de um vetor já é o endereço do elemento 0.
 * --------------------------------------------------------------- */
void perguntarSalvar(int atual) {
    if (atual < 0) return;                           /* nenhum projeto em uso: nada a salvar */

    printf("\nDeseja salvar o projeto atual (%s) antes de continuar?\n", projetos[atual].nome);
    printf(" 1 - Sim\n 2 - Não\n");
    if (lerOpcao("Escolha: ", 1, 2) == 1)
        salvarDados(projetos, numProjetos);          /* mostra "Informações salvas com sucesso!" */
    else
        printf("Ok, continuando sem salvar.\n");
}

int main(void) {
    int op;
    int atual = -1;   /* índice (posição no vetor 'projetos') do projeto em uso; -1 = nenhum */

    /* ACENTOS E CEDILHA NO TERMINAL
       O código-fonte está em UTF-8, mas o console do Windows usa por padrão outra tabela
       de caracteres (437/850). Colocamos o console em UTF-8 (código 65001), tanto para
       o que o programa ESCREVE quanto para o que o usuário DIGITA. */
#ifdef _WIN32
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif
    setlocale(LC_ALL, "");       /* idioma do sistema: mantém a vírgula decimal do Brasil (usada no lerNumero) */
    setlocale(LC_CTYPE, "C");    /* tolower/isspace só mexem em letras ASCII: não corrompem os bytes UTF-8 dos acentos */

    printf("========================================================================\n");
    printf("                    BEM-VINDA AO AGULHA & CONTA!\n");
    printf("\n");
    printf("        O MELHOR SISTEMA DE PRECIFICAÇÃO DE AMIGURUMIS DO MUNDO!\n");
    printf("========================================================================\n");

    /* Carga automática ao iniciar (silenciosa). Passamos DOIS endereços:
         projetos      -> o nome do vetor já é o endereço do elemento 0 (sem '&');
         &numProjetos  -> endereço da variável contadora, para a função poder alterá-la.
       carregarDados preenche as duas variáveis globais diretamente (passagem por referência). */
    if (carregarDados(projetos, &numProjetos, 1))
        atual = (numProjetos > 0) ? 0 : -1;   /* já deixa o primeiro projeto selecionado */

    do { // Laço Do/While não consta nos slides
        printf("\n=========== MENU PRINCIPAL ===========\n");
        if (atual >= 0) printf("Projeto atual: %s\n", projetos[atual].nome);
        else            printf("Projeto atual: (nenhum - use a opção 1)\n");
        printf(" 1 - Escolher ou cadastrar projeto\n");
        printf(" 2 - Dados do projeto atual (nome, agulha, pontos)\n");
        printf(" 3 - Materiais\n");
        printf(" 4 - Receita (partes e passos)\n");
        printf(" 5 - Horas, valor da hora e lucro\n");
        printf(" 6 - Ver precificação final\n");
        printf(" 7 - Salvar relatório em arquivo .txt\n");
        printf(" 8 - Pesquisar material\n");
        printf(" 9 - Salvar\n");
        printf("10 - Carregar Projetos Salvos \n");
        printf(" 0 - Sair\n");
        op = lerOpcao("Escolha: ", 0, 10);

        /* &projetos[atual] = endereço do projeto escolhido dentro do vetor
           (equivale a projetos + atual). As funções recebem só esse endereço, então
           alteram o projeto ORIGINAL do vetor, e não uma cópia. */
        switch (op) { // Estrutura switch/case não consta nos slides
            case 1:
                perguntarSalvar(atual);                      /* antes de trocar/cadastrar, oferece salvar o atual */
                atual = selecionarProjeto(atual);
                break;
            case 2: if (exigeProjeto(atual)) cadastrarProjeto(&projetos[atual]);   break; /* edita o projeto atual */
            case 3: if (exigeProjeto(atual)) menuMateriais(&projetos[atual]);      break;
            case 4: if (exigeProjeto(atual)) menuReceita(&projetos[atual]);        break;
            case 5: if (exigeProjeto(atual)) informarTrabalho(&projetos[atual]);   break;
            case 6: if (exigeProjeto(atual)) { imprimirRelatorio(stdout, &projetos[atual]); pausar(); } break;
            case 7: if (exigeProjeto(atual)) salvarRelatorio(&projetos[atual]);    break;
            case 8: if (exigeProjeto(atual)) menuPesquisa(&projetos[atual]);       break; /* pode ordenar o vetor de materiais original */
            case 9: salvarDados(projetos, numProjetos);      break; /* só lê o vetor (const): passa o vetor e a quantidade */
            case 10:
                /* escreve no vetor e em numProjetos, por isso passa os endereços */
                if (carregarDados(projetos, &numProjetos, 0))
                    atual = (numProjetos > 0) ? 0 : -1;      /* o índice antigo pode não existir mais */
                break;
            case 0: printf("\nAté logo!\n");                 break;
        }
    } while (op != 0);

    return 0;
}
