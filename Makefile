all: 
	gcc cliente_psta.c -o cliente_psta -g 
	gcc servidor_psta.c -o servidor_psta -g

cliente: cliente_psta.c
	gcc cliente_psta.c -o cliente_psta

servidor: servidor_psta.c
	gcc servidor_psta.c -o servidor_psta

