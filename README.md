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


Solo se modificó `user/minidb.c`. Se completaron
`guardar_snapshot()` y `snapshot()`, y se agregó la función auxiliar
`bits_en_uno()`, que cuenta las páginas marcadas por `pgaccess()`.

### Consulta paralela (`minidb consulta K`) y mapa (`minidb mapa`)

Ambos comandos venían implementados en el esqueleto y no se cambiaron.
`consulta` crea K trabajadores con `fork()`, cada uno procesa un rango de
cuentas y envía su resultado al padre por un pipe propio. Como los
trabajadores solo leen la tabla, CoW nunca copia sus páginas: por eso la
memoria libre casi no baja después de los `fork()`.

### Snapshot consistente (`minidb snapshot M`)

El padre sigue este orden:

1. Calcula el checksum de la tabla y el saldo total **antes** del `fork()`.
2. Limpia los bits de acceso de las 64 páginas con `pgaccess()`. Se hace
   después del cálculo del checksum, porque recorrer la tabla marca todas
   las páginas como accedidas.
3. Hace `fork()`. El hijo escribe `snap.db` (un `struct snap_encabezado`
   con el checksum previo al fork, seguido de las 8.192 cuentas) y termina.
   Las cuentas se escriben directamente desde la tabla, sin copiarla a otro
   buffer; esa es la "foto" que entrega CoW sin costo.
4. Sin esperar al hijo, el padre aplica las M transferencias con semilla 4.
   Rechaza una transferencia si origen y destino son la misma cuenta o si el
   origen no tiene saldo suficiente; si la aplica, mueve el monto y suma 1 a
   `nmov` en ambas cuentas.
5. Reporta las páginas tocadas (con `pgaccess()`) y la diferencia de
   `cowfaults` y `cowcopies` medida con `cowstat()` antes y después de las
   transferencias.
6. Hace `wait()`, vuelve a leer `snap.db` y verifica el encabezado, que el
   checksum de las cuentas guardadas sea el de antes del fork y que el saldo
   total de la tabla se haya conservado. Imprime `snapshot: OK` solo si todo
   se cumple.


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

# Compilación y ejecución
Desde la raíz del proyecto, para iniciar xv6:
```bash
make qemu
```
Una vez dentro de la consola, se pueden ejecutar las siguientes pruebas
## Pruebas de Copy-on-Write
```text
cowtest
```
## Pruebas de tablas de páginas
```text
pgtbltest
```
## Consultas de MiniDb
```text
minidb consulta 1
minidb consulta 2
minidb consulta 3
minidb consulta 8
```
## Consultas de snapshots
```text
minidb snapshot 2000
minidb snapshot 0
```
## Visualización del mapa de memoria
```text
minidb snapshot 2000
minidb snapshot 0
```
## Pruebas generales de xv6
```text
usertests -q
```
# Declaración de uso de IA
Se utilizó ChatGPT como herramienta de apoyo  para la detección y correción de errores
