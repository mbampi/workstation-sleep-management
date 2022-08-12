# workstation-sleep-management

Sleep management distributed system for workstations that belong to the same physical network segment.

INF01151 – SISTEMAS OPERACIONAIS II N


## Arquitetura

Threads:
- Discovery Subservice
  - incluir/remover participantes da lista
- Monitoring Subservice
  - atualizar o status de cada participante
- Management Subservice
  - manter a lista de participantes
- Interface Subservice
  - aguardar por atualizações na lista, e escrevê-las na tela para o usuário
  - enquanto a tabela estiver sendo lida, nenhum escritor poderá modificá-la


Dados a serem mantidos:
- lista  de  participantes
- participante
  - hostname
  - endereço IP
  - endereço MAC
  - status do participante (asleep ou awaken)

Estrutura de dados que devemos manter consistencia: lista de participantes
Ele será protegido por sessão crítica.

## Como executar

### Participante
Iniciar:
```
make participant
```

Sair do servico: 
```
EXIT
```

### Manager
Iniciar
```
make manager
```

Acordar outra maquina: (somente o manager consegue)
```
WAKEUP hostname 
```

