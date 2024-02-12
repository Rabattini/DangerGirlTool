#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <locale.h>
#include <direct.h>
#include <windows.h>
#include <fstream>
#include <map>
#include <string>
#include <sstream>

using namespace std;


#define EXIT(text) { printf(text); exit(EXIT_FAILURE); }


//void Escolhe_bloco(int seletor);
long contador(char* nome, unsigned int inicio, unsigned int Offset_fim);
void calcula_ponteiro(char* nome, unsigned int Offset_ini, unsigned int Offset_fim, unsigned int InicioPonteiro);
void Dumper(char* nome);
void Inserter(char* nome_trad);
void  Titulo(void);
void  ComoUsar(void);


/*----------------------------------------------------------------------------*/
int main(int argc, char** argv) {

	int opc;

	//setlocale(LC_ALL, "Portuguese");
	UINT CPAGE_UTF8 = 65001;
	SetConsoleOutputCP(CPAGE_UTF8);
	_mkdir("scripts_originais");
	_mkdir("scripts_traduzidos");

	Titulo();

	if (argc != 3) ComoUsar();

	if (!_strcmpi(argv[1], "-e")) Dumper(argv[2]);
	else if (!_strcmpi(argv[1], "-i")) Inserter(argv[2]);
	else                              EXIT("Parametro desconhecido \n");

	printf("\nRip...Feito\n");

	exit(EXIT_SUCCESS);
}

/*----------------------------------------------------------------------------*/
void Titulo(void) {
	printf(
		"\n"
		"DUMP - Copyright (C) 2024 Luke\n"
		"Script dumper para 'Danger Girl - PSX' (USA)\n"
		"\n"
	);
}

/*----------------------------------------------------------------------------*/
void ComoUsar(void) {
	EXIT(
		"Como Usar: DUMP -e|-i Nome do arquivo.msg\n"
		"\n"
		"-e ... extrair 'nome do arquivo.msg' irá criar duas pastas - scripts_originais - scripts_traduzidos\n"
		"-i ... insert 'nome do arquivo.msg' não esqueça de colocar o .txt traduzido dentro da pasta 'scripts_traduzidos' \n"
	);
}

/*----------------------------------------------------------------------------*/

void Dumper(char* nome) {
	
	map<int, string> tabEntry;
	map<int, string>::iterator it;

	//tabela hardcoded, já coloca em hex direto
	tabEntry.insert(pair<int, string>(0x10, "a"));
	tabEntry.insert(pair<int, string>(0x11, "b"));
	tabEntry.insert(pair<int, string>(0x12, "c"));
	tabEntry.insert(pair<int, string>(0x13, "d"));

	//tabela do arquivo, precisa ler e converter para hex
	ifstream fp("tabela.tbl");
	string line;
	string valHex, valChar;
	int hexValue;

	//apaga as letras hardcoded de antes
	tabEntry.clear();

	if (fp.is_open()) {

		while (getline(fp, line)) {

			stringstream ss(line);

			if (!getline(ss, valHex, '=') || !getline(ss, valChar)) {
				cout << "tabela com erro!" << endl;
				return;
			}

			stringstream sshex;
			sshex << hex << valHex;
			sshex >> hexValue;

			tabEntry.insert(pair<int, string>(hexValue, valChar));

		}
		fp.close();
	}
	else {
		cout << "Erro ao ler arquivo!" << endl;
		return;
	}
	
	
	FILE* arquivo, * arquivo_saida;
	unsigned char* memoria;
	char saida[50];
	char caminho_orig[50] = "scripts_originais\\";
	int cont = 0;
	int contador = 0;
	long tamanho, entries, table;
	short tamanho_corrigido;
	unsigned int offset_ini, i, end_ponteiro;
	unsigned int magic_number;

	arquivo = fopen(nome, "rb");

	fseek(arquivo, 0, SEEK_END);
	tamanho = ftell(arquivo);

	fseek(arquivo, SEEK_SET + 0x08, SEEK_SET);
	fread(&magic_number, sizeof(unsigned int), 1, arquivo);

	if (magic_number == 0x4556454c) {
		fseek(arquivo, SEEK_SET + 0x20, SEEK_SET);
		fread(&entries, sizeof(long), 1, arquivo);

		fseek(arquivo, SEEK_SET + 0x24, SEEK_SET);
		fread(&table, sizeof(long), 1, arquivo);
	}
	else {
		fseek(arquivo, SEEK_SET + 0x04, SEEK_SET);
		fread(&entries, sizeof(long), 1, arquivo);
		
		fseek(arquivo, SEEK_SET + 0x08, SEEK_SET);
		fread(&table, sizeof(long), 1, arquivo);
	
	}

	fseek(arquivo, -2, SEEK_END);
	fread(&tamanho_corrigido, sizeof(short), 2, arquivo);

	if (tamanho_corrigido == 0x0000) {
		offset_ini = tamanho - table - 2;
	}
	else {
		offset_ini = tamanho - table;
	}

	printf("Tamanho: %.8X\n", tamanho);
	printf("Entries: %d\n", entries);
	printf("Table: %d\n", table);
	printf("Offset_ini: %X", offset_ini);
	_getch();

	fseek(arquivo, 0, SEEK_SET);
	memoria = (unsigned char*)malloc(sizeof(unsigned char) * tamanho);
	cont = fread(memoria, sizeof(unsigned char), tamanho, arquivo);
	fclose(arquivo);

	strcpy(saida, nome);
	strcat(saida, ".txt");
	strcat(caminho_orig, saida);

	arquivo_saida = fopen(caminho_orig, "wt");

	for (i = offset_ini; i <= tamanho; i++) {
		it = tabEntry.find(memoria[i] & 0xFF); // Certifique-se de lidar com bytes negativos corretamente
		if (it != tabEntry.end()) {
			fprintf(arquivo_saida, "%s", it->second.c_str());
		}
		else if (memoria[i] == 0x00) {
			fprintf(arquivo_saida, "{%.2x}\n------------\n", memoria[i]);
		}
		else {
			// Se não encontrar na tabela, use o valor hexadecimal diretamente
			fprintf(arquivo_saida, "{%.2x}", memoria[i]);
		}
	}

	printf("\nScript Dumpado com êxito!\n");
	_getch();

	fclose(arquivo_saida);
	free(memoria);
}

void Inserter(char* nome_trad) {

	map<string, int> tabEntry;
	map<string, int>::iterator it;

	//tabela hardcoded, já coloca em hex direto
	tabEntry.insert(pair<string, int>("a", 0x10));
	tabEntry.insert(pair<string, int>("b", 0x11));
	tabEntry.insert(pair<string, int>("c", 0x12));
	tabEntry.insert(pair<string, int>("d", 0x13));

	//tabela do arquivo, precisa ler e converter para hex
	ifstream fp("tabela.tbl");
	string line;
	string valHex, valChar;
	int hexValue;

	//apaga as letras hardcoded de antes
	tabEntry.clear();

	if (fp.is_open()) {

		while (getline(fp, line)) {

			stringstream ss(line);

			if (!getline(ss, valHex, '=') || !getline(ss, valChar)) {
				cout << "tabela com erro!" << endl;

			}

			stringstream sshex;
			sshex << hex << valHex;
			sshex >> hexValue;

			tabEntry.insert(pair<string, int>(valChar, hexValue));

		}
		fp.close();
	}
	else {
		cout << "Erro ao ler arquivo!" << endl;

	}



	FILE* arq, * out;
	char s[100];
	int cont = 0;
	long cont_letras, char_cont_ant, soma, tamanho, table;
	short tamanho_corrigido;
	unsigned int c;
	unsigned int Offset_ini;
	unsigned int Offset_fim;
	unsigned int ponteiro_mod, ponteiro_ant;
	unsigned int ponteiro_orig;
	int i;
	unsigned int magic_number;
	char caminho_orig[50] = "scripts_originais\\";
	char caminho_trad[50] = "scripts_traduzidos\\";

	strcat(caminho_orig, nome_trad);
	strcat(caminho_trad, nome_trad);

	strcat(caminho_orig, ".txt");
	strcat(caminho_trad, ".txt");

	printf("Caminho original: %s\n\n", caminho_orig);
	printf("Caminho traduzido: %s\n\n", caminho_trad);

	if ((arq = fopen(caminho_trad, "r")) == NULL) {
		printf("Erro na abertura do arquivo modificado!\n\n");
		exit(0);
	}
	if ((out = fopen(nome_trad, "r+b")) == NULL) {
		printf("Erro na criação do arquivo de saída!\n");
		exit(0);
	}

	fseek(out, 0, SEEK_END);
	tamanho = ftell(out);

	fseek(out, SEEK_SET + 0x08, SEEK_SET);
	fread(&magic_number, sizeof(unsigned int), 1, out);

	if (magic_number == 0x4556454C) {
		fseek(out, SEEK_SET + 0x24, SEEK_SET);
		fread(&table, sizeof(long), 1, out);
		
	}
	else {
		fseek(out, SEEK_SET + 0x08, SEEK_SET);
		fread(&table, sizeof(long), 1, out);
	}

	fseek(out, -2, SEEK_END);
	fread(&tamanho_corrigido, sizeof(short), 2, out);

	if (tamanho_corrigido == 0x0000) {
		Offset_ini = tamanho - table - 2;
	}
	else {
		Offset_ini = tamanho - table;
	}

	fseek(out, Offset_ini, SEEK_SET);
	printf("\nInserindo...\n");

	while (fgets(s, 100, arq) != NULL) {
		if (!strcmp(s, "------------\n") || !strcmp(s, "------------")) {
			continue;
		}
		for (i = 0; i < (int)strlen(s) - 1; i++) {
			if (s[i] == '{') {
				if (s[i + 3] == '}') {
					sscanf(&s[i + 1], "%xx", &c);
					//fwrite(&tabEntry[string(1, c)], sizeof(unsigned char), 1, out);
					fwrite(&c, sizeof(unsigned char), 1, out);
					i = i + 3;
				}
			}
			else {
				c = s[i];
				fwrite(&tabEntry[string(1, c)], sizeof(unsigned char), 1, out);
				//fwrite(&c, sizeof(unsigned char), 1, out);
			}

		}
	}

	Offset_fim = ftell(out);

	Offset_fim = Offset_fim - 1;

	fclose(arq);
	fclose(out);

	// Escolher o valor apropriado para o terceiro argumento de calcula_ponteiro
	if (magic_number == 0x4556454C) {
		calcula_ponteiro(nome_trad, Offset_ini, Offset_fim, 0x2C);
	}
	else {
		calcula_ponteiro(nome_trad, Offset_ini, Offset_fim, 0x10);
	}

	printf("Script Inserido com sucesso!\nPressione qualquer tecla para continuar.");
	_getch();
}

void calcula_ponteiro(char* nome, unsigned int Offset_ini, unsigned int Offset_fim, unsigned int InicioPonteiro) {

	FILE* arquivo_mod;
	unsigned char* memoria_mod;
	unsigned char PonteiroByte1, PonteiroByte2, PonteiroByte3, PonteiroByte4;
	unsigned int i;
	unsigned int k = 0;
	int cont;
	unsigned int cont_letras = 0x00;
	unsigned int soma;
	unsigned int char_cont_ant = 0x00;
	long char_cont;
	long calculo_ponteiro1, calculo_ponteiro2;
	long table;

	arquivo_mod = fopen(nome, "r+b");

	fseek(arquivo_mod, 0, SEEK_SET);

	memoria_mod = (unsigned char*)malloc(sizeof(unsigned char) * Offset_fim);
	cont = fread(memoria_mod, sizeof(unsigned char), Offset_fim, arquivo_mod);
	k = InicioPonteiro;

	for (i = Offset_ini; k < Offset_ini; i++) {

		if (k == 0x2C) {
			soma = 0;
			cont_letras = cont_letras = contador(nome, i, Offset_fim);
			cont_letras = cont_letras + 1;

			fseek(arquivo_mod, SEEK_SET + k, SEEK_SET);

			calculo_ponteiro1 = soma;
			calculo_ponteiro2 = cont_letras;

			PonteiroByte1 = (unsigned char)(calculo_ponteiro1 & 0x000000FF);
			PonteiroByte2 = (unsigned char)((calculo_ponteiro1 >> 8) & 0x000000FF);

			PonteiroByte3 = (unsigned char)(calculo_ponteiro2 & 0x000000FF);
			PonteiroByte4 = (unsigned char)((calculo_ponteiro2 >> 8) & 0x000000FF);

			fwrite(&PonteiroByte1, sizeof(unsigned char), 1, arquivo_mod);
			fwrite(&PonteiroByte2, sizeof(unsigned char), 1, arquivo_mod);

			fwrite(&PonteiroByte3, sizeof(unsigned char), 1, arquivo_mod);
			fwrite(&PonteiroByte4, sizeof(unsigned char), 1, arquivo_mod);

			k = k + 8;
		}
		if (k == 0x10) {
			soma = 0;
			cont_letras = cont_letras = contador(nome, i, Offset_fim);
			cont_letras = cont_letras + 1;

			fseek(arquivo_mod, SEEK_SET + k, SEEK_SET);

			calculo_ponteiro1 = soma;
			calculo_ponteiro2 = cont_letras;

			PonteiroByte1 = (unsigned char)(calculo_ponteiro1 & 0x000000FF);
			PonteiroByte2 = (unsigned char)((calculo_ponteiro1 >> 8) & 0x000000FF);

			PonteiroByte3 = (unsigned char)(calculo_ponteiro2 & 0x000000FF);
			PonteiroByte4 = (unsigned char)((calculo_ponteiro2 >> 8) & 0x000000FF);

			fwrite(&PonteiroByte1, sizeof(unsigned char), 1, arquivo_mod);
			fwrite(&PonteiroByte2, sizeof(unsigned char), 1, arquivo_mod);

			fwrite(&PonteiroByte3, sizeof(unsigned char), 1, arquivo_mod);
			fwrite(&PonteiroByte4, sizeof(unsigned char), 1, arquivo_mod);

			k = k + 8;
		}

		if (memoria_mod[i - 1] == 0x00) {

			//Para obter o ponteiro
			cont_letras = contador(nome, i, Offset_fim);
			cont_letras = cont_letras + 1;

			soma = char_cont_ant + cont_letras;

			//Para obter o próximo char_cont
			cont_letras = contador(nome, i + cont_letras, Offset_fim);
			cont_letras = cont_letras + 1;

			fseek(arquivo_mod, SEEK_SET + k, SEEK_SET);

			calculo_ponteiro1 = soma;
			calculo_ponteiro2 = cont_letras;

			PonteiroByte1 = (unsigned char)(calculo_ponteiro1 & 0x000000FF);
			PonteiroByte2 = (unsigned char)((calculo_ponteiro1 >> 8) & 0x000000FF);

			PonteiroByte3 = (unsigned char)(calculo_ponteiro2 & 0x000000FF);
			PonteiroByte4 = (unsigned char)((calculo_ponteiro2 >> 8) & 0x000000FF);

			fwrite(&PonteiroByte1, sizeof(unsigned char), 1, arquivo_mod);
			fwrite(&PonteiroByte2, sizeof(unsigned char), 1, arquivo_mod);

			fwrite(&PonteiroByte3, sizeof(unsigned char), 1, arquivo_mod);
			fwrite(&PonteiroByte4, sizeof(unsigned char), 1, arquivo_mod);

			char_cont_ant = soma;
			cont_letras = 0;

			k = k + 8;
		}
	}
	fclose(arquivo_mod);
	free(memoria_mod);
}

long contador(char* nome, unsigned int inicio, unsigned int Offset_fim) {
	FILE* arquivo_cont;
	int cont;
	unsigned char* memoria_cont;
	int cont_mod = 0;
	unsigned int i;
	long contagem = 0;

	arquivo_cont = fopen(nome, "r+b");
	fseek(arquivo_cont, 0, SEEK_SET);

	memoria_cont = (unsigned char*)malloc(sizeof(unsigned char) * Offset_fim);
	cont = fread(memoria_cont, sizeof(unsigned char), Offset_fim, arquivo_cont);

	for (i = inicio; memoria_cont[i] != 0x00; i++) {
		contagem++;
	}

	fclose(arquivo_cont);
	free(memoria_cont);
	return contagem;



}

