#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stdlib.h>    // ADICIONADO para malloc, free, exit

#define MAX_CITIES 1000
#define MAX_NAME 100
#define LINE_BUF 256


typedef struct Edge {
    int to;
    int weight;
    struct Edge *next;
} Edge;

char city_names[MAX_CITIES][MAX_NAME];
int city_count = 0;
Edge *adj[MAX_CITIES] = {NULL};

void str_to_lower_trim(char *s) {
    char *p = s;
    while (*p) {*p = tolower((unsigned char)*p); p++;}
    char *start =s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start)+1);
    int len = strlen(s);
    while (len>0 && isspace((unsigned char)s[len-1])) s[--len] = '\0';
}

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
    strncpy(city_names[city_count], name_in,MAX_NAME-1);
    city_names[city_count][MAX_NAME-1] = '\0';
    city_count++;
    return city_count - 1;
}

void add_edge(int a, int b, int w) {
    Edge *e1 = malloc(sizeof(Edge));
    e1->to = b; e1->weight = w; e1->next = adj[a]; adj[a] = e1;
    Edge *e2 = malloc(sizeof(Edge));
    e2->to = a; e2->weight = w; e2->next = adj[b]; adj[b] =e2;
}

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

int fuzzy_match_city(const char *input, char *matched_name_out) {
    char temp_input[MAX_NAME];
    strncpy(temp_input, input, MAX_NAME-1);
    temp_input[MAX_NAME-1] = '\0';
    str_to_lower_trim(temp_input);

    int best_idx = -1;
    int best_dist = INT_MAX;
    for (int i=0; i<city_count; ++i) {
        char tmp[MAX_NAME];
        strncpy(tmp, city_names[i], MAX_NAME-1);
        tmp[MAX_NAME-1] = '\0';
        str_to_lower_trim(tmp);
        int d = levenshtein(temp_input, tmp);
        if (d < best_dist) {
            best_dist = d;
            best_idx = i;
        }
    }
    if (best_idx >= 0) {
        strncpy(matched_name_out, city_names[best_idx], MAX_NAME-1);
        matched_name_out[MAX_NAME-1] = '\0';
    }
    return best_idx;
}

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

int edge_weight_between(int a, int b) {
    for (Edge *e = adj[a]; e != NULL; e = e->next) {
        if (e->to == b) return e->weight;
    }
    return -1; 
}

int main() {
    FILE *arquivo;
    char origem[50], destino[50];
    int distancia;
    int contador = 0;
    
    // Abre o arquivo CSV
    arquivo = fopen("cidades_rs_grafo.csv", "r");

    // Verifica se o arquivo foi aberto com sucesso
    if (arquivo == NULL) {
        printf("ERRO: Nao foi possivel abrir o arquivo!\n");
        printf("Certifique-se de que 'cidades_rs_grafo.csv' esta na mesma pasta do executavel.\n");
        return 1;
    }
    
    printf("Arquivo aberto com sucesso!\n");
    printf("=========================================\n\n");
    
    // Ignora a primeira linha (cabeçalho)
    char cabecalho[150];
    fgets(cabecalho, sizeof(cabecalho), arquivo);
    
    printf("Lendo conexoes entre cidades:\n\n");
    
    // Lê cada linha do arquivo
    while (fscanf(arquivo, " %[^,],%[^,],%d\n", origem, destino, &distancia) == 3) {
        contador++;
        printf("%2d. %s -> %s = %d km\n", contador, origem, destino, distancia);
    }
    
    printf("\n=========================================\n");
    printf("Total de conexoes lidas: %d\n", contador);
    
    // Fecha o arquivo
    fclose(arquivo);
    
    printf("\nLeitura concluida com sucesso!\n");
    
    return 0;
}
