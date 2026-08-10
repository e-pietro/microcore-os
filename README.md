# MicroCore-OS | Firmware Simulation in C

O **MicroCore-OS** é uma simulação de firmware para um sistema embarcado de controle de Unidade de Controle Eletrônico (ECU). O projeto foi construído do zero com o objetivo de dominar a **linguagem C de baixo nível**, o gerenciamento defensivo de memória e padrões de arquitetura utilizados na indústria automobilística e de IoT.

---

## Finalidade do Projeto

Este projeto nasceu de uma jornada intensiva de aprendizado prático em C, progredindo desde a sintaxe básica até a implementação de arquiteturas complexas. Ele demonstra a capacidade de escrever código seguro, modular e otimizado para sistemas onde os recursos de memória e processamento são restritos.

### Principais Objetivos Alcançados:
- **Entendimento da Arquitetura de Memória:** Manipulação direta de regiões de *Stack* e *Heap*, prevenção de *Segmentation Faults* e controle de vazamentos de memória (*Memory Leaks*).
- **Abstração de Hardware:** Manipulação de registradores usando operações *Bitwise* e *Bit-Fields*.
- **Arquitetura Industrial:** Organização modular em camadas, padrões de Inversão de Dependência com *Callbacks* e controle de estados por *FSM*.

---

## 🛠️ Tecnologias e Padrões Implementados

- **Linguagem:** C (Padrão C11)
- **Compilador:** GCC (com flags de aviso ativadas: `-Wall -Wextra`)
- **Ambiente:** Linux Terminal / VS Code
- **Padrões de Software & Conceitos Aplicados:**
  - **Gerenciamento Dinâmico de Memória:** Uso seguro de `malloc()`, `realloc()` e `free()` para expansão do histórico de telemetria em tempo de execução.
  - **Programação Defensiva:** Cláusulas de guarda (`if (ptr == NULL)`), uso de `const` para parâmetros somente leitura e ponteiros aterrados.
  - **Máquina de Estados Finita (FSM):** Sistema transicional entre os estados `BOOT`, `STANDBY`, `ACTIVE` e `ERROR` (com travamento de segurança *Latching Fault*).
  - **Inversão de Dependência (Callbacks):** Ponteiros de função (`AlarmCallback`) para acionamento de emergência desacoplado da aplicação.
  - **Persistência Binária (Simulação Flash):** Leitura e escrita direta de bytes no disco (`fwrite`/`fread`) com serialização correta de ponteiros.
  - **Otimização Extrema de Memória:** Estruturas agrupadas com `union` e `struct Bit-Fields` para controle de atuadores bit a bit.

---

## Arquitetura do Projeto

O código-fonte foi separado modularmente em arquivos `.h` (interfaces/contratos) e `.c` (implementação):

```text
main_project/
├── ecu.h          # Interfaces, protótipos, definições de FSM, Structs e Bit-Fields
├── ecu.c          # Lógica do motor de FSM, atuadores, persistência e memória Heap
└── microcore.c    # Ponto de entrada (main), Super-Loop embarcado e registro de Callbacks
