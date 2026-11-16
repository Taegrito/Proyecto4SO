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

