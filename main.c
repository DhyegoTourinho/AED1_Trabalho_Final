#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// Role dos define
#define TEMPSIMAM 9000              // Café da manhã
#define TEMPSIMPM 12600             // Almoço
#define TEMPJANTAR 9000             // Jantar
#define BANSERMIN 3                 // Mínimo de serventes por bancada
#define BANSERMAX 6                 // Máximo de serventes por bancada
#define QTDBANMIN 1                 // Mínimo de bancadas
#define QTDBANMAX 3                 // Máximo de bancadas
#define QTDSERMIN 3                 // Mínimo de serventes ativos
#define QTSERMAX 6                  // Máximo de serventes ativos
#define TEMPSERMAX 3600             // Máximo de trabalho sem intervalo (1 hora)
#define TEMPSERINT 1800             // Intervalo obrigatório de descanso (30 minutos)
#define NING 6                      // Ingredientes do cardápio
#define CAPING 1000                 // Capacidade de cada vasilha em gramas
#define QTDMINING 50                // Mínimo de gramas por ingrediente
#define QTDMAXING 150               // Máximo de gramas por ingrediente
#define NFILAS 3                    // Número de filas de atendimento
#define TEMPUSUMIN 3                // Tempo mínimo de atendimento
#define TEMPUSUMAX 6                // Tempo máximo de atendimento
#define TEMPO_EXTRA_VEGETARIANO 5   // Tempo adicional para usuários vegetarianos

typedef struct Usuario {
    int id;
    bool vegetariano;
    int tempoEspera;
    struct Usuario* prox;
} Usuario;

typedef struct Fila {
    Usuario* inicio;
    Usuario* fim;
    int tamanho;
} Fila;

typedef struct Servente {
    int id;
    bool atendeVegano;
    bool ocupado;
    int usuariosAtendidos;
    int tempoTrabalhado;
    int tempoDesdeDescanso;
} Servente;

typedef struct Ingrediente {
    int id;
    int quantidade; 
} Ingrediente;

// Inicia e faz Manutenção das Filas

void inicializarFila(Fila* fila) {
    fila->inicio = NULL;
    fila->fim = NULL;
    fila->tamanho = 0;
}

bool filaVazia(Fila* fila) {
    return fila->tamanho == 0;
}

// Usuário entra na fila com menos pessoas
void enfileirar(Fila* filas, int numFilas, int id, bool vegetariano) {
    int menorFila = 0;
    for (int i = 1; i < numFilas; i++) {
        if (filas[i].tamanho < filas[menorFila].tamanho) {
            menorFila = i;
        }
    }
    Usuario* novo = (Usuario*)malloc(sizeof(Usuario));
    novo->id = id;
    novo->vegetariano = vegetariano;
    novo->tempoEspera = 0;
    novo->prox = NULL;

    if (filaVazia(&filas[menorFila])) {
        filas[menorFila].inicio = novo;
    } else {
        filas[menorFila].fim->prox = novo;
    }
    filas[menorFila].fim = novo;
    filas[menorFila].tamanho++;
    printf("Usuário %d entrou na fila %d. Vegetariano: %s\n", id, menorFila + 1, vegetariano ? "Sim" : "Não");
}

Usuario* desenfileirar(Fila* fila) {
    if (filaVazia(fila)) return NULL;
    Usuario* removido = fila->inicio;
    fila->inicio = removido->prox;
    if (fila->inicio == NULL) fila->fim = NULL;
    fila->tamanho--;
    return removido;
}

// Começa de Serventes e Ingredientes

void inicializarServentes(Servente* serventes, int qtd) {
    for (int i = 0; i < qtd; i++) {
        serventes[i].id = i + 1;
        serventes[i].atendeVegano = (i == 0 || i == 3);  // Serventes 1 e 4 são vegetarianos
        serventes[i].ocupado = false;
        serventes[i].usuariosAtendidos = 0;
        serventes[i].tempoTrabalhado = 0;
        serventes[i].tempoDesdeDescanso = 0;
    }
}

void inicializarIngredientes(Ingrediente* ingredientes) {
    for (int i = 0; i < NING; i++) {
        ingredientes[i].id = i + 1;
        ingredientes[i].quantidade = CAPING;
    }
}

// Função para Buscar Servente Disponível
Servente* buscarServenteDisponivel(Servente* serventes, int qtdServentes, Usuario* usuario) {
    if (usuario->vegetariano) {
        // Busca entre os serventes vegetarianos (serventes 1 e 4)
        for (int i = 0; i < qtdServentes; i++) {
            if ((serventes[i].id == 1 || serventes[i].id == 4) && !serventes[i].ocupado) {
                return &serventes[i];
            }
        }
        return NULL;  // Serventes vegetarianos ocupados, fila será pausada
    } else {
        // Busca por qualquer servente comum para não vegetarianos
        for (int i = 0; i < qtdServentes; i++) {
            if (!serventes[i].ocupado) {
                return &serventes[i];
            }
        }
        return NULL;  // Todos os serventes comuns ocupados
    }
}

// Atendimento e Controle de Ingredientes

void atenderUsuario(Usuario* usuario, Servente* servente, Ingrediente* ingredientes) {
    int tempoAtendimento = TEMPUSUMIN + rand() % (TEMPUSUMAX - TEMPUSUMIN + 1);
    if (usuario->vegetariano) {
        tempoAtendimento += TEMPO_EXTRA_VEGETARIANO;  // Tempo extra para vegetarianos
    }

    printf("Servente %d está atendendo o usuário %d por %d segundos.\n", 
           servente->id, usuario->id, tempoAtendimento);

    for (int i = 0; i < NING; i++) {
        if (rand() % 2 && ingredientes[i].quantidade >= QTDMINING) {
            ingredientes[i].quantidade -= QTDMINING;
        } else if (ingredientes[i].quantidade < QTDMINING) {
            printf("Ingrediente %d em reposição...\n", ingredientes[i].id);
            ingredientes[i].quantidade = CAPING;
        }
    }

    servente->usuariosAtendidos++;
    servente->tempoTrabalhado += tempoAtendimento;
    servente->tempoDesdeDescanso += tempoAtendimento;
    servente->ocupado = true;
    free(usuario);
}

// descanso dos serventes e ativa rodízio
void verificarDescanso(Servente* serventes, int qtd) {
    for (int i = 0; i < qtd; i++) {
        if (serventes[i].tempoDesdeDescanso >= TEMPSERMAX) {
            printf("Servente %d entrou em descanso obrigatório.\n", serventes[i].id);
            serventes[i].tempoDesdeDescanso = 0;
            serventes[i].ocupado = true;
        } else if (serventes[i].ocupado && serventes[i].tempoDesdeDescanso >= TEMPSERINT) {
            serventes[i].ocupado = false;
            printf("Servente %d retornou ao trabalho.\n", serventes[i].id);
        }
    }
}

// Turno e Relatório

void simularTurno(Fila* filas, Servente* serventes, int qtdServentes, Ingrediente* ingredientes, bool jantar) {
    bool filaProcessada;
    do {
        filaProcessada = false;
        for (int i = 0; i < (jantar ? NFILAS - 1 : NFILAS); i++) {
            printf("\n*---------------------*\n");
            printf("Processando Fila %d:\n", i + 1);
            if (!filaVazia(&filas[i])) {
                Usuario* usuario = filas[i].inicio;
                Servente* servente = buscarServenteDisponivel(serventes, qtdServentes, usuario);

                if (servente) {
                    desenfileirar(&filas[i]);
                    atenderUsuario(usuario, servente, ingredientes);
                    filaProcessada = true;
                } else {
                    printf("Nenhum servente disponível para o usuário %d. Fila pausada.\n", usuario->id);
                    break;
                }
                verificarDescanso(serventes, qtdServentes);
            }
        }
    } while (filaProcessada);
}

void gerarRelatorio(Servente* serventes, int qtdServentes, Ingrediente* ingredientes) {
    printf("\n--- Relatório Final ---\n");
    for (int i = 0; i < qtdServentes; i++) {
        printf("Servente %d atendeu %d usuários e trabalhou %d segundos.\n", 
               serventes[i].id, serventes[i].usuariosAtendidos, serventes[i].tempoTrabalhado);
    }

    for (int i = 0; i < NING; i++) {
        printf("Ingrediente %d - Quantidade restante: %d gramas\n", 
               ingredientes[i].id, ingredientes[i].quantidade);
    }
}

int main() {
    srand(time(NULL));

    int numeroUsuarios;
    printf("Digite o número de usuários para a simulação: ");
    scanf("%d", &numeroUsuarios);

    // Começa as filas
    Fila filas[NFILAS];
    for (int i = 0; i < NFILAS; i++) {
        inicializarFila(&filas[i]);
    }

    // Começa serventes e ingredientes
    Servente serventes[QTSERMAX];
    inicializarServentes(serventes, QTDSERMIN);
    Ingrediente ingredientes[NING];
    inicializarIngredientes(ingredientes);

    // Enfileira usuários, escolhendo a fila com menos pessoas
    for (int i = 0; i < numeroUsuarios; i++) {
        enfileirar(filas, NFILAS, i + 1, rand() % 2);
    }

    // Simulação dos turnos com a regra do jantar (uma fila a menos)
    simularTurno(filas, serventes, QTDSERMIN, ingredientes, false); // Manhã e Almoço
    simularTurno(filas, serventes, QTDSERMIN, ingredientes, true);  // Jantar

    // Gera relatório final
    gerarRelatorio(serventes, QTDSERMIN, ingredientes);

    return 0;
}