#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h> // toupper() func
#include <pthread.h>
#include <ncurses.h>
#include <termios.h>
#include <unistd.h>

// https://gist.github.com/RabaDabaDoba/145049536f815903c79944599c6f952a
// Regular text
#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"

// Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define BWHT "\e[1;37m"

// Regular underline text
#define UBLK "\e[4;30m"
#define URED "\e[4;31m"
#define UGRN "\e[4;32m"
#define UYEL "\e[4;33m"
#define UBLU "\e[4;34m"
#define UMAG "\e[4;35m"
#define UCYN "\e[4;36m"
#define UWHT "\e[4;37m"

// Regular background
#define BLKB "\e[40m"
#define REDB "\e[41m"
#define GRNB "\e[42m"
#define YELB "\e[43m"
#define BLUB "\e[44m"
#define MAGB "\e[45m"
#define CYNB "\e[46m"
#define WHTB "\e[47m"

// High intensty background
#define BLKHB "\e[0;100m"
#define REDHB "\e[0;101m"
#define GRNHB "\e[0;102m"
#define YELHB "\e[0;103m"
#define BLUHB "\e[0;104m"
#define MAGHB "\e[0;105m"
#define CYNHB "\e[0;106m"
#define WHTHB "\e[0;107m"

// High intensty text
#define HBLK "\e[0;90m"
#define HRED "\e[0;91m"
#define HGRN "\e[0;92m"
#define HYEL "\e[0;93m"
#define HBLU "\e[0;94m"
#define HMAG "\e[0;95m"
#define HCYN "\e[0;96m"
#define HWHT "\e[0;97m"

// Bold high intensity text
#define BHBLK "\e[1;90m"
#define BHRED "\e[1;91m"
#define BHGRN "\e[1;92m"
#define BHYEL "\e[1;93m"
#define BHBLU "\e[1;94m"
#define BHMAG "\e[1;95m"
#define BHCYN "\e[1;96m"
#define BHWHT "\e[1;97m"

// Reset
#define reset "\e[0m"
#define CRESET "\e[0m"
#define COLOR_RESET "\e[0m"

char jogando; // setar o loop nos modos de jogo
int seconds = 0; //contar tempo de rodada
int mult_dificuldade = 1; //multiplicador de dificuldade
int dica = 0; //flag para indicar a presença de dica
int ajuda = 0; //flag para indicar a presença de revelacao de letras

struct termios info;

//declaracao de funcoes
void main_menu();
void forca(int estado);
void multiplayer();
void singleplayer();
void limpa_tela();
void ver_ranking();
void ver_ajuda();
void exibeAjuda();
void mudarLeitura();
void restaurarLeitura();

//referencia: https://stackoverflow.com/questions/9033060/c-function-to-insert-text-at-particular-location-in-file-without-over-writing-th
/*The best way to do that kind of thing is to open your file for reading,
open a new file for writing, copy the part of the file before the insertion point,
insert the data, copy the rest, and then move the new file over the old one.*/

//escreve no arquivo de pontuação
void write_highscore(int score, char *name, char *maisInfo)
{
  FILE *fr;
  FILE *fw;

  // copiar o new_ranking para o ranking.txt (atualiza-lo)
  fw = fopen("ranking.txt", "w");
  fr = fopen("new_ranking.txt", "r");

  if (fw == NULL)
  {
    printf("Erro ao abrir arquivo para escrita\n");
    return;
  }

  if (fr == NULL)
  {
    printf("Erro ao abrir arquivo para leitura\n");
    return;
  }

  int id; //id do new_ranking
  int score_id; //score do new_ranking
  char resto[255];
  int qtd_linhas = -1; //-1 para descartar a ultima linha q é um \n
  
  //conta a qtd de linhas do arquivo new_ranking
  while (!feof(fr))
  {
    fscanf(fr, "%d_%d_%s", &id, &score_id, resto);
    qtd_linhas += 1;
  }

  fseek(fr, 0, SEEK_SET); // vai pro inicio do arq new_ranking.txt

  //copia new_ranking para ranking.txt
  for (int i = 0; i < qtd_linhas; i++)
  {
    fscanf(fr, "%d_%d_%s", &id, &score_id, resto);
    fprintf(fw, "%d_%d_%s\n", id, score_id, resto);
  }

  fclose(fw);
  fclose(fr);

  // descobrir em qual linha q vai ter q inserir
  fr = fopen("ranking.txt", "r");

  if (fr == NULL)
  {
    printf("Erro ao abrir arquivo para leitura\n");
    return;
  }

  int line;
  int flag_ultimo = 1; // saber se vai ser inserido na ultima linha
  int flag_primeiro = 0; //saber se é a primeira linha do arquivo
  int score_file; //score pego do arquivo para comparar com o do jogador atual
  
  while (!feof(fr))
  {
    fscanf(fr, "%d_%d_%s", &line, &score_file, resto);

    // printf("%s\n", resto);

    if (score > score_file)
    {
      // vai ser nessa linha a insercao da pontuacao do jogador
      flag_ultimo = 0;
      break;
    }
  }

  // verifica se é o primeiro input do arquivo
  fseek(fr, 0, SEEK_END);
  int size;
  size = ftell(fr);
  if (0 == size)
  {
    flag_primeiro = 1;
    line = 1;
  }

  if (flag_ultimo)
  {
    line++;
  }

  // printf("sua posicao vai ser a %d\n", line);

  fclose(fr);

  // printf("o score da pessoa vai ser inserido na posicao %d\n\n", line);
  // getchar();

  // abrir novo arquivo e escrever nele conforme o metodo adotado
  fr = fopen("ranking.txt", "r");

  if (fr == NULL)
  {
    printf("Erro ao abrir arquivo para leitura\n");
    return;
  }

  //verifica se é pra escrever na ultima posicao
  if (flag_ultimo)
  {
    fw = fopen("new_ranking.txt", "a");

    if (fw == NULL)
    {
      printf("Erro ao abrir arquivo para escrita\n");
      return;
    }

    fprintf(fw, "%d_%d_%s_%s\n", line, score, name, maisInfo);
  }
  else if (flag_primeiro) //verifica se eh a primeira linha do arquivo vazio
  {

    fw = fopen("new_ranking.txt", "w");

    if (fw == NULL)
    {
      printf("Erro ao abrir arquivo para escrita\n");
      return;
    }

    fprintf(fw, "%d_%d_%s_%s\n", line, score, name, maisInfo);

  }
  else
  {
    //se a linha de insercao for no meio do arquivo
    
    fw = fopen("new_ranking.txt", "w");

    if (fw == NULL)
    {
      printf("Erro ao abrir arquivo para escrita\n");
      return;
    }

    for (int i = 0; i < qtd_linhas; i++)
    {
      fscanf(fr, "%d_%d_%s", &id, &score_id, resto);

      if (id < line)
      {
        // ainda nao chegou na posicao de insercao; copia a linha
        fprintf(fw, "%d_%d_%s\n", id, score_id, resto);
      }
      else if (line == id)
      {
        // linha onde vai inserir o novo score
        // printf("entrou aqui!!!!\n");
        fprintf(fw, "%d_%d_%s_%s\n", line, score, name, maisInfo);
        fprintf(fw, "%d_%d_%s\n", (id + 1), score_id, resto);
        
      }
      else
      {
        // copia o resto das linhas acertando o id
        fprintf(fw, "%d_%d_%s\n", (id + 1), score_id, resto);
        id++;
      }
    }
  }

  fclose(fw);
  fclose(fr);
}

//menu tela de ajuda [4]
void exibeAjuda(){
  limpa_tela();
  printf(BHRED "\t\t[AJUDA]\n\n" reset);

  printf(HRED"MODO MULTIPLAYER: \n\n"reset);
  printf("   Nesse modo de jogo você e pelo menos mais um amigo vão participar.\n\n");
  printf("   Um dos jogadores vai escolher uma palavra para o outro tentar adivinhar.\n\n");
  printf("   O jogador que está tentando adivinhar pode inserir uma letra que acha pertencer\n   à palavra ou o comando /chut para tentar acertar a palavra completa.\n\n");
  printf("   O jogo acaba quando o jogador adivinhar a palavra ou errar 6 vezes.\n\n");

  printf(HRED"\nMODO SINGLEPLAYER: \n\n"reset);
  printf("   Nesse modo de jogo você jogará sozinho e poderá escolher a dificuldade do jogo.\n\n");
  printf("   As dificuldades possuem as seguintes características:\n\n");
  printf("   Fácil: Presença de Dica temática das palavras e de Dica por letras.\n");
  printf("   Médio: Presença de Dica temática das palavras.\n");
  printf("   Difícil: Sem dicas.\n\n");
  printf("   Para cada rodada do jogo o jogador terá que adivinhar uma palavra diferente sorteada aleatoriamente de um arquivo.\n\n");
  printf("   O jogador que está tentando adivinhar pode inserir uma letra que acha pertencer\n   à palavra ou o comando /chut para tentar acertar a palavra completa.\n\n");  
  printf("   Se ele tiver acesso a Dica por letras, ele poderá fazer uso do comando /dica para revelar alguma letra da palavra secreta.\n\n");
  printf("   O jogo somente acaba quando o jogador errar 6 vezes.\n\n");
  printf("   Após o fim do jogo, o jogador poderá inserir seu apelido para ser gravado na tabela de pontuação.\n\n");
  
  printf(HBLK "\nPressione " BHBLK "ENTER" reset HBLK " para retornar ao " BHBLK "MENU" reset "\n\n");
  getchar();
}

//referencia: https://stackoverflow.com/questions/21091191/implementing-a-keypress-event-in-c
//muda o tipo de leitura do stdin pra pegar teclas sem precisar de ENTER
void mudarLeitura()
{
  tcgetattr(0, &info);          /* get current terminal attirbutes; 0 is the file descriptor for stdin */
  info.c_lflag &= ~ICANON;      /* disable canonical mode */
  info.c_cc[VMIN] = 1;          /* wait until at least one keystroke available */
  info.c_cc[VTIME] = 0;         /* no timeout */
  tcsetattr(0, TCSANOW, &info); /* set immediately */
}

//volta a leitura de teclas para o modo padrao
void restaurarLeitura()
{
  tcgetattr(0, &info);
  info.c_lflag |= ICANON;
  tcsetattr(0, TCSANOW, &info);
}


//printa as colocacoes dos jogadores: opcao [3]
void ver_highscore()
{

  limpa_tela();

  printf(BHRED "\t\t[HIGHSCORE]\n\n" reset);

  FILE *f;

  f = fopen("new_ranking.txt", "r");

  if (f == NULL)
  {
    printf("Erro ao abrir arquivo para leitura\n");
    return;
  }

  char line[255];
  int alternaCor = 0;

  while (!feof(f))
  {
    sprintf(line , ""); //limpa o conteudo de line

    fscanf(f, "%s", line);

    // troca '_' por tab
    for (int i = 0; i < strlen(line); i++)
    {
      if (line[i] == '_')
      {
        line[i] = '\t';
      }
    }

    //para editar o visual da tabela
    if (alternaCor % 2 == 0)
    {
      printf(REDB "%s\n" reset, line);
    }
    else
    {
      printf("%s\n", line);
    }

    alternaCor++;
  }

  printf(HBLK "\nPressione " BHBLK "ENTER" reset HBLK " para retornar ao " BHBLK "MENU" reset "\n\n");
  getchar();

  fclose(f);
}

//roda em background para contar o tempo de cada rodada
void *timer(void *arg)
{

  while (1)
  {
    sleep(1);
    seconds++;
  }

  return NULL;
}

//referencia: https://stackoverflow.com/questions/2907062/fgets-instructions-gets-skipped-why
//limpa a entrada padrao de resquicio de caracteres
void limpa_stdin()
{
  int c;
  while ((c = getchar()) != '\n' && c != EOF);
}

//modo multiplayer continuar jogando
char continuar_jogando()
{
  printf("\n" BRED "[1] " reset BWHT "Menu principal\n" reset);
  printf("" BRED "[2] " reset BWHT "Continuar jogando\n" reset);

  mudarLeitura();
  int resp;
  resp = getchar();
  restaurarLeitura();

  return resp;
}

//exibe o main menu
void main_menu()
{
  int resp;
  int invalido;
  int loop = 1;
  int segundos = 0;

  do
  {
    limpa_tela();
    // printf("MAIN MENU\n");

    printf(BRED);
    printf("    _                           _             __                    \n");
    printf("   (_)                         | |           / _|                   \n");
    printf("    _  ___   __ _  ___       __| | __ _     | |_ ___  _ __ ___ __ _ \n");
    printf("   | |/ _ \\ / _` |/ _ \\     / _` |/ _` |    |  _/ _ \\| '__/ __/ _` |\n");
    printf("   | | (_) | (_| | (_) |   | (_| | (_| |    | || (_) | | | (_| (_| |\n");
    printf("   | |\\___/ \\__, |\\___/     \\__,_|\\__,_|    |_| \\___/|_|  \\___\\__,_|\n");
    printf("  _/ |       __/ |                                                  \n");
    printf(" |__/       |___/                                                   \n\n\n");
    printf(reset);

    // printf("seconds: %d\n", segundos);

    printf("  " BRED "[1] " reset BWHT "Jogar com amigo\n" reset);
    printf("  " BRED "[2] " reset BWHT "Jogar sozinho\n" reset);
    printf("  " BRED "[3] " reset BWHT "Ranking de pontuação\n" reset);
    printf("  " BRED "[4] " reset BWHT "Ajuda\n" reset);
    printf("  " BRED "[5] " reset BWHT "Sair\n\n" reset);

    // printf("Opcao:\n");
    // scanf("%d", &resp);

    mudarLeitura();
    int resp;
    resp = getchar();
    restaurarLeitura();

    // resolver bug do skip do fgets
    // limpa_stdin();

    jogando = '2';

    switch (resp)
    {
    case 49:
      while (jogando == '2')
      {
        multiplayer();
      }
      break;

    case 50:
      singleplayer();
      break;

    case 51:
      ver_highscore();
      break;

    case 52:
      exibeAjuda();
      break;

    case 53:
      limpa_tela();
      loop = 0;
      break;

    case 27: // ESC key
      limpa_tela();
      loop = 0;
      break;

    default:
      limpa_tela();
      printf("\nOpcao inválida! Tente novamente.\n\n");
      break;
    }
  } while (loop == 1);
}

//exibe estados da forca
void forca(int estado)
{
  printf(YEL);
  if (estado == 0)
  {
    printf("\n\t   ▄▄▄▄▄▄▄▄▄▄");
    printf("\n\t  ▐▐▛       ▜▋");
    printf("\n\t  ▋▛         ▋");
    printf("\n\t  ▋▋         ╿");
    printf("\n\t  ▋▋");
    printf("\n\t  ▊▊");
    printf("\n\t  ▊▊");
    printf("\n\t  ▉▉");
    printf("\n\t▃▆██▇▆▃");
    printf("\n");
  }
  else if (estado == 1)
  {
    printf("\n\t   ▄▄▄▄▄▄▄▄▄▄");
    printf("\n\t  ▐▐▛       ▜▋");
    printf("\n\t  ▋▛         ▋");
    printf("\n\t  ▋▋         ╿");
    printf("\n\t  ▋▋         " RED "⬮" reset YEL);
    printf("\n\t  ▊▊");
    printf("\n\t  ▊▊");
    printf("\n\t  ▉▉");
    printf("\n\t▃▆██▇▆▃");
    printf("\n");
  }
  else if (estado == 2)
  {
    printf("\n\t   ▄▄▄▄▄▄▄▄▄▄");
    printf("\n\t  ▐▐▛       ▜▋");
    printf("\n\t  ▋▛         ▋");
    printf("\n\t  ▋▋         ╿");
    printf("\n\t  ▋▋         " RED "⬮" reset YEL);
    printf("\n\t  ▊▊         " RED "█" reset YEL);
    printf("\n\t  ▊▊");
    printf("\n\t  ▉▉");
    printf("\n\t▃▆██▇▆▃");
    printf("\n");
  }
  else if (estado == 3)
  {
    printf("\n\t   ▄▄▄▄▄▄▄▄▄▄");
    printf("\n\t  ▐▐▛       ▜▋");
    printf("\n\t  ▋▛         ▋");
    printf("\n\t  ▋▋         ╿");
    printf("\n\t  ▋▋         " RED "⬮" reset YEL);
    printf("\n\t  ▊▊        " RED "▔█" reset YEL);
    printf("\n\t  ▊▊");
    printf("\n\t  ▉▉");
    printf("\n\t▃▆██▇▆▃");
    printf("\n");
  }
  else if (estado == 4)
  {
    printf("\n\t   ▄▄▄▄▄▄▄▄▄▄");
    printf("\n\t  ▐▐▛       ▜▋");
    printf("\n\t  ▋▛         ▋");
    printf("\n\t  ▋▋         ╿");
    printf("\n\t  ▋▋         " RED "⬮" reset YEL);
    printf("\n\t  ▊▊        " RED "▔█▔" reset YEL);
    printf("\n\t  ▊▊");
    printf("\n\t  ▉▉");
    printf("\n\t▃▆██▇▆▃");
    printf("\n");
  }
  else if (estado == 5)
  {
    printf("\n\t   ▄▄▄▄▄▄▄▄▄▄");
    printf("\n\t  ▐▐▛       ▜▋");
    printf("\n\t  ▋▛         ▋");
    printf("\n\t  ▋▋         ╿");
    printf("\n\t  ▋▋         " RED "⬮" reset YEL);
    printf("\n\t  ▊▊        " RED "▔█▔" reset YEL);
    printf("\n\t  ▊▊         " RED "▎" reset YEL);
    printf("\n\t  ▉▉");
    printf("\n\t▃▆██▇▆▃");
    printf("\n");
  }
  else if (estado == 6)
  {
    printf("\n\t   ▄▄▄▄▄▄▄▄▄▄");
    printf("\n\t  ▐▐▛       ▜▋");
    printf("\n\t  ▋▛         ▋");
    printf("\n\t  ▋▋         ╿");
    printf("\n\t  ▋▋         " RED "⬮" reset YEL);
    printf("\n\t  ▊▊        " RED "▔█▔" reset YEL);
    printf("\n\t  ▊▊         " RED "▎▎" reset YEL);
    printf("\n\t  ▉▉");
    printf("\n\t▃▆██▇▆▃     PERDEU!\n");
  }
  printf(reset);
}

void limpa_tela()
{
  system("clear");
}

void multiplayer()
{

  limpa_tela();

  char p_sec[100]; // palavra secreta
  char dica[100];

  printf(BHRED "[MODO MULTIPLAYER]\n\n" reset);
  printf("  " HBLK "O " BHBLK "JOGADOR 1" reset HBLK " deve escolher uma palavra secreta para o " BHBLK "JOGADOR 2" reset HBLK " adivinhar.\n\n" reset);

  printf(BHWHT "\nInsira a PALAVRA SECRETA: " reset);

  fgets(p_sec, 100, stdin);

  printf(BHWHT "\nInsira uma DICA: " reset);

  fgets(dica, 100, stdin);

  // converte a p_sec para uppercase
  int j = 0;
  char ch;
  while (p_sec[j])
  {
    ch = toupper(p_sec[j]);
    p_sec[j] = ch;
    j++;
  }

  limpa_tela();

  // retira o ultimo caracter de p_sec que esta a mais devido a captura ser com fgets()
  p_sec[strlen(p_sec) - 1] = '\0';

  char p_tela[100];      // palavra para tela
  strcpy(p_tela, p_sec); // copia de p_sec

  // substitui letras por '_'
  for (int i = 0; i < strlen(p_tela); i++)
  {
    if (p_tela[i] == ' ')
    {
      continue;
    }
    else
    {
      p_tela[i] = '_';
    }
  }

  int erros = 0;
  char chutadas[255] = "";
  int repetida = 0;
  while (1)
  {
    // imprimir a forca
    limpa_tela();

    printf(BHRED "\n\t\t[DICA] " reset BHWHT "%s\n" reset, dica);

    printf(HBLK "\nTentadas:" BHBLK " %s\n" reset, chutadas);

    int errou = 1; // 1=sim 0=nao

    forca(erros);

    // imprimir os underline/underscores '_' para cada letra da palavra secreta, ou seja, imprimir p_tela
    printf(BHWHT "\n\t");
    for (int i = 0; i < strlen(p_tela); i++)
    {
      printf("%c ", p_tela[i]);
    }
    printf(reset);

    // recebe a letra
    printf(BHWHT "\n\nLetra: " reset);
    char letra;

    letra = fgetc(stdin);

    // converte letra minuscula para maiuscula
    if (letra <= 122 && letra >= 97)
    {
      letra -= 32;
    }

    // confere se letra ja nao foi chutada
    repetida = 0;
    for (int i = 0; i < strlen(chutadas); i += 2)
    {
      if (chutadas[i] == letra)
      {
        repetida = 1;
      }
    }

    // verifica se o usuario pediu para chutar
    char strBarra[5] = "";
    if (letra == '/')
    {
      fgets(strBarra, 5, stdin);
      letra = ' '; // para nao triggar erro depois
      limpa_stdin();
    }

    if (strcmp(strBarra, "chut") == 0){ // pega o comando /chutar
      char chute[strlen(p_sec) + 1];

      printf(BHWHT "\nPalavra: " reset);
      scanf("%s",chute);
      limpa_stdin();
    
      // lowercase to uppercase na string chute
      for (int i = 0; i < strlen(chute); i++)
      {
        if (chute[i] <= 122 && chute[i] >= 97)
        {
          chute[i] -= 32;
        }
      }

      printf(HBLK"\nPressione "BHBLK"ENTER"reset HBLK" para chutar a palavra "BHBLK"%s\n"reset, chute);
      getchar();

      if (strcmp(chute, p_sec) == 0)
      {

        // preenche palavra acertada
        for (int i = 0; i < strlen(p_tela); i++)
        {
          p_tela[i] = p_sec[i];
        }

        goto vitoria1;
      }
      else
      {
        goto derrota1;
      }
    }

    // adiciona na string de letras ja chutadas
    char strLetras[3];
    if (letra != '\n' && letra != ' ' && repetida == 0 && letra != '/')
    {
      sprintf(strLetras, "%c ", letra);
      strcat(chutadas, strLetras);
    }

    //'limpa' stdin
    limpa_stdin();

    if (repetida)
    {
      // printf(HBLK"\nEsta letra ja foi tentada!\n"reset);
      errou = 0;
    }

    // se letra correta atualiza palavra na tela
    // verifica se a letra esta correta
    for (int i = 0; i < strlen(p_tela); i++)
    {
      if (letra == p_sec[i])
      { // certo
        p_tela[i] = letra;
        errou = 0;
      }
    }
    // senao, incrementa erros
    if (errou == 1)
    {
      erros++;
    }

    // verifica se p_sec eh igual a p_tela
    if (strcmp(p_tela, p_sec) == 0)
    {
      vitoria1:
      // ganhou
      limpa_tela();

      forca(erros);

      printf(BHWHT "\n\t");
      for (int i = 0; i < strlen(p_tela); i++)
      {
        printf("%c ", p_tela[i]);
      }
      printf(reset);

      printf(BHGRN "\n\n[ACERTOU] " reset GRN);
     
      printf("%s", p_tela);
      printf("\n" reset);
      jogando = continuar_jogando();
      break;
    }
    // verifica se perdeu
    if (erros == 6)
    {
      derrota1:
      // perdeu
      limpa_tela();

      forca(erros);

      printf(BHWHT "\n\t");
      for (int i = 0; i < strlen(p_tela); i++)
      {
        printf("%c ", p_tela[i]);
      }
      printf(reset);

      printf(BHRED "\n\n[ERROU] " reset RED);
      /*for(int i=0;i<strlen(p_tela);i++){
        printf("%c", p_tela[i]);
      }*/
      printf("%s", p_sec);
      printf("\n" reset);

      jogando = continuar_jogando();
      break;
    }
  }
}

void singleplayer()
{
  int escolha;
  char dificuldade[20];

  int invalido;

  int perdeu = 0;

  limpa_tela();

  do
  {

    invalido = 0;

    printf(BHRED "[MODO SINGLEPLAYER]\n\n" reset);
    printf("  " HBLK "Escolha uma dificuldade e tente acertar o máximo de palavras possíveis para pontuar.\n\n" reset);

    printf(BHWHT "\nSelecione a dificuldade: \n\n" reset);
    printf("  " BRED "[1] " reset BWHT "Fácil\n" reset);
    printf("  " BRED "[2] " reset BWHT "Médio\n" reset);
    printf("  " BRED "[3] " reset BWHT "Difícil\n\n" reset);

    mudarLeitura();
    escolha = getchar();
    restaurarLeitura();

    switch (escolha)
    {
    case 49: // 1
      strcpy(dificuldade, "FACIL");
      // com dica + ajuda de letras
      dica = 1;
      ajuda = 1;
      break;

    case 50:
      strcpy(dificuldade, "MEDIO");
      // com dica
      dica = 1;
      ajuda = 0;
      mult_dificuldade = 2;
      break;

    case 51:
      strcpy(dificuldade, "DIFICIL");
      dica = 0;
      ajuda = 0;
      mult_dificuldade = 3;
      // sem dica
      break;

    case 27: // ESC key
      exit(0);
      break;

    default:
      invalido = 1;
      limpa_tela();
      printf("Opcao inválida! Tente novamente.\n\n");
      break;
    }
  } while (invalido);

  printf(HBLK "\nDificuldade " BHBLK "%s" reset HBLK " selecionada. Aperte " BHBLK "ENTER" reset HBLK " para começar!\n\n" reset, dificuldade);
  // printf("\nDificuldade %s selecionada. Aperte ENTER para começar!\n", dificuldade);

  getchar();

  //para gravar no arquivo de pontuacao depois
  int highscore = 0;
  int tempo_total = 0; 
  int rodada = 1;

  while (perdeu == 0)
  {

    limpa_tela();

    //referencia: https://stackoverflow.com/questions/232237/whats-the-best-way-to-return-a-random-line-in-a-text-file-using-c/232287#232287
    // pegar palavra secreta do arquivo
    FILE *f;
    int nLines = 0;
    char line[100];
    int randLine;
    int i;

    char p_sec[100];
    char dica_game[100];

    srand(time(NULL));

    srand(time(0));
    f = fopen("wordlist.txt", "r");

    //pegar palavra aleatoria do arquivo wordlist.txt

    //conta linhas
    while (!feof(f))
    {
      fgets(line, 100, f);
      nLines++;
    }

    // numero da linha aleatoria
    randLine = rand() % nLines;

    fseek(f, 0, SEEK_SET);
    for (i = 0; !feof(f) && i <= randLine; i++) //vai andando no arquivo
      fgets(line, 100, f);

    sscanf(line, "%s %100[0-9a-zA-Z ]", dica_game, p_sec);

    printf("A palavra secreta sorteada foi %s e a dica é %s", p_sec, dica_game);
    getchar();

    char p_tela[100];      // palavra para tela
    strcpy(p_tela, p_sec); // copia de p_sec

    // substitui letras por '_'
    for (int i = 0; i < strlen(p_tela); i++)
    {

      if (p_tela[i] == ' ')
      {
        continue;
      }
      else
      {
        p_tela[i] = '_';
      }
    }

    int erros = 0;
    int points;
    int score_multiplier = 10; // leva em conta: tempo, dificuldade, tamanho palavra, erros
    seconds = 0;               // resetar a contagem que roda em segundo plano

    char chutadas[255] = "";
    int repetida = 0;
    int qtd_dicas = strlen(p_sec) / 3;

    while (1)
    {
      // imprimir a forca
      limpa_tela();

      printf(BHBLU "[SCORE] " reset BHCYN "%d\n" reset, highscore);
      printf(BHBLU "[FORCA] " reset BHWHT "%d\n" reset, rodada);

      if (dica)
      {
        printf(BHRED "\n\t\t[DICA] " reset BHWHT "%s\n" reset, dica_game);
      }

      printf(HBLK "\nTentadas:" BHBLK " %s\n" reset, chutadas);

      int errou = 1; // 1=sim 0=nao

      forca(erros);

      printf(BHWHT "\n\t");
      for (int i = 0; i < strlen(p_tela); i++)
      {
        printf("%c ", p_tela[i]);
      }
      printf(reset);

      // recebe a letra
      printf(BHWHT "\n\nLetra: " reset);
      char letra;

      // scanf("%c", &letra);

      letra = fgetc(stdin);

      // converte letra minuscula para maiuscula
      if (letra <= 122 && letra >= 97)
      {
        letra -= 32;
      }

      // confere se letra ja nao foi chutada
      repetida = 0;
      for (int i = 0; i < strlen(chutadas); i += 2)
      {
        if (chutadas[i] == letra)
        {
          repetida = 1;
        }
      }

      // verifica se o usuario pediu uma dica
      char strBarra[5] = "";
      int letraAleatoria;
      int dicaValidada = 0;
      if (letra == '/')
      {
        fgets(strBarra, 5, stdin);
        letra = ' '; // para nao triggar erro depois
        limpa_stdin();
      }

      if (strcmp(strBarra, "dica") == 0)
      {
        if (qtd_dicas > 0)
        {
          while (1)
          { // roda ate encontrar uma posicao vazia para dica
            letraAleatoria = rand() % strlen(p_sec);
            if (p_tela[letraAleatoria] == '_')
            {
              p_tela[letraAleatoria] = p_sec[letraAleatoria];

              if (strcmp(p_tela, p_sec) == 0)
              {
                goto vitoria;
              }

              dicaValidada = 1;
              qtd_dicas--;
              break;
            }
            else
            {
              continue;
            }
          }
        }
        else
        {
          dicaValidada = 1;
        }
      }
      else if (strcmp(strBarra, "chut") == 0)
      { // pega o comando /chutar
        char chute[strlen(p_sec) + 1];

        printf(BHWHT "\nPalavra: " reset);
        scanf("%s",chute);
        limpa_stdin();

        // lowercase to uppercase na string chute
        for (int i = 0; i < strlen(chute); i++)
        {
          if (chute[i] <= 122 && chute[i] >= 97)
          {
            chute[i] -= 32;
          }
        }

        printf(HBLK"\nPressione "BHBLK"ENTER"reset HBLK" para chutar a palavra "BHBLK"%s\n"reset, chute);
        getchar();

        if (strcmp(chute, p_sec) == 0)
        {

          // preenche palavra acertada
          for (int i = 0; i < strlen(p_tela); i++)
          {
            p_tela[i] = p_sec[i];
          }

          goto vitoria;
        }
        else
        {
          goto derrota;
        }
      }

      if (dicaValidada)
      {
        continue;
      }

      // adiciona na string de letras ja chutadas
      char strLetras[1];
      if (letra != '\n' && letra != ' ' && repetida == 0 && letra != '/')
      {
        sprintf(strLetras, "%c ", letra);
        strcat(chutadas, strLetras);
      }

      //'limpa' stdin
      limpa_stdin();

      // se letra correta atualiza palavra na tela
      // verifica se a letra esta correta

      //se o usuario inserir uma letra repetida nao ocorre erro
      if (repetida)
      {
        errou = 0;
      }

      for (int i = 0; i < strlen(p_tela); i++)
      {
        if (letra == p_sec[i])
        { // certo
          p_tela[i] = letra;
          errou = 0;
        }
      }
      // senao, incrementa erros
      if (errou == 1)
      {
        erros++;
      }

      // verifica se p_sec eh igual a p_tela
      if (strcmp(p_tela, p_sec) == 0)
      {
      vitoria:
        // ganhou
        limpa_tela();

        forca(erros);

        printf(BHWHT "\n\t");
        for (int i = 0; i < strlen(p_tela); i++)
        {
          printf("%c ", p_tela[i]);
        }
        printf(reset);

        printf(BHGRN "\n\n[ACERTOU] " reset GRN);
       
        printf("%s", p_tela);
        printf("\n" reset);

        printf(BHBLU "\n[TEMPO]" reset BHWHT " %02d:%02d:%02d\n" reset, seconds / 3600, (seconds % 3600) / 60, seconds % 60);
        tempo_total += seconds;
        rodada++;

        score_multiplier *= mult_dificuldade;
        score_multiplier *= strlen(p_sec);
        score_multiplier /= (seconds / 60) + 1;
        score_multiplier /= erros + 1;

        points = 100 * score_multiplier;
        highscore += points;

        printf(BHCYN "\n+" reset BHCYN "%d" reset BHWHT " pontos!\n" reset, points);
        printf(HBLK "\n\nPressione " BHBLK "ENTER" reset HBLK " para jogar com a próxima palavra!\n\n" reset);

        getchar();
        limpa_tela();

        break;
      }
      // verifica se perdeu
      if (erros == 6)
      {
      derrota:
        tempo_total += seconds;

        // perdeu
        limpa_tela();

        forca(erros);

        printf(BHWHT "\n\t");
        for (int i = 0; i < strlen(p_tela); i++)
        {
          printf("%c ", p_tela[i]);
        }
        printf(reset);

        printf(BHRED "\n\n[ERROU] " reset RED);
        
        printf("%s", p_sec);
        printf("\n" reset);

        perdeu = 1;
        break;
      }
    }
  }

  printf(BHBLU "\n[SCORE FINAL]" reset BHCYN " %d!\n" reset, highscore);
  printf(BHBLU "[APELIDO]" reset BLU " (ate 7 caracteres): " reset);

  char apelido[8];
  fgets(apelido, 8, stdin);
  // limpa_stdin();

  // remove \n do apelido
  for (int i = 0; i < strlen(apelido) && i < 8; i++)
  {
    if (apelido[i] == '\n')
    {
      apelido[i] = '\0';
    }
  }

  char maisInfo[255];

  sprintf(maisInfo, "%d_%s_%02d:%02d:%02d", rodada, dificuldade, tempo_total / 3600, (tempo_total % 3600) / 60, tempo_total % 60);

  write_highscore(highscore, apelido, maisInfo); // maisInfo é uma string com a dificuldade, rodadas e tempo
  
  printf(HBLK "\nPressione " BHBLK "ENTER" reset HBLK " para retornar ao " BHBLK "MENU" reset "\n\n");

  getchar();
}

int main(void)
{

  //cria a thread do timer
  pthread_t t1;
  pthread_create(&t1, NULL, timer, NULL);

  //chama o main menu e o loop do jogo inicia
  main_menu();

  return 0;
}

/*

Referencias:
  https://gist.github.com/RabaDabaDoba/145049536f815903c79944599c6f952a
  https://stackoverflow.com/questions/21091191/implementing-a-keypress-event-in-c
  https://stackoverflow.com/questions/2907062/fgets-instructions-gets-skipped-why
  https://stackoverflow.com/questions/232237/whats-the-best-way-to-return-a-random-line-in-a-text-file-using-c/232287#232287
  https://stackoverflow.com/questions/9033060/c-function-to-insert-text-at-particular-location-in-file-without-over-writing-th
  https://www.youtube.com/watch?v=CylpD8zXHZA&t=658s
  https://www.youtube.com/watch?v=JYMAid8-aOE

*/
