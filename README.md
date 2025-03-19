# Trabalho Prático 2: Redes de Sensores Sem Fio (RSSF)

## Descrição
Este projeto simula uma Rede de Sensores Sem Fio (RSSF), implementando um servidor e clientes que se comunicam via protocolo TCP/IP. O sistema é desenvolvido em C e utiliza múltiplas threads no servidor para gerenciar conexões e distribuir dados de sensoriamento como temperatura, umidade e qualidade do ar.

## Funcionalidades
- **Servidor Multithread:** Gerencia conexões simultâneas e distribui dados entre os sensores.
- **Cliente de Sensor:** Simula sensores que coletam dados ambientais.
- **Descoberta Dinâmica de Vizinhos:** Sensores ajustam suas medições com base nos dados dos sensores mais próximos.

## Compilação
É necessário ter o GCC instalado para compilar os códigos fonte. Após clonar o repositório, navegue até a pasta do projeto e execute make para a compilação.

## Uso
### Servidor
Para executar o servidor:
./bin/server <v4|v6> <porta>

- `<v4|v6>`: Define o uso de IPv4 ou IPv6.
- `<porta>`: Porta para o servidor escutar.

### Cliente
Para executar um cliente:

./bin/client <endereço IP> <porta> -type <tipo> -coords <x> <y>

- `<endereço IP>`: IP do servidor.
- `<porta>`: Porta do servidor.
- `<tipo>`: Tipo do sensor (`temperature`, `humidity`, `air_quality`).
- `<x> <y>`: Coordenadas do sensor no grid 10x10.

