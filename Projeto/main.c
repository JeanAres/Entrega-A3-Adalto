#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_CITIES 1000
#define MAX_NAME 100

// aresta (lista ligada)
typedef struct Edge {
    int to;
    int weight;
    struct Edge *next;
} Edge;

// estrutura usada pra listar vizinhos (opção 3)
typedef struct {
    int city_id;
    int distance;
} VizinhoInfo;

// estrutura pra contar conexões (opção 2)
typedef struct {
    int city_id;
    int count;
    char name[MAX_NAME];
} ConexaoCount;

char city_names[MAX_CITIES][MAX_NAME];
int city_count = 0;
Edge *adj[MAX_CITIES] = {NULL};

/* --- utilitárias de string --- */

// normalizo para minúsculas e tiro espaços nas pontas
void str_to_lower_trim(char *s) {
    char *p = s;
    while (*p) {*p = tolower((unsigned char)*p); p++;}
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start)+1);
    int len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len-1])) s[--len] = '\0';
}

// retorna índice da cidade (cria se não existir)
int city_index(const char *name_in) {
    char name[MAX_NAME];
    strncpy(name, name_in, MAX_NAME-1);
    name[MAX_NAME-1] = '\0';
    str_to_lower_trim(name);

    for (int i = 0; i < city_count; ++i) {
        char tmp[MAX_NAME];
        strncpy(tmp, city_names[i], MAX_NAME-1);
        tmp[MAX_NAME-1] = '\0';
        str_to_lower_trim(tmp);
        if (strcmp(tmp, name) == 0) return i;
    }
    if (city_count >= MAX_CITIES) {
        fprintf(stderr, "ERRO: Numero maximo de cidades atingidos\n");
        exit(1);
    }
    strncpy(city_names[city_count], name_in, MAX_NAME-1);
    city_names[city_count][MAX_NAME-1] = '\0';
    city_count++;
    return city_count - 1;
}

// adiciona aresta (grafo não direcionado)
void add_edge(int a, int b, int w) {
    Edge *e1 = malloc(sizeof(Edge));
    e1->to = b; e1->weight = w; e1->next = adj[a]; adj[a] = e1;
    Edge *e2 = malloc(sizeof(Edge));
    e2->to = a; e2->weight = w; e2->next = adj[b]; adj[b] = e2;
}

/* Levenshtein - usado pra busca aproximada de nomes */
int levenshtein(const char *s, const char *t) {
    int n = strlen(s), m = strlen(t);
    if (n == 0) return m;
    if (m == 0) return n;
    int *v0 = malloc((m+1)*sizeof(int));
    int *v1 = malloc((m+1)*sizeof(int));
    for (int j=0;j<=m;++j) v0[j] = j;
    for (int i=1;i<=n;++i) {
        v1[0] = i;
        for (int j=1;j<=m;++j) {
            int cost = (s[i-1] == t[j-1]) ? 0 : 1;
            int deletion = v0[j] + 1;
            int insertion = v1[j-1] + 1;
            int substitution = v0[j-1] + cost;
            int v = deletion;
            if (insertion < v) v = insertion;
            if (substitution < v) v = substitution;
            v1[j] = v;
        }
        memcpy(v0, v1, (m+1)*sizeof(int));
    }
    int res = v0[m];
    free(v0); free(v1);
    return res;
}

/* Busca aproximada: tenta achar cidade pelo input do usuário */
int fuzzy_match_city(const char *input, char *matched_name_out) {
    char temp_input[MAX_NAME];
    strncpy(temp_input, input, MAX_NAME-1);
    temp_input[MAX_NAME-1] = '\0';
    str_to_lower_trim(temp_input);

    int best_idx = -1;
    int best_dist = INT_MAX;
    int threshold = strlen(temp_input) > 3 ? 4 : 2;

    for (int i=0; i<city_count; ++i) {
        char tmp[MAX_NAME];
        strncpy(tmp, city_names[i], MAX_NAME-1);
        tmp[MAX_NAME-1] = '\0';
        str_to_lower_trim(tmp);

        if (strstr(tmp, temp_input) != NULL) {
             best_dist = 0;
             best_idx = i;
             break;
        }

        int d = levenshtein(temp_input, tmp);
        if (d < best_dist) {
            best_dist = d;
            best_idx = i;
        }
    }

    if (best_idx >= 0 && best_dist <= threshold) {
        if (matched_name_out) {
            strncpy(matched_name_out, city_names[best_idx], MAX_NAME-1);
            matched_name_out[MAX_NAME-1] = '\0';
        }
        return best_idx;
    }
    return -1;
}

/* Dijkstra (menor distância) */
void dijkstra(int src, int dist[], int prev[]) {
    for (int i=0;i<city_count;++i) { dist[i] = INT_MAX; prev[i] = -1; }
    dist[src] = 0;
    int visited[MAX_CITIES] = {0};

    for (int it=0; it<city_count; ++it) {
        int u = -1;
        int best = INT_MAX;
        for (int i=0;i<city_count;++i) if (!visited[i] && dist[i] < best) { best = dist[i]; u = i; }
        if (u == -1) break;
        visited[u] = 1;
        for (Edge *e = adj[u]; e != NULL; e = e->next) {
            int v = e->to;
            int w = e->weight;
            if (!visited[v] && dist[u] != INT_MAX && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                prev[v] = u;
            }
        }
    }
}

int compare_vizinhos(const void *a, const void *b) {
    VizinhoInfo *va = (VizinhoInfo *)a;
    VizinhoInfo *vb = (VizinhoInfo *)b;
    return va->distance - vb->distance;
}

int compare_conexoes(const void *a, const void *b) {
    ConexaoCount *ca = (ConexaoCount *)a;
    ConexaoCount *cb = (ConexaoCount *)b;
    if (ca->count != cb->count) return ca->count - cb->count;
    return strcmp(ca->name, cb->name);
}

/* --- menus simples --- */

void menu_listar_cidades() {
    printf("\n--- Cidades Cadastradas (%d) ---\n", city_count);
    for (int i = 0; i < city_count; i++) {
        printf("%d. %s\n", i + 1, city_names[i]);
    }
    printf("---------------------------------\n");
}

void menu_contar_conexoes() {
    ConexaoCount lista[MAX_CITIES];
    for (int i = 0; i < city_count; i++) {
        int count = 0;
        for (Edge *e = adj[i]; e != NULL; e = e->next) count++;
        lista[i].city_id = i;
        lista[i].count = count;
        strcpy(lista[i].name, city_names[i]);
    }
    qsort(lista, city_count, sizeof(ConexaoCount), compare_conexoes);
    printf("\n--- Numero de Conexoes por Cidade (Ordem Crescente) ---\n");
    for (int i = 0; i < city_count; i++) {
        printf("%s: %d conexoes\n", lista[i].name, lista[i].count);
    }
    printf("-------------------------------------------------------\n");
}

/* Lê cidade do usuário com fuzzy match */
int ler_cidade_input(char *prompt) {
    char input[MAX_NAME];
    char matched_name[MAX_NAME];
    int idx = -1;
    do {
        printf("%s", prompt);
        if (fgets(input, sizeof(input), stdin) == NULL) return -1;
        input[strcspn(input, "\n")] = 0;
        idx = fuzzy_match_city(input, matched_name);
        if (idx == -1) printf("Cidade '%s' nao encontrada ou ambigua. Tente novamente.\n", input);
        else printf("-> Selecionado: %s\n", matched_name);
    } while (idx == -1);
    return idx;
}

/* Mostra conexões de uma cidade, ordenadas por distância */
void menu_conexoes_ordenadas() {
    int cidade_idx = ler_cidade_input("\nDigite o nome da cidade para ver vizinhos: ");
    if (cidade_idx == -1) return;

    VizinhoInfo vizinhos[MAX_CITIES];
    int count = 0;
    for (Edge *e = adj[cidade_idx]; e != NULL; e = e->next) {
        vizinhos[count].city_id = e->to;
        vizinhos[count].distance = e->weight;
        count++;
    }

    if (count == 0) {
        printf("Esta cidade nao possui conexoes diretas.\n");
        return;
    }

    qsort(vizinhos, count, sizeof(VizinhoInfo), compare_vizinhos);

    printf("\nConexoes de %s (por distancia):\n", city_names[cidade_idx]);
    for (int i = 0; i < count; i++) {
        printf("%d. %s (%d km)\n", i+1, city_names[vizinhos[i].city_id], vizinhos[i].distance);
    }
}

/* --- auxiliars para componentes e reconstrução de caminho --- */

// marca componente por DFS
void dfs_mark_component(int u, int comp_id, int comp[]) {
    comp[u] = comp_id;
    for (Edge *e = adj[u]; e != NULL; e = e->next) {
        if (comp[e->to] == -1) dfs_mark_component(e->to, comp_id, comp);
    }
}

// reconstrói caminho usando array prev (retorna tamanho; ordem from->to)
int reconstruct_path(int prev[], int from, int to, int caminho[]) {
    int tam = 0;
    int cur = to;
    while (cur != -1) {
        caminho[tam++] = cur;
        if (cur == from) break;
        cur = prev[cur];
    }
    if (tam == 0 || caminho[tam-1] != from) return 0;
    for (int i = 0; i < tam/2; ++i) {
        int tmp = caminho[i];
        caminho[i] = caminho[tam-1-i];
        caminho[tam-1-i] = tmp;
    }
    return tam;
}

/* Opção 4: calcula distância/trajeto; sugere rota parcial se componentes diferentes */
void menu_distancia_entre_cidades() {
    printf("\n--- Calcular Distancia e Trajeto ---\n");
    int origem = ler_cidade_input("Cidade de Origem: ");
    int destino = ler_cidade_input("Cidade de Destino: ");

    if (origem == destino) {
        printf("Distancia: 0 km (mesma cidade).\n");
        return;
    }

    int dist[MAX_CITIES], prev[MAX_CITIES];
    dijkstra(origem, dist, prev);

    if (dist[destino] != INT_MAX) {
        // caminho completo existe
        printf("\nMenor distancia entre %s e %s: %d km\n", city_names[origem], city_names[destino], dist[destino]);
        printf("Trajeto a ser percorrido: ");
        int caminho[MAX_CITIES];
        int tam_caminho = 0;
        int atual = destino;
        while (atual != -1) {
            caminho[tam_caminho++] = atual;
            atual = prev[atual];
        }
        for (int i = tam_caminho - 1; i >= 0; i--) {
            printf("%s", city_names[caminho[i]]);
            if (i > 0) printf(" -> ");
        }
        printf("\n");
        return;
    }

    // componentes diferentes: identifico componentes
    int comp[MAX_CITIES];
    for (int i = 0; i < city_count; ++i) comp[i] = -1;
    int comp_id = 0;
    for (int i = 0; i < city_count; ++i) {
        if (comp[i] == -1) {
            dfs_mark_component(i, comp_id, comp);
            comp_id++;
        }
    }

    int comp_origem = comp[origem];
    int comp_destino = comp[destino];

    if (comp_origem == comp_destino) {
        // caso raro, trato como sem caminho
        printf("Nao ha caminho registrado entre %s e %s.\n", city_names[origem], city_names[destino]);
        return;
    }

    // escolho par (u,v) entre componentes com heurística (Levenshtein nos nomes)
    int best_u = -1, best_v = -1, best_lev = INT_MAX;
    for (int u = 0; u < city_count; ++u) {
        if (comp[u] != comp_origem) continue;
        for (int v = 0; v < city_count; ++v) {
            if (comp[v] != comp_destino) continue;
            char a[MAX_NAME], b[MAX_NAME];
            strncpy(a, city_names[u], MAX_NAME-1); a[MAX_NAME-1] = '\0';
            strncpy(b, city_names[v], MAX_NAME-1); b[MAX_NAME-1] = '\0';
            str_to_lower_trim(a); str_to_lower_trim(b);
            int lev = levenshtein(a, b);
            if (lev < best_lev) { best_lev = lev; best_u = u; best_v = v; }
        }
    }

    // caminho da origem até best_u (já tenho prev do Dijkstra com origem)
    int caminho_a[MAX_CITIES], tam_a = reconstruct_path(prev, origem, best_u, caminho_a);

    // caminho de best_v até destino (rodo Dijkstra com source = best_v)
    int dist2[MAX_CITIES], prev2[MAX_CITIES];
    dijkstra(best_v, dist2, prev2);
    int caminho_b[MAX_CITIES], tam_b = reconstruct_path(prev2, best_v, destino, caminho_b);

    // exibo sugestão parcial e aviso que falta ligação entre best_u e best_v
    printf("\nNao existe caminho completo registrado entre %s e %s.\n", city_names[origem], city_names[destino]);
    if (best_u != -1 && best_v != -1 && tam_a > 0 && tam_b > 0) {
        printf("Sugestao parcial (componentes distintos):\n");
        for (int i = 0; i < tam_a; ++i) {
            printf("%s", city_names[caminho_a[i]]);
            if (i < tam_a - 1) printf(" -> ");
        }
        printf("\nFALTA LIGACAO ENTRE '%s' E '%s'\n", city_names[best_u], city_names[best_v]);
        printf("Para completar o trajeto, seria necessario ligar essas duas cidades.\n");
        for (int i = 0; i < tam_b; ++i) {
            if (i == 0) printf("%s", city_names[caminho_b[i]]);
            else printf(" -> %s", city_names[caminho_b[i]]);
        }
        printf("\n");
    } else {
        printf("Nao foi possivel sugerir uma rota parcial entre as componentes.\n");
    }
}

/* cria nova conexão (menu 5) e persiste no CSV */
void menu_nova_conexao() {
    char buffer[MAX_NAME];
    printf("\n--- Criar Nova Conexao e Salvar ---\n");

    printf("Nome da primeira cidade: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    int id1 = city_index(buffer);

    printf("Nome da segunda cidade: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;
    int id2 = city_index(buffer);

    if (id1 == id2) {
        printf("Erro: As cidades devem ser diferentes.\n");
        return;
    }

    int dist;
    printf("Distancia (km): ");
    scanf("%d", &dist);
    while (getchar() != '\n');

    add_edge(id1, id2, dist);

    FILE *f = fopen("cidades_rs_grafo.csv", "a");
    if (f == NULL) {
        printf("ERRO: Conexao criada na memoria, mas falha ao abrir arquivo para salvar!\n");
    } else {
        fprintf(f, "%s,%s,%d\n", city_names[id1], city_names[id2], dist);
        fclose(f);
        printf("Sucesso! Dados salvos em 'cidades_rs_grafo.csv'.\n");
    }

    printf("Conexao criada: %s <--> %s (%d km)\n", city_names[id1], city_names[id2], dist);
}

/* main: carrega CSV e mostra menu */
int main() {
    FILE *arquivo;
    char origem[50], destino[50];
    int distancia;

    arquivo = fopen("cidades_rs_grafo.csv", "r");

    if (arquivo == NULL) {
        printf("ERRO CRITICO: Arquivo 'cidades_rs_grafo.csv' nao encontrado.\n");
        return 1;
    }

    printf("Carregando grafo...\n");
    char cabecalho[150];
    fgets(cabecalho, sizeof(cabecalho), arquivo);

    while (fscanf(arquivo, " %[^,],%[^,],%d", origem, destino, &distancia) == 3) {
        int id_origem = city_index(origem);
        int id_destino = city_index(destino);
        add_edge(id_origem, id_destino, distancia);
    }
    fclose(arquivo);
    printf("Dados carregados! Total de cidades: %d\n", city_count);

    int opcao = 0;

    do {
        printf("\n================ MENU ================\n");
        printf("1) Mostrar todas as cidades cadastradas\n");
        printf("2) Numero de conexoes por cidade (Ordenado)\n");
        printf("3) Ver conexoes de uma cidade\n");
        printf("4) Calcular distancia e trajeto entre cidades\n");
        printf("5) Criar nova conexao\n");
        printf("0) Sair\n");
        printf("======================================\n");
        printf("Escolha uma opcao: ");

        if (scanf("%d", &opcao) != 1) {
            while (getchar() != '\n');
            opcao = -1;
        }
        while (getchar() != '\n');

        switch (opcao) {
            case 1: menu_listar_cidades(); break;
            case 2: menu_contar_conexoes(); break;
            case 3: menu_conexoes_ordenadas(); break;
            case 4: menu_distancia_entre_cidades(); break;
            case 5: menu_nova_conexao(); break;
            case 0: printf("Saindo do sistema...\n"); break;
            default: printf("Opcao invalida!\n");
        }

        if (opcao != 0) {
            printf("\nPressione ENTER para continuar...");
            getchar();
        }

    } while (opcao != 0);

    return 0;
}
