all: 
	gcc cliente_psta.c -o cliente_psta
	gcc servidor_psta.c -o servidor_psta

cliente: cliente_psta.c
	gcc cliente_psta.c -o cliente_psta

servidor: servidor_psta.c
	gcc servidor_psta.c -o servidor_psta

