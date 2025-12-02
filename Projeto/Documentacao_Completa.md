# Documentação Completa do Sistema de Gerenciamento de Grafo de Cidades

## Índice
1. [Visão Geral](#visão-geral)
2. [Estruturas de Dados](#estruturas-de-dados)
3. [Funções Utilitárias](#funções-utilitárias)
4. [Algoritmo de Dijkstra](#algoritmo-de-dijkstra)
5. [Algoritmo de Levenshtein](#algoritmo-de-levenshtein)
6. [Funções de Menu](#funções-de-menu)
7. [Função Principal](#função-principal)

---

## Visão Geral

Este sistema implementa um gerenciamento de grafo de cidades do Rio Grande do Sul, permitindo:
- Cadastro e listagem de cidades
- Cálculo de distâncias e trajetos entre cidades
- Busca aproximada de cidades (tolerante a erros de digitação)
- Contagem de conexões por cidade
- Criação de novas conexões
- Sugestão de rotas parciais quando não há caminho direto

---

## Estruturas de Dados

### 1. Inclusões e Definições (linhas 1-9)

```c
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_CITIES 1000
#define MAX_NAME 100
```

**Explicação:**
- Inclui bibliotecas padrão do C para I/O, manipulação de strings, limites numéricos, caracteres e alocação de memória
- Define constantes: máximo de 1000 cidades e nomes com até 100 caracteres

### 2. Estrutura Edge (linhas 11-15)

```c
typedef struct Edge {
    int to;
    int weight;
    struct Edge *next;
} Edge;
```

**Explicação:**
- Representa uma aresta do grafo usando lista ligada
- `to`: índice da cidade destino
- `weight`: distância em km
- `next`: ponteiro para próxima aresta (lista de adjacências)

### 3. Estrutura VizinhoInfo (linhas 18-21)

```c
typedef struct {
    int city_id;
    int distance;
} VizinhoInfo;
```

**Explicação:**
- Usada para listar vizinhos de uma cidade ordenados por distância
- Armazena ID da cidade e distância até ela

### 4. Estrutura ConexaoCount (linhas 24-28)

```c
typedef struct {
    int city_id;
    int count;
    char name[MAX_NAME];
} ConexaoCount;
```

**Explicação:**
- Usada para contar e ordenar conexões por cidade
- Armazena ID, quantidade de conexões e nome da cidade

### 5. Variáveis Globais (linhas 30-32)

```c
char city_names[MAX_CITIES][MAX_NAME];
int city_count = 0;
Edge *adj[MAX_CITIES] = {NULL};
```

**Explicação:**
- `city_names`: array com nomes de todas as cidades
- `city_count`: contador de cidades cadastradas
- `adj`: array de listas ligadas representando o grafo (lista de adjacências)

---

## Funções Utilitárias

### 1. str_to_lower_trim (linhas 37-45)

```c
void str_to_lower_trim(char *s) {
    char *p = s;
    while (*p) {*p = tolower((unsigned char)*p); p++;}
    char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    if (start != s) memmove(s, start, strlen(start)+1);
    int len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len-1])) s[--len] = '\0';
}
```

**Explicação:**
- Normaliza strings: converte para minúsculas e remove espaços nas extremidades
- Usado para comparações case-insensitive e remoção de espaços extras

### 2. city_index (linhas 48-69)

```c
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
```

**Explicação:**
- Retorna o índice de uma cidade no array
- Se a cidade não existir, cria uma nova entrada
- Normaliza nomes para comparação (case-insensitive)
- Retorna erro se exceder o limite de cidades

### 3. add_edge (linhas 72-77)

```c
void add_edge(int a, int b, int w) {
    Edge *e1 = malloc(sizeof(Edge));
    e1->to = b; e1->weight = w; e1->next = adj[a]; adj[a] = e1;
    Edge *e2 = malloc(sizeof(Edge));
    e2->to = a; e2->weight = w; e2->next = adj[b]; adj[b] = e2;
}
```

**Explicação:**
- Adiciona aresta não direcionada entre duas cidades
- Cria duas arestas (uma em cada direção) pois o grafo é não direcionado
- Insere no início da lista de adjacências (estrutura LIFO)

---

## Algoritmo de Dijkstra

### O que é?

O **Algoritmo de Dijkstra** é um algoritmo de busca em grafo que resolve o problema do caminho mais curto de fonte única em um grafo com arestas de peso não negativo. Ele encontra o caminho de menor custo de um vértice origem para todos os outros vértices do grafo.

### Implementação no Código (linhas 147-167)

```c
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
```

### Como Funciona - Passo a Passo

#### 1. Inicialização (linhas 148-149)
```c
for (int i=0;i<city_count;++i) { dist[i] = INT_MAX; prev[i] = -1; }
dist[src] = 0;
```
- Inicializa todas as distâncias como infinito (`INT_MAX`)
- Inicializa todos os predecessores como -1 (sem predecessor)
- Define a distância da origem como 0

#### 2. Loop Principal (linhas 152-166)
```c
for (int it=0; it<city_count; ++it) {
    // Seleciona vértice não visitado com menor distância
    int u = -1;
    int best = INT_MAX;
    for (int i=0;i<city_count;++i) 
        if (!visited[i] && dist[i] < best) { 
            best = dist[i]; 
            u = i; 
        }
    
    if (u == -1) break;  // Não há mais vértices acessíveis
    visited[u] = 1;      // Marca como visitado
    
    // Relaxa arestas
    for (Edge *e = adj[u]; e != NULL; e = e->next) {
        int v = e->to;
        int w = e->weight;
        if (!visited[v] && dist[u] != INT_MAX && dist[u] + w < dist[v]) {
            dist[v] = dist[u] + w;  // Atualiza distância
            prev[v] = u;             // Atualiza predecessor
        }
    }
}
```

**Processo:**
1. **Seleção**: Escolhe o vértice não visitado com menor distância conhecida
2. **Marca como visitado**: Garante que não será processado novamente
3. **Relaxação**: Para cada vizinho não visitado:
   - Se `dist[origem] + peso < dist[vizinho]`, atualiza a distância
   - Atualiza o predecessor para reconstruir o caminho depois

### Exemplo Visual

```
Grafo:
    A ──5──> B
    │        │
    3        2
    │        │
    v        v
    C ──1──> D

Execução com origem = A:

Iteração 1: u = A (dist=0)
  - Relaxa B: dist[B] = 5, prev[B] = A
  - Relaxa C: dist[C] = 3, prev[C] = A

Iteração 2: u = C (dist=3, menor não visitado)
  - Relaxa D: dist[D] = 3+1 = 4, prev[D] = C

Iteração 3: u = B (dist=5)
  - Relaxa D: dist[D] = min(4, 5+2) = 4 (não atualiza)

Iteração 4: u = D (dist=4)

Resultado:
  dist[A] = 0, prev[A] = -1
  dist[B] = 5, prev[B] = A
  dist[C] = 3, prev[C] = A
  dist[D] = 4, prev[D] = C

Caminho A→D: A → C → D (distância = 4)
```

### Onde é Usado no Código

1. **Cálculo de distância entre cidades** (linha 291):
   ```c
   dijkstra(origem, dist, prev);
   ```

2. **Reconstrução de caminho completo** (linhas 299-308):
   - Usa o array `prev[]` para reconstruir o caminho da origem ao destino

3. **Sugestão de rotas parciais** (linha 352):
   - Executa Dijkstra novamente a partir de uma cidade intermediária para encontrar caminho até o destino

### Complexidade

- **Tempo**: O(V²) onde V é o número de vértices (cidades)
- **Espaço**: O(V) para arrays de distância e predecessores

---

## Algoritmo de Levenshtein

### O que é?

O **Algoritmo de Levenshtein** (também conhecido como distância de edição) calcula o número mínimo de operações de edição (inserção, deleção ou substituição de caracteres) necessárias para transformar uma string em outra.

### Implementação no Código (linhas 80-104)

```c
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
```

### Como Funciona - Passo a Passo

#### 1. Casos Base (linhas 82-83)
```c
if (n == 0) return m;  // Se s é vazia, precisa m inserções
if (m == 0) return n;  // Se t é vazia, precisa n deleções
```

#### 2. Programação Dinâmica (linhas 87-100)

O algoritmo usa programação dinâmica com otimização de espaço (usa apenas 2 arrays em vez de matriz completa):

```c
for (int i=1;i<=n;++i) {
    v1[0] = i;  // Custo de transformar s[0..i-1] em string vazia
    for (int j=1;j<=m;++j) {
        int cost = (s[i-1] == t[j-1]) ? 0 : 1;  // Custo de substituição
        int deletion = v0[j] + 1;      // Deletar s[i-1]
        int insertion = v1[j-1] + 1;   // Inserir t[j-1]
        int substitution = v0[j-1] + cost;  // Substituir s[i-1] por t[j-1]
        
        // Escolhe a operação de menor custo
        int v = deletion;
        if (insertion < v) v = insertion;
        if (substitution < v) v = substitution;
        v1[j] = v;
    }
    memcpy(v0, v1, (m+1)*sizeof(int));  // Atualiza para próxima iteração
}
```

**Operações:**
- **Deleção**: Remove um caractere de `s` → `v0[j] + 1`
- **Inserção**: Adiciona um caractere em `s` → `v1[j-1] + 1`
- **Substituição**: Troca um caractere → `v0[j-1] + cost` (0 se igual, 1 se diferente)

### Exemplo Visual

Calcular distância entre "casa" e "cara":

```
Matriz (conceitual):
        ""  c   a   r   a
    ""   0   1   2   3   4
    c    1   0   1   2   3
    a    2   1   0   1   2
    s    3   2   1   1   2
    a    4   3   2   2   1

Cálculo passo a passo:
- "c" vs "c": 0 (iguais)
- "ca" vs "ca": 0 (iguais)
- "cas" vs "car": 
  - Substituir 's' por 'r': 0 + 1 = 1
- "casa" vs "cara":
  - Substituir 's' por 'r': 1 + 1 = 2
  - Mas "cas" vs "ca" = 1, então "casa" vs "cara" = 1 + 1 = 2
  - Na verdade, melhor: deletar 's' de "casa" → "caa", inserir 'r' → "cara" = 2

Resultado: distância = 2
```

### Onde é Usado no Código

#### 1. Busca Aproximada de Cidades (linha 129)

```c
int fuzzy_match_city(const char *input, char *matched_name_out) {
    // ...
    int d = levenshtein(temp_input, tmp);
    if (d < best_dist) {
        best_dist = d;
        best_idx = i;
    }
    // ...
    if (best_idx >= 0 && best_dist <= threshold) {
        return best_idx;
    }
    return -1;
}
```

**Funcionalidade:**
- Permite encontrar cidades mesmo com erros de digitação
- Exemplo: usuário digita "porto alegr" → encontra "Porto Alegre" (distância = 1)
- Threshold: até 2 erros para nomes curtos, até 4 para nomes longos

#### 2. Sugestão de Rotas Parciais (linha 342)

```c
// escolho par (u,v) entre componentes com heurística (Levenshtein nos nomes)
int best_u = -1, best_v = -1, best_lev = INT_MAX;
for (int u = 0; u < city_count; ++u) {
    if (comp[u] != comp_origem) continue;
    for (int v = 0; v < city_count; ++v) {
        if (comp[v] != comp_destino) continue;
        // ...
        int lev = levenshtein(a, b);
        if (lev < best_lev) { best_lev = lev; best_u = u; best_v = v; }
    }
}
```

**Funcionalidade:**
- Quando não há caminho direto entre duas cidades (componentes diferentes)
- Usa Levenshtein como heurística para escolher pontos de ligação
- Escolhe cidades com nomes mais similares entre componentes diferentes
- Exemplo: sugere ligar "Porto Alegre" e "Porto Velho" por terem nomes similares

### Complexidade

- **Tempo**: O(n × m) onde n e m são os tamanhos das strings
- **Espaço**: O(min(n, m)) com otimização (usa apenas 2 arrays)

---

## Funções de Menu

### 1. menu_listar_cidades (linhas 184-190)

```c
void menu_listar_cidades() {
    printf("\n--- Cidades Cadastradas (%d) ---\n", city_count);
    for (int i = 0; i < city_count; i++) {
        printf("%d. %s\n", i + 1, city_names[i]);
    }
    printf("---------------------------------\n");
}
```

**Explicação:**
- Lista todas as cidades cadastradas numeradas

### 2. menu_contar_conexoes (linhas 192-207)

```c
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
```

**Explicação:**
- Conta conexões de cada cidade percorrendo a lista de adjacências
- Ordena por quantidade de conexões (crescente) e depois por nome
- Exibe resultado ordenado

### 3. ler_cidade_input (linhas 210-223)

```c
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
```

**Explicação:**
- Lê nome de cidade do usuário
- Usa busca aproximada (fuzzy_match_city) para encontrar a cidade
- Repete até encontrar uma cidade válida
- Mostra qual cidade foi selecionada

### 4. menu_conexoes_ordenadas (linhas 226-249)

```c
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
```

**Explicação:**
- Solicita uma cidade ao usuário
- Coleta todos os vizinhos da cidade
- Ordena por distância (menor para maior)
- Exibe lista ordenada

### 5. menu_distancia_entre_cidades (linhas 280-373)

Esta é a função mais complexa. Ela:

1. **Lê origem e destino** usando busca aproximada
2. **Verifica se são a mesma cidade** (retorna 0 km)
3. **Executa Dijkstra** para calcular distâncias
4. **Se há caminho** (dist[destino] != INT_MAX):
   - Reconstrói e exibe o caminho completo
5. **Se não há caminho**:
   - Identifica componentes conexas usando DFS
   - Usa Levenshtein para escolher pontos de ligação entre componentes
   - Calcula rotas parciais até os pontos escolhidos
   - Sugere rota parcial indicando a ligação faltante

**Explicação Detalhada:**

```c
void menu_distancia_entre_cidades() {
    // 1. Lê origem e destino
    int origem = ler_cidade_input("Cidade de Origem: ");
    int destino = ler_cidade_input("Cidade de Destino: ");

    // 2. Verifica mesma cidade
    if (origem == destino) {
        printf("Distancia: 0 km (mesma cidade).\n");
        return;
    }

    // 3. Executa Dijkstra
    int dist[MAX_CITIES], prev[MAX_CITIES];
    dijkstra(origem, dist, prev);

    // 4. Se há caminho completo
    if (dist[destino] != INT_MAX) {
        // Reconstrói e exibe caminho
        // ...
        return;
    }

    // 5. Se não há caminho - sugere rota parcial
    // Identifica componentes
    int comp[MAX_CITIES];
    // ... marca componentes com DFS
    
    // Escolhe pontos de ligação usando Levenshtein
    // Calcula rotas parciais
    // Exibe sugestão
}
```

### 6. menu_nova_conexao (linhas 376-412)

```c
void menu_nova_conexao() {
    // Lê duas cidades e distância
    // Cria aresta no grafo
    // Salva no arquivo CSV
    FILE *f = fopen("cidades_rs_grafo.csv", "a");
    if (f == NULL) {
        printf("ERRO: Conexao criada na memoria, mas falha ao abrir arquivo para salvar!\n");
    } else {
        fprintf(f, "%s,%s,%d\n", city_names[id1], city_names[id2], dist);
        fclose(f);
        printf("Sucesso! Dados salvos em 'cidades_rs_grafo.csv'.\n");
    }
}
```

**Explicação:**
- Permite criar nova conexão entre duas cidades
- Adiciona ao grafo em memória
- Persiste no arquivo CSV para manter dados entre execuções

---

## Funções Auxiliares

### 1. dfs_mark_component (linhas 254-259)

```c
void dfs_mark_component(int u, int comp_id, int comp[]) {
    comp[u] = comp_id;
    for (Edge *e = adj[u]; e != NULL; e = e->next) {
        if (comp[e->to] == -1) dfs_mark_component(e->to, comp_id, comp);
    }
}
```

**Explicação:**
- Usa busca em profundidade (DFS) para marcar todos os vértices de uma componente conexa
- Componente conexa = conjunto de cidades conectadas entre si

### 2. reconstruct_path (linhas 262-277)

```c
int reconstruct_path(int prev[], int from, int to, int caminho[]) {
    int tam = 0;
    int cur = to;
    while (cur != -1) {
        caminho[tam++] = cur;
        if (cur == from) break;
        cur = prev[cur];
    }
    // Inverte o caminho (estava de trás para frente)
    for (int i = 0; i < tam/2; ++i) {
        int tmp = caminho[i];
        caminho[i] = caminho[tam-1-i];
        caminho[tam-1-i] = tmp;
    }
    return tam;
}
```

**Explicação:**
- Reconstrói o caminho usando o array `prev[]` do Dijkstra
- Caminho começa do destino e vai até a origem (precisa inverter)
- Retorna o tamanho do caminho

### 3. Funções de Comparação (linhas 169-180)

```c
int compare_vizinhos(const void *a, const void *b) {
    VizinhoInfo *va = (VizinhoInfo *)a;
    VizinhoInfo *vb = (VizinhoInfo *)b;
    return va->distance - vb->distance;  // Ordena por distância
}

int compare_conexoes(const void *a, const void *b) {
    ConexaoCount *ca = (ConexaoCount *)a;
    ConexaoCount *cb = (ConexaoCount *)b;
    if (ca->count != cb->count) return ca->count - cb->count;  // Primeiro por quantidade
    return strcmp(ca->name, cb->name);  // Depois por nome
}
```

**Explicação:**
- Funções de comparação para `qsort()`
- `compare_vizinhos`: ordena vizinhos por distância
- `compare_conexoes`: ordena por quantidade de conexões, depois por nome

---

## Função Principal

### main (linhas 415-476)

```c
int main() {
    // 1. Carrega grafo do CSV
    FILE *arquivo = fopen("cidades_rs_grafo.csv", "r");
    if (arquivo == NULL) {
        printf("ERRO CRITICO: Arquivo 'cidades_rs_grafo.csv' nao encontrado.\n");
        return 1;
    }

    // Lê cabeçalho
    char cabecalho[150];
    fgets(cabecalho, sizeof(cabecalho), arquivo);

    // Lê cada linha: origem,destino,distância
    while (fscanf(arquivo, " %[^,],%[^,],%d", origem, destino, &distancia) == 3) {
        int id_origem = city_index(origem);
        int id_destino = city_index(destino);
        add_edge(id_origem, id_destino, distancia);
    }
    fclose(arquivo);

    // 2. Menu interativo
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

        scanf("%d", &opcao);
        while (getchar() != '\n');  // Limpa buffer

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
```

**Explicação:**
1. **Carregamento**: Abre arquivo CSV e carrega todas as conexões no grafo
2. **Menu Loop**: Exibe menu e processa opções até usuário escolher sair (0)
3. **Tratamento de Input**: Limpa buffer após scanf para evitar problemas com entrada

---

## Resumo dos Algoritmos

### Tabela Comparativa

| Algoritmo | O que calcula | Complexidade | Usado para |
|-----------|---------------|--------------|------------|
| **Dijkstra** | Menor caminho em grafo | O(V²) | Distâncias e trajetos entre cidades |
| **Levenshtein** | Similaridade entre strings | O(n×m) | Busca aproximada e sugestão de ligações |

### Fluxo de Execução

```
1. main() carrega CSV → constrói grafo
2. Usuário escolhe opção do menu
3. Sistema processa:
   - Lista cidades
   - Conta conexões
   - Mostra vizinhos ordenados
   - Calcula distância (Dijkstra) + busca aproximada (Levenshtein)
   - Cria nova conexão
4. Volta ao menu ou sai
```

---

## Conclusão

Este sistema implementa um gerenciamento completo de grafo de cidades com:
- ✅ Estrutura de dados eficiente (lista de adjacências)
- ✅ Algoritmo de menor caminho (Dijkstra)
- ✅ Busca tolerante a erros (Levenshtein)
- ✅ Detecção de componentes conexas
- ✅ Sugestão inteligente de rotas parciais
- ✅ Persistência de dados (CSV)
- ✅ Interface amigável com menu interativo

Os algoritmos de **Dijkstra** e **Levenshtein** são fundamentais para o funcionamento do sistema, permitindo cálculos precisos de distâncias e busca flexível de cidades.

---

*Documentação gerada para o projeto de gerenciamento de grafo de cidades do Rio Grande do Sul*


