#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h> // Necessário para a função sleep()

#define TEMPSIMAM 1800 // 06h30 - 09h00 (1800 segundos)
#define TEMPSIMPM 12600 // 11h00 - 14h30 (12600 segundos)
#define TEMPSIMJANTAR 9000 // 17h00 - 19h30 (9000 segundos)

#define TEMPUSUMIN 3 // Tempo mínimo de atendimento em segundos
#define TEMPUSUMAX 6 // Tempo máximo de atendimento em segundos

#define NFILAS 5 // Número de filas

#define TEMPOESPECIALVEGETARIANO 2 // Tempo adicional para vegetarianos

#define QTDBANMIN 1 // Mínimo de bancadas
#define QTDBANMAX 5 // Máximo de bancadas
#define BANSERMIN 3 // Mínimo de serventes por bancada
#define BANSERMAX 6 // Máximo de serventes por bancada

#define TEMPSERMAX 3600 // Máximo de trabalho contínuo em segundos (1 hora)
#define TEMPSERINT 1800 // Intervalo mínimo em segundos (30 minutos)

#define NING 6 // Número de ingredientes
#define CAPING1 5000 // Capacidade da vasilha em gramas

#define NPROBING1 70 // Probabilidade de aceitação do ingrediente 1 (%)
#define QTDMINING1 100 // Quantidade mínima servida do ingrediente 1 em gramas
#define QTDMAXING1 300 // Quantidade máxima servida do ingrediente 1 em gramas

// Definindo a resolução do simulador
#define RESOLUCAOSEGUNDOS 1 // Resolução em segundos

typedef struct {
    int id;
    bool vegetariano;
    int filaEscolhida; // Fila escolhida pelo usuário
} Usuario;

typedef struct {
    int id;
    int usuariosAtendidos;
    float tempoMedioAtendimento;
    float tempoTotalAtendimento;
    int tempoTrabalho; // Tempo total trabalhado pelo servente
    bool ativo;       // Indica se o servente está ativo ou não
} Servente;

typedef struct {
    int id;
    int serventesAtendidos;
    float tempoMedioAtendimento;
    float tempoTotalAtendimento;
} Bancada;

Bancada bancadas[QTDBANMAX];
Servente serventes[BANSERMAX]; // Total de serventes é definido aqui
int totalUsuariosAtendidos = 0;
float totalTempoAtendimento = 0;
float totalTempoEspera = 0;
int totalConsumido[NING] = {0}; // Total consumido por cada ingrediente

void iniciarSimulador(int numAtendimentos);
void fecharRU();
void atenderUsuario(int tempoAtual, int usuarioId);
int gerarTempoAtendimento();
void gerarRelatorios();
int chegadaUsuarios(int tempoAtual);
bool usuarioVegetariano();
int calcularTempoDeEspera(int fila);
void atualizarRelatorios(int ingrediente, int tempoAtendimento, int serventeId, int bancadaId);
void trocarServente(int *serventeId);

int main() {
    srand(time(NULL)); // Inicializa a semente para números aleatórios
    
    int numAtendimentos;
    printf("Digite o numero total de atendimentos a serem feitos: ");
    scanf("%d", &numAtendimentos);

    iniciarSimulador(numAtendimentos);
    return 0;
}

void iniciarSimulador(int numAtendimentos) {
    int tempoAtual = 0;

    for (int i = 0; i < QTDBANMIN; i++) { 
        bancadas[i].id = i + 1; 
        bancadas[i].serventesAtendidos = 0; 
        bancadas[i].tempoMedioAtendimento = 0; 
        bancadas[i].tempoTotalAtendimento = 0; 
        for (int j = 0; j < BANSERMAX; j++) {
            serventes[j].usuariosAtendidos = -1; 
            serventes[j].tempoTrabalho = -1;     // Marca como não disponíveis inicialmente
            serventes[j].ativo = false;          // Inicialmente não ativos
        }
    }

    for (int j = 0; j < BANSERMIN; j++) { // Ativa apenas os primeiros três serventes
        serventes[j].usuariosAtendidos = 0; 
        serventes[j].tempoMedioAtendimento = 0; 
        serventes[j].tempoTotalAtendimento = 0; 
        serventes[j].tempoTrabalho = rand() % TEMPSERMAX; // Tempo inicial trabalhado aleatório
        serventes[j].ativo = true;   // Marca como ativo
    }

    for (int atendimentosFeitos = 0; atendimentosFeitos < numAtendimentos; atendimentosFeitos++) {
        if (tempoAtual < (TEMPSIMAM + TEMPSIMPM + TEMPSIMJANTAR)) {
            if (tempoAtual % RESOLUCAOSEGUNDOS == 0) {
                int usuarioId = atendimentosFeitos + 1; // ID do usuário
                atenderUsuario(tempoAtual, usuarioId);
            }
            tempoAtual += RESOLUCAOSEGUNDOS; 
        } else {
            break; // Se o horário do RU já passou, encerra o loop
            
        }
        
        if (tempoAtual % TEMPSERMAX == 0) { 
            for (int i = BANSERMIN - 1; i < BANSERMAX; i++) {
                if (serventes[i].ativo) {
                    printf("Horario: - Servente %d foi para intervalo.\n", i + 1);
                    serventes[i].ativo = false;   // Marca como não ativo
                }
            }
            
            for (int i = BANSERMIN - 1; i < BANSERMAX; i++) {
                if (!serventes[i].ativo) {
                    printf("Horario: - Servente %d voltou do intervalo.\n", i + 1);
                    serventes[i].ativo = true;   // Marca como ativo novamente
                }
            }
            
            printf("Serventes ativos: ");
            for (int k = BANSERMIN - 1; k < BANSERMAX; k++) {
                if (serventes[k].ativo) {
                    printf("%d ", k + 1); // Mostra os números dos serventes ativos
                }
            }
            printf("\n");
        }
    }

    fecharRU();
}

void fecharRU() {
    gerarRelatorios();
}

void atenderUsuario(int tempoAtual, int usuarioId) {
    Usuario usuario;
    usuario.id = usuarioId;
    
    usuario.vegetariano = usuarioVegetariano();
    
    int filaEscolhida = rand() % NFILAS; 
    usuario.filaEscolhida = filaEscolhida;

    int tempoEspera = calcularTempoDeEspera(filaEscolhida);
    
    totalTempoEspera += tempoEspera;

    int tempoAtendimento = gerarTempoAtendimento();
    
    if (usuario.vegetariano) {
        tempoAtendimento += TEMPOESPECIALVEGETARIANO;
    }

    totalTempoAtendimento += tempoAtendimento;

    static int serventeId = -1; // Servente atual (-1 indica que ainda não foi escolhido)
    
    if (serventeId == -1) { 
        serventeId = rand() % BANSERMIN; // Escolhe o primeiro servente aleatoriamente entre os três ativos
        printf("Horário: - Servente %d iniciado.\n", serventeId + 1);
    } else {
        trocarServente(&serventeId); // Troca para o próximo servente na sequência
    }

    int bancadaId = rand() % QTDBANMIN;   // Escolhe uma bancada aleatoriamente
    
    printf("Ordem do atendimento: %d | Fila: %d | Vegetarian: %s | Atendido por Servente: %d\n", 
           usuario.id, usuario.filaEscolhida + 1, 
           usuario.vegetariano ? "Sim" : "Não", 
           serventeId + 1); 

    atualizarRelatorios(rand() % NING, tempoAtendimento, serventeId, bancadaId); 

    totalUsuariosAtendidos++;

}

int gerarTempoAtendimento() {
    return rand() % (TEMPUSUMAX - TEMPUSUMIN + 1) + TEMPUSUMIN;
}

bool usuarioVegetariano() {
    return rand() % 2 == 0; 
}

int calcularTempoDeEspera(int fila) {
    return rand() % (10); 
}

void atualizarRelatorios(int ingrediente, int tempoAtendimento, int serventeId, int bancadaId) {
    totalConsumido[ingrediente] += QTDMINING1 + rand() % (QTDMAXING1 - QTDMINING1 + 1);

    serventes[serventeId].usuariosAtendidos++;
    serventes[serventeId].tempoTotalAtendimento += tempoAtendimento;

    bancadas[bancadaId].serventesAtendidos++;
    bancadas[bancadaId].tempoTotalAtendimento += tempoAtendimento;
}

void trocarServente(int *serventeId) {
   *serventeId = (*serventeId + 1) % BANSERMAX; // Alterna para o próximo servente na sequência
   printf("Horario: - Servente trocado para o Servente %d.\n", *serventeId + 1);
   printf("Serventes ativos: ");
   for (int k = BANSERMIN - 1; k < BANSERMAX; k++) {
       if (serventes[k].ativo) {
           printf("%d ", k + 1); // Mostra os números dos serventes ativos
       }
   }
   printf("\n");
}

void gerarRelatorios() {
    if (totalUsuariosAtendidos > 0) {
        printf("\n\n===========================================================>RELATORIO<===========================================================");
        printf("\n->    Total de usuarios atendidos: %d\n", totalUsuariosAtendidos);
        printf("->    Tempo medio de atendimento: %.2f segundos\n", totalTempoAtendimento / totalUsuariosAtendidos);
        printf("->    Tempo medio de espera: %.2f segundos\n", totalTempoEspera / totalUsuariosAtendidos);
        
        for (int i = 0; i < NING; i++) {
            printf("->    Total consumido do ingrediente %d: %d gramas\n", i + 1, totalConsumido[i]);
        }
        
        for (int i = 0; i < BANSERMAX; i++) { // Inclui todos os seis serventes no relatório
            if (serventes[i].usuariosAtendidos >= 0) { // Verifica se o servente foi utilizado
                printf("->    Servente %d: | Atendidos: %d | Tempo medio: %.2f segundos\n", 
                        i + 1, 
                        serventes[i].usuariosAtendidos, 
                        (serventes[i].usuariosAtendidos > 0 ? 
                        serventes[i].tempoTotalAtendimento / serventes[i].usuariosAtendidos : 0));
            }
        }

        for (int i = 0; i < QTDBANMIN; i++) {
            if (bancadas[i].serventesAtendidos > 0) {
               printf("->    Bancada %d: | Atendidos: %d | Tempo medio: %.2f segundos\n", 
                       bancadas[i].id,
                       bancadas[i].serventesAtendidos,
                       bancadas[i].tempoTotalAtendimento / bancadas[i].serventesAtendidos);
            }
        }
        
    } else {
        printf("Nenhum usuario foi atendido.\n");
    }
}