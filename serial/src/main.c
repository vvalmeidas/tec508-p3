/**
 * @file main.c
 * @brief Arquivo principal para execução da regressão logistica.
 * 
 * Esse arquivo contém os principais métodos para execução do algoritmo
 * de regressão logistica. A execução é iniciada pela função main(), que
 * realiza a chamada a todas as funções auxiliares (internas ou externas
 * ao arquivo) necessárias.
 * 
 * @author Nadine Cerqueira Marques (nadymarkes@gmail.com)
 * @author Valmir Vinicius de Almeida Santos (vvalmeida96@gmail.com)
 * @version 0.1
 * @date 12/10/2018
 * 
 * @copyright Copyright (c) 2018
 * 
 */

/* -- Includes -- */

/** Inclusão da biblioteca stdio **/
#include <stdio.h>

/** Inclusão da biblioteca stdlib **/
#include <stdlib.h>

/** Inclusão da biblioteca string **/
#include <string.h>

/** Inclusão da biblioteca errno **/
#include <errno.h>

/** Inclusão da biblioteca math **/
#include <math.h>

/** Inclusão da biblioteca time **/
#include <time.h>

/** Inclusão do arquivo de cabeçalho responsável pela leitura do arquivo de entrada **/
#include "csv.h"


/**
 * @brief Constante definindo o número de imagens para teste.
 * 
 */
static const int NUM_IMAGES_TESTING = 1210;

/**
 * @brief Constante definindo o número de pixels.
 * 
 */
static const int NUM_PIXELS = 128 * 128;



/**
 * @brief Realiza alocação dinâmica para uma string.
 * 
 * Realiza a alocação dinâmica para um espaço de memória de acordo com
 * a string informada.
 * 
 * @param s string para a qual se deseja alocar um espaço de memória
 * @return void* ponteiro para o endereço de memória alocado
 */
char *estrdup (char *s) {
	char *t;
	t = (char *) malloc(strlen(s)+1);
	strcpy(t, s);
	return t;
}

/**
 * @brief Gera números reais aleatórios.
 * 
 * Gera números reais aleatórios de acordo com o intervalo entre
 * mínimo e máximo informado.
 * 
 * @param min valor mínimo
 * @param max valor máximo
 * @return float valor aleatório gerado 
 */
float rand_range(float min, float max) {
    return min + ((float)(max - min) * (rand() / (RAND_MAX + 1.0)));
}

/**
 * @brief Inicializa o vetor de pesos.
 * 
 * Inicializa o vetor de pesos com valores aleatórios gerados.
 * 
 * @param row referência para o vetor de pesos
 */
void initialize_weights(float *row, int num_total_images_training) {
    for(int i=0; i < NUM_PIXELS; i++) {
        *(row+i) = rand_range(-1, 1) / (NUM_PIXELS + num_total_images_training);
    }
}

/**
 * @brief Realiza a leitura completa do arquivo .csv de entrada.
 * 
 * Realiza a leitura completa do arquivo .csv de entrada e armazena
 * os pixels lidos nas matrizes para dados de teste e treinamento. Os
 * labels lidos são armazenados nos vetores de treinamento e de teste.
 * 
 * @param testing_images_names[NUM_IMAGES_TESTING][60] array contendo os nomes das imagens de teste
 * @param file_log_output ponteiro para escrita no log de saída
 * @param data_testing referência para matriz com dados para teste
 * @param data_training referência para matriz com dados para treinamento
 * @param labels_testing referência para vetor com labels para teste
 * @param labels_training referência para vetor com labels para treinamento
 * 
 * @return int 1, se a leitura foi bem sucedida; 0, caso contrário
 */
int read_data_and_labels(char testing_images_names[NUM_IMAGES_TESTING][60], FILE *file_log_output, float **data_testing, float **data_training, int *labels_testing, int *labels_training, int num_total_images_training) {
    char *line; //linha lida do arquivo
    int row_testing = 0; //contador para linhas da matriz com dados para teste
    int row_training = 1; //contador para linhas da matriz com dados para treinamento
    char *label, *pixels, *usage, *pch, *name; 
    float temp; 
    FILE *file_input;
    char *file_name;
    int file_cont = 0;

    /** aloca espaço para o bias e adiciona o bias ao dataset de treinamento **/
    data_training[0] = (float *) malloc(NUM_PIXELS * sizeof(float));
    for(int i = 0; i < NUM_PIXELS; i++) {
        data_training[0][i] = 0;
    } 
    labels_training[0] = 1;

    while (file_cont < 5 && row_training != num_total_images_training) {

        if(file_cont == 0) {
            file_name = estrdup("../../data/fold_0_after.csv");
        } else if(file_cont == 1) {
            file_name = estrdup("../../data/fold_1_after.csv");
        } else if(file_cont == 2) {
            file_name = estrdup("../../data/fold_2_after.csv");
        } else if(file_cont == 3) {
            file_name = estrdup("../../data/fold_3_after.csv");
        } else if(file_cont == 4) {
            file_name = estrdup("../../data/fold_4_after.csv");
        }

        /* tenta abrir o arquivo de entrada */
        if ((file_input = fopen(file_name, "r")) == NULL) {
            fprintf(file_log_output, "Não foi possível abrir o arquivo!");
            return -1;
        }

        /* itera o arquivo até o final, ou seja, até a linha obtida ser NULL */
        while(((line = csvgetline(file_input, ',', 0)) != NULL) && row_training != num_total_images_training) {
            name = estrdup(csvfield(0)); //obtém o nome do arquivo 
            label = estrdup(csvfield(1)); //obtém o primeiro campo de uma linha (label)
            pixels = estrdup(csvfield(2)); //obtém o segundo campo de uma linha (pixels)

            pch = strtok(pixels, " "); //separa os pixels por espaços

            if(file_cont == 0) { //verifica se a linha possui dados para teste
                data_testing[row_testing] = (float *) malloc(NUM_PIXELS * sizeof(float));
                labels_testing[row_testing] = atoi(label); //converte a label para inteiro e divide por 4, tornando-a 1 ou 0
                strcpy(testing_images_names[row_testing], estrdup(csvfield(0)));
            } else {
                data_training[row_training] = (float *) malloc(NUM_PIXELS * sizeof(float));
                labels_training[row_training] = atoi(label);
            }

            /** realiza iteração para cada pixel que será lido **/
            for(int c = 0; c < NUM_PIXELS; c++) {
                temp = strtof(pch, NULL); //obtém 1 pixel
                temp = temp/255; //realiza normalização no pixel

                if(file_cont == 0) { 
                    data_testing[row_testing][c] = temp;
                } else {
                    data_training[row_training][c] = temp;
                }

                pch = strtok(NULL, " ");
            }

            if(file_cont == 0) {
                row_testing++; //atualiza a contagem das linhas de teste
            } else {
                row_training++; //atualiza a contagem das linhas de treinamento
            }
        }

        fclose(file_input);
        file_cont++;
    }

    return 0;
}

/**
 * @brief Salva os resultados do treinamento em arquivo
 * 
 * @param epoch_num número da epoca
 * @param results vetor contendo resultados da regressão logistica
 * @param labels labels lidas do arquivo (valores corretos)
 * @param num_images número de imagens que foram processadas
 * @param file_log_output ponteiro para o arquivo de log de saída
 * @param file_accuracy_output ponteiro para o arquivo com registros de acurácia
 * @param file_precision_output ponteiro para o arquivo com registros de precisão
 * @param file_f1_output ponteiro para o arquivo com registros de f1
 * @param file_recall_output ponteiro para o arquivo com registros de recall
 */
void save_training_results(int epoch_num, int *results, int *labels, int num_images, FILE *file_log_output, FILE *file_accuracy_output, FILE *file_precision_output, FILE *file_f1_output, FILE *file_recall_output) {
    int true_positive = 0, true_negative = 0, false_positive = 0, false_negative = 0;
    float accuracy = 0, precision = 0, recall = 0, f1 = 0;

    /** computa os verdadeiros positivos e negativos e os falsos positivos e negativos **/
    for(int i=0; i < num_images; i++) {
        if(results[i] == 1 && labels[i] == 1) {
            true_positive++;
        } else if(results[i] == 1 && labels[i] == 0) {
            false_positive++;
        } else if(results[i] == 0 && labels[i] == 1) {
            false_negative++;
        } else {
            true_negative++;
        }
    }

    accuracy = (float) (true_positive + true_negative) / (true_positive + true_negative + false_positive + false_negative);
    precision = (float) (true_positive) / (true_positive + false_positive); 
    recall = (float) true_positive/(true_positive + false_negative);
    f1 = 2 * ((precision * recall) / (precision + recall));

    fprintf(file_log_output, "-- ÉPOCA %d --\n", epoch_num + 1);
    fprintf(file_log_output, "MATRIZ DE CONFUSÃO:\n");
    fprintf(file_log_output, "%d    %d\n", true_negative, false_positive);
    fprintf(file_log_output, "%d    %d\n\n", false_negative, true_positive);

    fprintf(file_log_output, "Acertos: %d       Erros: %d\n", true_positive + true_negative, false_positive + false_negative);
    fprintf(file_log_output, "Acurácia: %f      Precisão: %f        Revocação: %f       F1: %f\n", accuracy, precision, recall, f1);
    fprintf(file_accuracy_output, "%d,%f\n", epoch_num + 1, accuracy);
    fprintf(file_precision_output, "%d,%f\n", epoch_num + 1, precision);
    fprintf(file_recall_output, "%d,%f\n", epoch_num + 1, recall);
    fprintf(file_f1_output, "%d,%f\n", epoch_num + 1, f1);
}

/**
 * @brief Salva os resultados do teste em arquivo
 * 
 * @param results vetor contendo resultados da regressão logistica
 * @param labels labels lidas do arquivo (valores corretos)
 * @param num_images número de imagens que foram processadas
 * @param testing_images_names[NUM_IMAGES_TESTING][60] array contendo os nomes das imagens de teste
 * @param file_log_output ponteiro para escrita no log de saída
 * @param file_csv_output ponteiro para escrita no arquivo csv de saída
 */
void save_testing_results(int *results, int *labels, int num_images, char testing_images_names[NUM_IMAGES_TESTING][60], FILE *file_log_output, FILE *file_csv_output) {
    int true_positive = 0, true_negative = 0, false_positive = 0, false_negative = 0;
    float accuracy = 0, precision = 0, recall = 0, f1 = 0;

    fprintf(file_csv_output, "%s,%s,%s\n", "nome_da_imagem", "genero", "genero_predito");

    /** computa os verdadeiros positivos e negativos e os falsos positivos e negativos **/
    for(int i=0; i < num_images; i++) {
        if(results[i] == 1 && labels[i] == 1) {
            true_positive++;
        } else if(results[i] == 1 && labels[i] == 0) {
            false_positive++;
        } else if(results[i] == 0 && labels[i] == 1) {
            false_negative++;
        } else {
            true_negative++;
        }

        fprintf(file_csv_output, "%s,%c,%c\n", testing_images_names[i], labels[i] == 0 ? 'f' : 'm', results[i] == 0 ? 'f' : 'm');
    }

    accuracy = (float) (true_positive + true_negative) / (true_positive + true_negative + false_positive + false_negative);
    precision = (float) (true_positive) / (true_positive + false_positive); 
    recall = (float) true_positive/(true_positive + false_negative);
    f1 = 2 * ((precision * recall) / (precision + recall));

    fprintf(file_log_output, "MATRIZ DE CONFUSÃO:\n");
    fprintf(file_log_output, "%d    %d\n", true_negative, false_positive);
    fprintf(file_log_output, "%d    %d\n\n", false_negative, true_positive);

    fprintf(file_log_output, "Acertos: %d       Erros: %d\n", true_positive + true_negative, false_positive + false_negative);
    fprintf(file_log_output, "Acurácia: %f      Precisão: %f        Revocação: %f       F1: %f\n", accuracy, precision, recall, f1);
}

/**
 * @brief Realiza o cálculo da função hipótese.
 * 
 * Realiza o cálculo da função hipótese de acordo com uma
 * linha da matriz e com o vetor de pesos informado.
 * 
 * @param row linha da matriz
 * @param weights vetor de pesos
 * @return float resultado da função hipotese
 */
float hypothesis_function(float *row, float *weights) {
    float result = 0;

    for(int c=0; c < NUM_PIXELS; c++) {
        result += weights[c] * row[c];
    }

    return 1/(1 + exp(-result)); //aplica a função sigmoid e retorna o resultado
}

/**
 * @brief Realiza o cálculo do gradiente descendente.
 * 
 * Realiza o cálculo do gradiente descendente de acordo com o dataset de treinamento
 * informado, labels de treinamento e valores de hipótese.
 * 
 * @param data_training dataset de treinamento
 * @param labels labels de treinamento
 * @param all_hypothesis valores calculados para hipótese
 * @param c valor atual da coluna
 * @param learning_rate taxa de aprendizado
 * @return float valor de gradiente resultante
 */
float gradient(float **data_training, int *labels, float *all_hypothesis, int c, float learning_rate, int num_total_images_training) {
    float gradient_sum = 0;

    for(int r = 0; r < num_total_images_training; r++) {
        gradient_sum += (all_hypothesis[r] - labels[r]) * data_training[r][c];
    }

    return gradient_sum * learning_rate;
}

/**
 * @brief Realiza a atualização dos pesos.
 * 
 * Realiza a atualização do vetor de pesos após a
 * execução de uma época de treinamento.
 * 
 * @param data_training dataset de treinamento
 * @param weights vetor de pesos
 * @param all_hypothesis valores calculados para hipótese
 * @param labels labels de treinamento
 * @param learning_rate taxa de aprendizado
 */
void update_weights(float **data_training, float *weights, float *all_hypothesis, int *labels, float learning_rate, int num_total_images_training) {

    for(int c=0; c < NUM_PIXELS; c++) {
        weights[c] = weights[c] - (gradient(data_training, labels, all_hypothesis, c, learning_rate, num_total_images_training)/num_total_images_training);
    }

}

/**
 * @brief Realiza o cálculo da função de custo.
 * 
 * Realiza o cálculo da função de custo.
 * 
 * @param all_hypothesis vetor com valores calculados para hipótese
 * @param weights vetor de pesos
 * @param labels vetor de labels
 * @param num_total_images_training número de imagens
 * @return float custo resultante
 */
float cost_function(float *all_hypothesis, float *weights, int *labels, int num_total_images_training) {
    float cost = 0;

    for(int r=0; r < num_total_images_training; r++) {
        cost += -(labels[r] * log(all_hypothesis[r])) - (1 - labels[r]) * log(1 - all_hypothesis[r]);
    }

    return cost / num_total_images_training;
}

/**
 * @brief Função principal, na qual é iniciada a execução do algoritmo.
 * 
 * Função principal, na qual é iniciada a execução do algoritmo de acordo
 * com os argumntos informados no terminal para número de épocas e taxa
 * de aprendizagem.
 * 
 * @param argc quantidade de argumentos
 * @param argv vetor contendo os argumentos número de épocas e taxa de aprendizado
 * @return int 0, se a execução foi finalizada sem erros; -1, caso contrário
 */
int main(int argc, char *argv[]) {
    /** obtém os argumentos, converte para int ou float e inicializa o número de 
     * épocas e a taxa de aprendizado **/
    int num_max_epochs = atoi(argv[1]);
    float learning_rate = atof(argv[2]);
    int num_total_images_training = atoi(argv[3]);

    /* vetor de pesos */
    float *weights = (float *) malloc(NUM_PIXELS * sizeof(float));
    /* número de épocas */
    int num_epochs = 0;

    int corretos = 0;
    
    /* vetor contendo todos os valores de hipóteses calculados na épca */
    float *all_hypothesis = (float *) malloc(num_total_images_training * sizeof(float));

    /* vetor contendo os resultados, ou seja, os valores de hipótese binarizados */
    int *results = (int *) malloc(num_total_images_training * sizeof(int));

    int *results_testing = (int *) malloc(NUM_IMAGES_TESTING * sizeof(int));

    /* ponteiro para o arquivo de entrada */
    FILE *file_input;

    /* ponteiro para o arquivos de log de saída */
    FILE *file_log_output, *file_csv_output;

    /* ponteiro para o arquivo de dados de saída */
    FILE *file_cost_output, *file_accuracy_output, *file_precision_output, *file_recall_output, *file_f1_output;

    /* linha do arquivo */
    char *line; 

    /* matrizes com dados para teste e treinamento */
    float **data_testing, **data_training; 

    /* vetores com labels para teste e treinamento */
    int *labels_testing, *labels_training;

    /* nomes das imagens lidas */
    char testing_images_names[NUM_IMAGES_TESTING][60];

    char filename[400], filename2[400];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(filename, sizeof(filename)-1, "../output/%Y%m%d-%H%M-output.txt", t);
    strftime(filename2, sizeof(filename2)-1, "../output/%Y%m%d-%H%M-output.csv", t);

    /* cria o arquivo log de saída */
    file_log_output = fopen(filename, "w");
    file_csv_output = fopen(filename2, "w");

    char file_name_graphics[80], *file_name_middle = "_pdataset_", *file_name_end = "_epochs_output.csv";

    /* cria os arquivos de saída de dados */
    strcpy(file_name_graphics, "../graphics/cost_");
    strcat(file_name_graphics, argv[3]);
    strcat(file_name_graphics, file_name_middle);
    strcat(file_name_graphics, argv[1]);
    strcat(file_name_graphics, file_name_end);

    file_cost_output = fopen(file_name_graphics, "w");

    strcpy(file_name_graphics, "../graphics/accuracy_");
    strcat(file_name_graphics, argv[3]);
    strcat(file_name_graphics, file_name_middle);
    strcat(file_name_graphics, argv[1]);
    strcat(file_name_graphics, file_name_end);

    file_accuracy_output = fopen(file_name_graphics, "w");

    strcpy(file_name_graphics, "../graphics/precision_");
    strcat(file_name_graphics, argv[3]);
    strcat(file_name_graphics, file_name_middle);
    strcat(file_name_graphics, argv[1]);
    strcat(file_name_graphics, file_name_end);

    file_precision_output = fopen(file_name_graphics, "w");

    strcpy(file_name_graphics, "../graphics/recall_");
    strcat(file_name_graphics, argv[3]);
    strcat(file_name_graphics, file_name_middle);
    strcat(file_name_graphics, argv[1]);
    strcat(file_name_graphics, file_name_end);

    file_recall_output = fopen(file_name_graphics, "w");

    strcpy(file_name_graphics, "../graphics/f1_");
    strcat(file_name_graphics, argv[3]);
    strcat(file_name_graphics, file_name_middle);
    strcat(file_name_graphics, argv[1]);
    strcat(file_name_graphics, file_name_end);

    file_f1_output = fopen(file_name_graphics, "w");

    /* realiza alocação de espaços de memórias para matrizes e vetores usados */
    data_testing = (float **) malloc(NUM_IMAGES_TESTING * sizeof(float *));
    data_training = (float **) malloc(num_total_images_training * sizeof(float *));
    labels_testing = (int *) malloc(NUM_IMAGES_TESTING * sizeof(int));
    labels_training = (int *) malloc(num_total_images_training * sizeof(int));
    
    if(read_data_and_labels(testing_images_names, file_log_output, data_testing, data_training, labels_testing, labels_training, num_total_images_training) == -1) {
        return -1;
    }

    initialize_weights(weights, num_total_images_training);

    fprintf(file_log_output, "RESULTADO - TREINAMENTOS:\n");
    fprintf(file_log_output, "NÚMERO DE AMOSTRAS: %d  /  NÚMERO DE ÉPOCAS: %d  /  TAXA DE APRENDIZADO: %f\n", num_total_images_training, num_max_epochs, learning_rate);
    fprintf(file_log_output, "NÚMERO DE THREADS: %d\n\n\n", atoi(argv[3]));

    /* realiza iterações até o número máximo de épocas */
    while (num_epochs < num_max_epochs) {

        /* realiza iterações de acordo com o número de imagens para treinamento */
        for(int r=0; r < num_total_images_training; r++) {

            all_hypothesis[r] = hypothesis_function((float *) data_training[r], weights);

            //realiza binarização dos valores de hipótese
            if(all_hypothesis[r] >= 0.5) {
                results[r] = 1;
            } else {
                results[r] = 0;
            }
        }

        save_training_results(num_epochs, results, labels_training, num_total_images_training, file_log_output, file_accuracy_output, file_precision_output, file_f1_output, file_recall_output);
        fprintf(file_cost_output, "%d,%f\n", num_epochs+1, cost_function(all_hypothesis, weights, labels_training, num_total_images_training));
        fprintf(file_log_output, "Custo:    %f\n\n", cost_function(all_hypothesis, weights, labels_training, num_total_images_training));
        update_weights(data_training, weights, all_hypothesis, labels_training, learning_rate, num_total_images_training);
        num_epochs++;
    }

    fclose(file_cost_output);
    fclose(file_accuracy_output);
    fclose(file_f1_output);
    fclose(file_precision_output);
    fclose(file_recall_output);

    fprintf(file_log_output, "\n\n\nRESULTADO - TESTE:\n");
    fprintf(file_log_output, "NÚMERO DE AMOSTRAS: %d  /  TAXA DE APRENDIZADO: %f\n\n\n", NUM_IMAGES_TESTING, learning_rate);

    float hypothesis = 0;

    //executa a etapa de testes
    for(int r=0; r < NUM_IMAGES_TESTING; r++) {
        hypothesis = hypothesis_function((float *) data_testing[r], weights);
                 
        //realiza binarização dos valores de hipótese
        if(hypothesis >= 0.5) {
            results_testing[r] = 1;
        } else {
            results_testing[r] = 0;
        }
    }


    save_testing_results(results_testing, labels_testing, NUM_IMAGES_TESTING, testing_images_names, file_log_output, file_csv_output);

    fclose(file_log_output);
    fclose(file_csv_output);
}
