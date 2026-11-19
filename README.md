# Proyecto 4: Copy-On-Write y MiniDB
# Integrantes
* Ana Isabella Gómez García
* Samuel Quintero Quintero
* Ashly Robayo Parra
# Descripción del proyecto
Este proyecto extiende el sistema operativo xv6-riscv mediante la
implementación de mecanismos de administración eficiente de memoria
basados en Copy-on-Write (CoW), estadísticas del uso de memoria y
un sistema de almacenamiento de snapshots para una aplicación MiniDB.
# Funcionalidades
## Parte A: Copy-on-Write
Se modifica el mecanismo de creación de procesos para compartir páginas de memoria entre el proceso padre y el proceso hijo.

Las páginas que originalmente son escribibiles, se marcan con `PTE_COW` y se eliminan sus permisos de escritura temporalmente.
Cuando un proceso intenta escribir sobre una página compartida, se genera un fallo de página.
También se adapta copyout() para resolver correctamente las escrituras
sobre páginas marcadas como CoW.

## Parte B: cowstat
Es una syscall para ver información de la memoria Copy-on-Write. El programa le pasa la dirección de un struct y el kernel la llena con freepages (páginas físicas libres en todo el sistema), sharedpages (páginas físicas compartidas por más de un proceso), cowfaults (fallos CoW atendidos para el proceso que llama) y cowcopies (de los fallos, cuántos copiaron la página).

Cuando un programa lo llama el kernel cuenta las páginas libres y compartidas, toma los contadores del proceso que llama y escribe los cuatro valores en la memoria del programa.

- `kernel/sysproc.c` arma el struct y devuelve con `copyout()`.
- `kernel/kalloc.c` cuenta páginas libres y compartidas.
- `kernel/cowstat.h` tiene el struct.
- `kernel/kalloc.c` agrega las funciones para contar páginas.
- `kernel/proc.c/h` agregan los contadores por proceso y los ponen en 0 al crear un proceso.
- `kernel/vm.c` hace los `cowfaults` y `cowcopies` del proceso y actualiza los contadores.
  
minidb.c la usa para medir la memoria libre y los fallos del snapshot

## Parte C: MiniDB

# Archivos principales
| Archivo | Descripción |
|---|---|
| kernel/kalloc.c | Administra las páginas físicas y referencias. |
| kernel/vm.c | Maneja las tablas de páginas y mecanismo CoW. |
| kernel/trap.c | Atención de fallos de página. |
| kernel/proc.c | Crea libera de procesos. |
| kernel/proc.h | Estructura de los procesos. |
| kernel/cowstat.h | Estructura de estadísticas CoW. |
| kernel/sysproc.c | Implementa llamadas al sistema. |
| user/cowtest.c | Pruebas del mecanismo CoW. |
| user/minidb.c | Aplicación de demostración MiniDB. |
| user/minidb.h | Estructuras y funciones auxiliares de MiniDB. |

