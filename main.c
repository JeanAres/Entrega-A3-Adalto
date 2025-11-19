#include <stdio.h>
#include <string.h>

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