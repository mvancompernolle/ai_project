/***********************************************************************************
*																				   *
*                         -=NetAgenda=-  VERSAO 0.04                               *
*                                                                                  *
*																				   *
*      Desenvolvido por Roberto Mello Pereira Filho (REPOLHO) <repolho@dsgx.org>   *
*																				   *
************************************************************************************
*																				   *
*                                                                                  *
*    This program is free software; you can redistribute it and/or modify          *
*    it under the terms of the GNU General Public License as published by          *
*    the Free Software Foundation; either version 2 of the License, or             *
*    (at your option) any later version.                                           *
*                                                                                  *
*    This program is distributed in the hope that it will be useful,               *
*    but WITHOUT ANY WARRANTY; without even the implied warranty of                *
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 *
*    GNU General Public License for more details.                                  *
*																				   *
*    You should have received a copy of the GNU General Public License             *
*    along with this program; if not, write to the Free Software                   *
*    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA     *
*																				   *
*                                                                                  *
************************************************************************************
*  Copyright 2003 Roberto Mello Pereira Filho                                      *
***********************************************************************************/






#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define VERSION 0.04

/****Declaracao de estrutura***/
//futuramente ira para um arquivo soh com a struct

	struct stru {
		int id;
		char stat;
		char nome[100];
		char apelido[100];
		char tel[15]; 
		char cel[15];
		char end[255];
		char email[100];
		char icq[15];
		char aim[15];
		char yahoo[15];
				} r[100];

//prototipo das funcoes
void cabec(void);
void tela_saida(void);
void menu(void);
void adicionar(void);
void inserir(int x);
void simples(void);
void completo(void);
void file_read(void);
void file_write(void);
void inic_stru(void);
void buscar(int cont);
void menu_remover(int cont);
void remover(int x);

//Declaracao global de variaveis

FILE *file_in, *file_out;
struct stru reg[100];
int count=0;

	
//funcao principal
void main(int argc, char *argv[])
{

	inic_stru();
	file_read();
	menu();



}


//funcao que define o MENU principal (usando recursividade)
void menu(void)
{

	char opc;
	
	
	system("cls");
	cabec();
	printf("|                                                                   MENU   |\n");  
	printf(" --------------------------------------------------------------------------\n");
    
	
	puts("");
	puts("");
	puts("");
    printf("[A]dicionar\n");
	printf("[E]ditar\n");
	printf("[R]emover\n");
	printf("[B]uscar\n");
	printf("[S]air\n");
	printf(">> ");
	
	scanf("%c",&opc);
	
	//teste para digitar valor certo
	if(opc!='a' && opc!='e' && opc!='r' && opc!='b' && opc!='s' )
		menu();
	//escolha da opcao
	switch(opc)
	{
	case 'a': adicionar();
		break;
	case 'e': //editar();
		break;
	case 'r': menu_remover(count);
		break;
	case 'b': buscar(count);
		break;
	case 's':
		tela_saida();
		exit(1);
	}



}

//funcao que mostra MEIO cabecalho
//********************************ARRUMAR**********************************
void cabec(void)
{
		printf(" --------------------------------------------------------------------------\n");
		printf("|              Agenda de Contatos *NETAGENDA* Versao %.2f                  |\n",VERSION); 
		printf("|             Desenvolvido por REPOLHO <repolho@dsgx.org>                  |\n");
		printf(" --------------------------------------------------------------------------\n");
		

}

//funcao que mostra a tela de saida
void tela_saida(void)
{

	file_write();

	system("cls");
	puts("");
	printf("==========================================================================\n");
	puts("");
	puts("");
	puts("");
	puts("");
	puts("");
	puts("");
	printf("         OBRIGADO POR UTILIZAR O *NetAgenda* Versao %.2f\n",VERSION);
	printf("           Desenvolvido por REPOLHO <repolho@dsgx.org>                   \n");
	puts("");
	puts("");
	puts("");
	puts("");
	puts("");
	puts("");
	printf("                                      Digite qualquer tecla para sair\n ");
	printf("==========================================================================\n");
	
	getch();



}

void adicionar(void)
{

	char opc;

	system("cls");
	cabec();
	printf("|                                                                ADICIONAR |\n");  
	printf(" --------------------------------------------------------------------------\n");
	puts("");
	printf("Escolha o tipo de contato: \n");
	printf("[S]imples\n");
	printf("[C]ompleto\n");
	printf(">> ");

	scanf("%c",&opc);

	if(opc!='s' && opc!='c' )
		adicionar();

	switch(opc)
	{
	case 'c':
		inserir(2);
		break;
	case 's':
		inserir(1);
		break;
	}
/*** 2 para insercao completa e 1 para insercao simples fazer swtich na funcao inserir ***/

}

void inserir(int x)
{

	switch(x)
	{
	case 1: 
		simples();
		break;
	case 2:
		completo();
			break;
	}

}


void simples(void)
{

	int i;
	char simnao='n';

	system("cls");
	cabec();
	printf("|                                                        ADICIONAR-SIMPLES |\n");  
	printf(" --------------------------------------------------------------------------\n");
	puts("");

	

	for(i=0;i<100;i++)
	{
	
		if(r[i].stat=='*')
			{
		
			printf("TESTE APENAS o i VALE = %d\n",i);
			printf("Nome: ");
			fflush(stdin);
			gets(r[i].nome);
			printf("Apelido: ");
			gets(r[i].apelido);
			printf("Telefone: ");
			gets(r[i].tel);
			printf("Celular: ");
			gets(r[i].cel);
			printf("ICQ: ");
			gets(r[i].icq);
			r[i].stat='x';
			printf("Deseja adicionar mais um? (s/n): ");
		    scanf("%c",&simnao);
			if(simnao == 's')
			{
				system("cls");
				cabec();
				printf("|                                                        ADICIONAR-SIMPLES |\n");  
				printf(" --------------------------------------------------------------------------\n");
				puts("");
			}
			else 
			{
				file_write();
				menu();
			}
			
		}
		
	}



}

void completo(void)
{

		int i;
	char simnao='n';

	system("cls");
	cabec();
	printf("|                                                       ADICIONAR-COMPLETO |\n");  
	printf(" --------------------------------------------------------------------------\n");
	puts("");

	

	for(i=0;i<100;i++)
	{
	
		if(r[i].stat=='*')
			{
		
			printf("TESTE APENAS o i VALE = %d\n",i);
			printf("Nome: ");
			fflush(stdin);
			gets(r[i].nome);
			printf("Apelido: ");
			gets(r[i].apelido);
			printf("Telefone: ");
			gets(r[i].tel);
			printf("Celular: ");
			gets(r[i].cel);
			printf("Endereco: ");
			gets(r[i].end);
			printf("E-mail: ");
			gets(r[i].email);
			printf("ICQ: ");
			gets(r[i].icq);
			printf("AIM :");
			gets(r[i].aim);
			printf("Yahoo Messager: ");
			gets(r[i].yahoo);
			r[i].stat='x';
			printf("Deseja adicionar mais um? (s/n): ");
		    scanf("%c",&simnao);
			if(simnao == 's')
			{
				system("cls");
				cabec();
				printf("|                                                        ADICIONAR-SIMPLES |\n");  
				printf(" --------------------------------------------------------------------------\n");
				puts("");
			}
			else 
			{
				menu();
				file_write();
			}
		}
		
	}
}

void file_read(void)
{

	//char opc;

	if((file_out=fopen("teste.txt","rb+"))==NULL)
	{
		//***********OPCIONAL PENSAR DEPOIS EM DEIXAR OU NAO******/
		//printf("Nao Existe o arquivo deseja cria-lo? (s/n) ");
		//scanf("%c",&opc);
		
		if((file_out=fopen("teste.txt","wb+"))==NULL)
		{
			printf("erro na criacao do arquivo!\n");
			getch();
			exit(1);
		}
		
	}
	else
	{//else
	
		if((fread(&r,sizeof(struct stru),100,file_out))==NULL)
			{
			printf("Nao Leu o arquivo\n");
			getch();
			exit(1);
			}
	}//else
	

}

void file_write(void)
{

	if((file_out=fopen("teste.txt","wb+"))==NULL)
	{
		printf("Nao Abriu o arquivo\n");
		getch();
		exit(1);
	}

	if((fwrite(&r,sizeof(struct stru),100,file_out))==NULL)
		{
		printf("Nao Escreve no arquivo.\n");
		getch();
		exit(1);
	}


}


void inic_stru(void)
{

	int i;

	for(i=0;i<100;i++)
	{
		r[i].stat = '*';
		r[i].id = (i+1);
	}

}

void buscar(int cont)
{

	int i, result[100];
	char busca[100], opc;

	
	system("cls");
	cabec();
	printf("|                                                         BUSCAR REGISTROS |\n");  
	printf(" --------------------------------------------------------------------------\n");
	puts("");



	printf("Digite o nome completo: ");
	fflush(stdin);
	gets(busca);
	

	for(i=0;i<100;i++)
	{
				
		if((strcmp(r[i].nome,busca)==0))
		{
			result[i] = 1;
			cont++;
			
		} 
		else
		{
			result[i] = 0;
			cont = cont;
		}
	}
	

		
		
		if(cont == 0)
		{
			
			
			system("cls");
			cabec();
			printf("|                                                         BUSCAR REGISTROS |\n");  
			printf(" --------------------------------------------------------------------------\n");
			puts("");
            
			//implementar sistema (tentar de novo)//
			printf("Nao teve nenhum resultado, tentar de novo?");
			getch();
			menu();
		}
		else
		{
			for(i=0;i<100;i++)
			{
				if(result[i]!=0)
				{
					system("cls");
					cabec();
					printf("|                                                         BUSCAR REGISTROS |\n");  
					printf(" --------------------------------------------------------------------------\n");
					puts("");

					printf("Numero: %i\n",r[i].id);
					printf("Nome: %s\n",r[i].nome);
					printf("Apelido: %s\n",r[i].apelido);
					printf("Telefone: %s\n",r[i].tel);
					printf("Celular: %s\n",r[i].cel);
					printf("Endereco: %s\n",r[i].end);
					printf("E-mail: %s\n",r[i].email);
					printf("ICQ: %s\n",r[i].icq);
					printf("AIM :%s\n",r[i].aim);
					printf("Yahoo Messager: %s\n",r[i].yahoo);
					if(cont > 1)
					{
						puts("");
						printf("Aperte B para voltar ao menu!! ");
						scanf("%c",&opc);
						
						if(opc=='b')
						{
							menu();
						}
					
					}
					else
					{
						puts("");
						printf("Aperte qualquer tecla para sair!!\n");
						getch();
					}

				}
				
			}
		
			menu();
		}

	  
		
	

}



void menu_remover(int cont)
{//abre menu_remover

	int i, result[100];
	char busca[100], simnao;

	
	system("cls");
	cabec();
	printf("|                                                         BUSCAR REGISTROS |\n");  
	printf(" --------------------------------------------------------------------------\n");
	puts("");




	printf("Digite o nome completo: ");
	fflush(stdin);
	gets(busca);
	

	for(i=0;i<100;i++)
	{
				
		if((strcmp(r[i].nome,busca)==0))
		{
			result[i] = 1;
			cont++;
			
		} 
		else
		{
			result[i] = 0;
			cont = cont;
		}
	}
	

		
		
		if(cont == 0)
		{
			
			
			system("cls");
			cabec();
			printf("|                                                         BUSCAR REGISTROS |\n");  
			printf(" --------------------------------------------------------------------------\n");
			puts("");
            
			//implementar sistema (tentar de novo)//
			printf("Nao teve nenhum resultado, tentar de novo?");
			getch();
			menu();
		}
		else
		{
			for(i=0;i<100;i++)
			{
				if(result[i]!=0)
				{
					system("cls");
					cabec();
					printf("|                                                         BUSCAR REGISTROS |\n");  
					printf(" --------------------------------------------------------------------------\n");
					puts("");

					printf("Numero: %i\n",r[i].id);
					printf("Nome: %s\n",r[i].nome);
					printf("Apelido: %s\n",r[i].apelido);
					printf("Telefone: %s\n",r[i].tel);
					printf("Celular: %s\n",r[i].cel);
					printf("Endereco: %s\n",r[i].end);
					printf("E-mail: %s\n",r[i].email);
					printf("ICQ: %s\n",r[i].icq);
					printf("AIM :%s\n",r[i].aim);
					printf("Yahoo Messager: %s\n",r[i].yahoo);
					puts("");
					printf("Deseja apagar este cadastro (s/n) ??");
					fflush(stdin);
					scanf("%c",&simnao);
					
					if(simnao=='s')
						remover(i);
					


					/*switch(simnao)
					{
					case 's': remover(i);
						break;
					default: menu();
					}*/



					//	getch();
				
				}
				
			}
		
			menu();
		}
}// fecha menu_remover


void remover(int x)
{
	r[x].stat='*';
	r[x].nome[0]=' ';
	//**********OU VOLTA PRO MENU OU VOLTA PRA REMOVER***********//
	//menu();
	//menu_remover(0);
}

