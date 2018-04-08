# Trivalent-fridge-controller

Controlador de neveras trivalentes con Arduino nano y lcd Nokia 5110 de bajo coste.

## Caracteristicas

- 4 modos de funcionamiento: OFF, GAS(solo gas), ECO(gas y electrico) y NORMAL (solo electrico).
- Logo de inicio personalizable.
- Control voltaje batería y apagado por seguridad nevera y alarma sonora en caso de voltaje bajo/alto .
- Control consumo resistencias y apagado por seguridad nevera y alarma sonora en caso de consumo bajo/alto tanto en AC y DC.
- Control de 4 temperaturas simultaneas (nevera, congelador, ambiente y hervidor) y alarma sonora en caso de temperaturas bajas/altas.
- Control ventilador temperatura ambiente.
- Control ventilador interior (como frigoríficos no frost).
- Detección apagado llama en modo gas y alarma sonora.
- Detección puerta abierta mucho tiempo y alarma sonora.
- Detección y selección automática AC/DC (pasa automáticamente a DC si se corta alimentación AC y a la inversa).
- Data logger con capacidad de casi 4h para visualizar gráficas con temperaturas y estado salidas para detección de fallos.
- Control mediante encoder rotatorio que sustituye selector termostato.
- Display LCD con retroiluminación con menús de visualización y configuración de alarmas.

Video ejemplo instalacion final y funcionamiento basico v1.00:

https://www.youtube.com/watch?v=7i3DB6VGs78&t=31s

## Requisitos necesarios

* [Optiboot](https://github.com/Optiboot/optiboot) - Necesario para ampliar memoria disponible Arduino nano
* [Carcasa 3D](https://www.thingiverse.com/thing:2838724) - Diseño carcasa 3D

## Diseñado y contruido con

* [Arduino IDE 1.6.12](https://www.arduino.cc/) 
* [EasyEDA](https://easyeda.com) - Esquemas electricos y diseño de la PCB
* [FreeCAD](https://easyeda.com) - Diseño carcasa
* [Cura](https://easyeda.com) - Slicer 3D

## Autores

* **J.M. Mesa** - *Initial work* - [Kirch80](https://github.com/Kirch80)

Ver tambien la lista de [colaboradores](https://github.com/your/project/contributorshttps://github.com/kirch80/Trivalent-fridge-controller/contributors) Los cuales han participado en el proyecto.

## Licencia

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Agradecimientos

* A todos aquellos que me han aportado animos e ideas para sacar esta idea adelante.
* Todos los comentarios seran bienvenidos!



