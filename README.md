# Desenvolvimento do Projeto
Projeto desenvolvido por:
- Adriano Munin - [@adrianomunin](https://github.com/adrianomunin)
- Fabio Irokawa - [@fabioirokawa](https://github.com/fabioirokawa)
- Iago Lourenço - [@iaglourenco](https://github.com/iaglourenco)
- Lucas Coutinho - [@lucasrcoutinho](https://github.com/lucasrcoutinho)
# Descrição do Projeto
## Objetivo:
O objetivo deste  projeto é implementar um cliente  e um servidor para  um **P**rotocolo  **S**imples para
**T**ransferência de **A**rquivos (**PSTA**). O Cliente fornece uma interface na forma de linha de comandos que
permite ao usuário:

 - Conectar-se a um servidor;
  
 - Listar os arquivos localizados no diretório corrente do servidor;

 - Receber um arquivo do servidor;

 - Enviar um arquivo para o servidor;

 - Encerrar a conexão com o servidor.

O servidor espera por conexões de clientes em uma porta conhecida. Após um cliente conectar-se ao
servidor, o servidor espera por comandos. Quando o cliente enviar uma mensagem de encerramento de
conexão, o servidor terminará a conexão. O servidor descrito trata-se de um servidor concorrente, ou seja,
ele é capaz de atender a diversos clientes simultaneamente.
O projeto consiste na implementação de dois programas independentes em C que executem em sistemas
operacionais  *`Linux`*, um cliente  chamado `cliente_psta` e um servidor chamado `servidor_psta`. 
Ao  ser executado o programa cliente exibe um prompt, que permite ao usuário digitar os seguintes comandos:

 - `conectar <nome do servidor> [<porta do servidor>]`

 - `listar`

 - `receber <arquivo remoto> [<arquivo local>]`

 - `enviar <arquivo local> [<arquivo remoto>]`

 - `encerrar`
 
Todos os comandos digitados pelo usuário serão sequências de caracteres ASCII. Os comandos enviados
pelo cliente ao servidor também devem estar na forma de sequências de caracteres ASCII. Ao receber um
comando, o servidor deve analisá-lo e executar a ação apropriada, conforme descrito a seguir: 

#### `conectar <nome do servidor> [<porta do servidor>]`
 - Este comando permite ao cliente conectar-se a um servidor. O nome do servidor poderá ser dado
na forma literal (p.ex. maquina1.puc-campinas.edu.br) ou na forma de um endereço IP em notação
decimal pontuada (p. ex. 200.136.254.139). Ao receber este comando, o cliente deve iniciar uma
conexão com o servidor.

#### `listar`
 - Quando este comando for enviado ao servidor, este retornará a lista de arquivos no diretório onde
ele está sendo executado. A lista será enviada como uma sequência de caracteres ASCII terminada
por   um   delimitador   de   linha   (\n).   O   cliente   deverá   receber   a   lista   e   exibi-la   na   tela.
Dica: veja as páginas do manual para getcwd, opendir, readdir, closedir
  
#### `receber <arquivo remoto> [<arquivo local>]`
 - Permite ao cliente solicitar ao servidor o arquivo de nome <arquivo remoto>. Este arquivo deverá
ser sempre tratado como um arquivo binário. Ao receber o arquivo especificado o cliente deverá
salvá-lo no diretório onde está executando com o nome <arquivo local> ou, caso este nome não
tenha sido especificado, com o nome <arquivo remoto>.

#### `enviar <arquivo local> [<arquivo remoto>]`
 - Este comando é simétrico ao comando receber. Ele permite o envio de um arquivo local ao cliente
para o diretório de trabalho do servidor.

#### `encerrar`
 - Ao receber este comando, o cliente deverá enviá-lo para o servidor e terminar a conexão. Quando o
servidor receber este comando ele deverá fechar o seu lado da conexão.
Para implementar a comunicação entre o cliente e o servidor use dois soquetes TCP: um de controle e outro
de dados. O soquete de controle será usado para o envio de mensagens de controle (comandos enviados
pelo cliente). 
O soquete de dados será usado para o envio e recepção de mensagens de dados. Neste esquema, o programa 
`servidor_psta` age como servidor na porta de controle e o programa 
`cliente_psta` age como servidor na porta de dados. A conexão de dados é estabelecida e desfeita para cada comando
