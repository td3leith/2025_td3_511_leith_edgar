# tp3

Esta actividad práctica debe hacerse en un proyecto independiente llamado `firmware` dentro de este directorio. Se debe entregar el código fuente completo dentro de este directorio ignorando el directorio destino de la compilación (_build_ o _debug_ normalmente) a través de un _.gitignore_.

Este trabajo debe hacerse en una nueva rama llamada `tp3/v1`. Se puede hacer con el comando:

```bash
git checkout -b tp3/v1
```

## Consigna

Implementar una aplicación que funcione como un contador de frecuencia. El microcontrolador debe ser capaz de detectar cada flanco ascendente por un GPIO que provenga de un generador de onda cuadrada y mostrar el valor que corresponda. Como máximo podemos estimar 10KHz de frecuencia.

### Primera versión

Implementar la consigna a través de una tarea que lea el estado del GPIO por polling y mostrar el valor de frecuencia por consola.

Una vez resuelta la consiga, hacer un commit de los cambios:

```bash
git add firmware
git commit -m "Primer consigna de TP3"
```

### Segunda versión

Reemplazar la tarea que lee el GPIO por una interrupción por flanco ascendente y mostrar el valor de frecuencia en el LCD en lugar de la consola. Usar la biblioteca [lcd](lcd) que se encuentra en este directorio.

Una vez resuelta la consiga, hacer un commit de los cambios:

```bash
git add firmware
git commit -m "Segunda consigna de TP3"
```

## Entrega

Pushear la rama `tp3/v1` al repositorio personal.

```bash
git push origin tp3/v1
```

Luego, crear un pull request a la rama correspondiente del repositorio de la cátedra con el título del pull request de _Entrega de TP3_.