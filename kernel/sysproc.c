#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#define MAXSEM 512

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

struct semaphore{
  int value; //Valor del semaforo 
  struct spinlock lock; //variable en la cual puedo decidir si esta bloqueado o no el proceso 
};

struct semaphore semaphore_counter[MAXSEM]; //Arreglo con todos los semaforos disponibles 

/* void semaphore_init(void){
  for(unsigned int i = 0; i < MAXSEM; i++){
    semaphore_counter[i].value = -1;
  }
} 
*/

//Abre y/o inicializa el semáforo “sem” con  un valor arbitrario “value”.
uint64 
sys_sem_open(void){

  char num[4];
  char lock_name[18] = "semaphore no. "; //variables donde vamos a guardar el nombre para iniciar 
  
  int semaphore_id, init_value;
  argint(0, &semaphore_id); // En el registro 0 se guarda el id 
  argint(1, &init_value); // En el registro 1 se guarda el value

  if (semaphore_id < 0){
    return 0;
  }

  uint_to_string(num, semaphore_id);
  strcat(lock_name,num); //creamos el nombre del semaforo

  initlock(&semaphore_counter[semaphore_id].lock, lock_name); //Inicializamos la variable lock

  //Critical zone
  acquire(&semaphore_counter[semaphore_id].lock); //bloqueamos para que ningun otro proceso pueda acceder al semaforo

  semaphore_counter[semaphore_id].value = init_value; //asignamos

  release(&semaphore_counter[semaphore_id].lock); //liberamos 

  return 1;

}

//Incrementa el semáforo ”sem” desbloqueando los procesos cuando su valor es 0.
uint64 
sys_sem_up(void){
  int semaphore_id;
  argint(0,&semaphore_id);

  if (semaphore_id < 0){
    return 0;
  }

  //Critical zone
  acquire(&semaphore_counter[semaphore_id].lock);

  if(semaphore_counter[semaphore_id].value == 0){
    wakeup(&semaphore_counter[semaphore_id]);
  }

  semaphore_counter[semaphore_id].value++;

  release(&semaphore_counter[semaphore_id].lock);

  return 1;
}

//Decrementa el semáforo ”sem” bloqueando los procesos cuando su valor es 0. El valor del semaforo nunca puede ser menor a 0
uint64 
sys_sem_down(void){
  int semaphore_id;
  argint(0,&semaphore_id);

  if (semaphore_id < 0){
    return 0;
  }

  //Critical zone
  acquire(&semaphore_counter[semaphore_id].lock);

  while(semaphore_counter[semaphore_id].value == 0){
    sleep(&semaphore_counter[semaphore_id],&semaphore_counter[semaphore_id].lock);
  }

  semaphore_counter[semaphore_id].value--;

  release(&semaphore_counter[semaphore_id].lock);

  return 1;
}

//Libera el semáforo “sem”
uint64 
sys_sem_close(void){
  int semaphore_id;
  argint(0,&semaphore_id);

  if (semaphore_id < 0){
    return 0;
  }

  //Critical zone
  acquire(&semaphore_counter[semaphore_id].lock);

  semaphore_counter[semaphore_id].value = -1; //Cerramos semaforo

  release(&semaphore_counter[semaphore_id].lock);

  return 1;
}

//acquire() release(), wakeup(), sleep() y argint()
//acquire bloquea informacion para que no se pueda acceder a ella de forma simultanea por muchos hilos, release la libera 