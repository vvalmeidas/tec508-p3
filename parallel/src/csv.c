/**
 * @file csv.c
 * @brief Biblioteca para leitura de arquivos .csv.
 * 
 * Esse arquivo contém os métodos utilizados para leitura de 
 * arquivos .csv. Os métodos são chamados pela função main()
 * no arquivo principal. Retirado do livro 'The Practice of Programming'.
 * 
 * @author Brian W. Kernighan
 * @author Rob Pike
 * @date 12/10/2018
 * 
 * @copyright Copyright (C) 1999 Lucent Technologies
 * 
 */

/* -- Includes -- */

/** Inclusão da biblioteca stdio **/
#include <stdio.h>

/** Inclusão da biblioteca string **/
#include <string.h>

/** Inclusão da biblioteca stdlib **/
#include <stdlib.h>

/** Inclusão da biblioteca assert **/
#include <assert.h>

#include "csv.h"

/* Indicação de fim de memória */
enum { NOMEM = -2 };          

/* String de linhas lidas da entrada */
static char *line    = NULL;  

/* Cópia da linha */
static char *sline   = NULL;  

/* Tamanho da linha */
static int  maxline  = 0;   

/* Pointeiro para campos */
static char **field  = NULL; 

/* Tamanho dos campos */
static int  maxfield = 0;   

/* Número de campos */
static int  nfield   = 0;   

/** protótipos de funções **/
static char *advquoted(char *p, char delim);
static int split(char delim, int compress);


/* endofline: check for and consume \r, \n, \r\n, or EOF */
/**
 * @brief Verifica se o fim do arquivo foi atingido.
 * 
 * Verifica se o fim do arquivo foi atingido.
 * 
 * @param fin ponteiro para o arquivo
 * @param c caractere
 * @return int 1, se o fim do arquivo foi atingido; 0, caso contrário
 */
static int endofline(FILE *fin, int c)
{
	int eol;

	eol = (c=='\r' || c=='\n');
	if (c == '\r') {
		c = getc(fin);
		if (c != '\n' && c != EOF)
			ungetc(c, fin);
	}
	return eol;
}

/* reset: set variables back to starting values */
/**
 * @brief Reseta a leitura do arquivo.
 * 
 * Reseta a leitura do arquivo, ajustando as 
 * variáveis com seus valores iniciais.
 * 
 */
static void reset(void)
{
	free(line);
	free(sline);
	free(field);
	line = NULL;
	sline = NULL;
	field = NULL;
	maxline = maxfield = nfield = 0;
}

/**
 * @brief Obtém uma linha do arquivo.
 * 
 * Obtém uma linha do arquivo.
 * 
 * @param fin ponteiro para o arquivo
 * @param delim delimitador de linha
 * @param compress 
 * @return char* string contendo a linha lida
 */
char *csvgetline(FILE *fin, char delim, int compress)
{
	int i, c;
	char *newl, *news;

	if (line == NULL) {			/* realiza alocaççoes na primeira leitura */
		maxline = maxfield = 1;
		line = (char *) malloc(maxline);
		sline = (char *) malloc(maxline);
		field = (char **) malloc(maxfield*sizeof(field[0]));
		if (line == NULL || sline == NULL || field == NULL) {
			reset();
			return NULL;		/* estouro de memória */
		}
	}
	for (i=0; (c=getc(fin))!=EOF && !endofline(fin,c); i++) {
		if (i >= maxline-1) {	/* cresce a linha */
			maxline *= 2;		/* dobra o tamanho atual */
			newl = (char *) realloc(line, maxline);
			if (newl == NULL) {
				reset();
				return NULL;
			}
			line = newl;
			news = (char *) realloc(sline, maxline);
			if (news == NULL) {
				reset();
				return NULL;
			}
			sline = news;


		}
		line[i] = c;
	}
	line[i] = '\0';
	if (split(delim, compress) == NOMEM) {
		reset();
		return NULL;			/* estouro de memória */
	}
	return (c == EOF && i == 0) ? NULL : line;
}

/**
 * @brief Separa a linha em campos.
 * 
 * Separa a linha em campos.
 * 
 * @param delim delimitador de campos
 * @param compress 
 * @return int número de campos
 */
static int split(char delim, int compress)
{
	char *p, **newf;
	char *sepp; /* ponteiro para caractere separador temporário */
	int sepc;   /* caractere separador temporário */

	nfield = 0;
	if (line[0] == '\0')
		return 0;
	strcpy(sline, line);
	p = sline;

	do {
		if (nfield >= maxfield) {
			maxfield *= 2;			/* dobra o tamanho atual */
			newf = (char **) realloc(field,
						maxfield * sizeof(field[0]));
			if (newf == NULL)
				return NOMEM;
			field = newf;
		}
		/* comprime os caracteres delimitadores subsequentes */
		if (compress) {
		    while (*p != '\0' && *p == delim) {
		        p++;
		    }
		}
		if (*p == '"')
			sepp = advquoted(++p, delim);	/* pula o inicial */
		else
			sepp = p + strcspn(p, &delim);
		sepc = sepp[0];
		sepp[0] = '\0';				/* termina o campo */
		field[nfield++] = p;
		p = sepp + 1;
	} while (sepc == (int) delim);

	return nfield;
}

/**
 * @brief Retorna ponteiro para o próximo separador.
 * 
 * @param p ponteiro para liha
 * @param delim delimitador
 * @return char* string do próximo separador
 */
static char *advquoted(char *p, char delim)
{
	int i, j;

	for (i = j = 0; p[j] != '\0'; i++, j++) {
		if (p[j] == '"' && p[++j] != '"') {
			/* copia o próximo separador */
			int k = strcspn(p+j, &delim);
			memmove(p+i, p+j, k);
			i += k;
			j += k;
			break;
		}
		p[i] = p[j];
	}
	p[i] = '\0';
	return p + j;
}

/**
 * @brief Retorna ponteiro para o valor do n-ésimo campo da linha.
 * 
 * Retorna ponteiro para o valor do n-ésimo campo da linha.
 * 
 * @param n index do campo
 * @return char* string do campo
 */
char *csvfield(int n)
{
	if (n < 0 || n >= nfield)
		return NULL;
	return field[n];
}

/**
 * @brief Obtém o número de campos do arquivo.
 * 
 * Obtém o número de campos do arquivo.
 * 
 * @return int número de campos do arquivo
 */
int csvnfield(void)
{
	return nfield;
}
